// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// self
#include <eosio.yield/eosio.yield.hpp>

// @protocol
[[eosio::action]]
void yield::regprotocol( const name protocol, const map<name, string> metadata )
{
    const auto config = get_config();
    const bool is_admin = has_auth( config.admin_contract );
    if ( !is_admin ) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );

    // protocol must be smart contract that includes ABI
    eosiosystem::abihash_table _abihash( "eosio"_n, "eosio"_n.value );
    _abihash.get( protocol.value, "yield::regprotocol: [protocol] must be a smart contract");

    auto insert = [&]( auto& row ) {
        if ( !row.status.value ) row.status = "pending"_n;
        row.tvl.symbol = EOS;
        row.usd.symbol = USD;
        row.contracts.insert( protocol );
        row.protocol = protocol;
        row.metadata = metadata;
        row.balance.contract = config.rewards.get_contract();
        row.balance.quantity.symbol = config.rewards.get_symbol();
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    const name ram_payer = is_admin ? config.admin_contract : protocol;
    auto itr = _protocols.find( protocol.value );
    if ( itr == _protocols.end() ) _protocols.emplace( ram_payer, insert );
    else _protocols.modify( itr, ram_payer, insert );

    // validate via admin contract
    require_recipient( config.admin_contract );
}

// @protocol
[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver )
{
    require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    const auto config = get_config();

    if ( receiver ) check( is_account( *receiver ), "yield::claim: [receiver] does not exists");

    // validate
    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    const extended_asset claimable = itr.balance;
    check( itr.status == "active"_n, "yield::claim: [status] must be `active`");
    check( claimable.quantity.amount > 0, "yield::claim: nothing to claim");

    // check eosio.yield balance
    const asset balance = eosio::token::get_balance( claimable.contract, get_self(), claimable.quantity.symbol.code() );
    check( balance >= claimable.quantity, "yield::claim: contract has insuficient balance, please contact administrator");

    // transfer funds to receiver
    const name to = receiver ? *receiver : protocol;
    transfer( get_self(), to, claimable, "Yield+ TVL reward");

    // modify balances
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity.amount = 0;
        row.claimed_at = current_time_point();
    });

    // logging
    yield::claimlog_action claimlog( get_self(), { get_self(), "active"_n });
    claimlog.send( protocol, to, claimable );
}

// @eosio.code
[[eosio::action]]
void yield::claimlog( const name protocol, const name receiver, const extended_asset claimed )
{
    require_auth( get_self() );
}

void yield::set_status( const name protocol, const name status )
{
    yield::protocols_table _protocols( get_self(), get_self().value );

    auto & itr = _protocols.get(protocol.value, "yield::set_status: [protocol] does not exists");
    check( PROTOCOL_STATUS_TYPES.find( status ) != PROTOCOL_STATUS_TYPES.end(), "yield::set_status: [status] is invalid");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        check( row.status != status, "yield::set_status: [status] not modified");
        row.status = status;
    });
}

// @system
[[eosio::action]]
void yield::approve( const name protocol )
{
    const auto config = get_config();
    require_auth( config.admin_contract );
    set_status( protocol, "active"_n);
    add_active_protocol( protocol );
}

// @system
[[eosio::action]]
void yield::deny( const name protocol )
{
    const auto config = get_config();
    require_auth( config.admin_contract );
    set_status( protocol, "denied"_n);
    remove_active_protocol( protocol );
}

// @system
[[eosio::action]]
void yield::setrate( const int16_t annual_rate, const asset min_tvl_report, const asset max_tvl_report )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    check( _config.exists(), "yield::setrate: contract must first call [init] action");
    auto config = get_config();

    check( annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    check( min_tvl_report <= max_tvl_report, "yield::setrate: [min_tvl_report] must be less than [max_tvl_report]");

    // validate symbols
    check( min_tvl_report.symbol == EOS, "yield::setrate: [min_tvl_report] invalid EOS symbol");
    check( max_tvl_report.symbol == EOS, "yield::setrate: [min_tvl_report] invalid EOS symbol");

    config.annual_rate = annual_rate;
    config.min_tvl_report = min_tvl_report;
    config.max_tvl_report = max_tvl_report;
    _config.set(config, get_self());
}

// @system
[[eosio::action]]
void yield::init( const extended_symbol rewards, const name oracle_contract, const name admin_contract )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();

    // check if accounts exists
    check( is_account( rewards.get_contract() ), "yield::init: [rewards.contract] account does not exists");
    check( is_account( oracle_contract ), "yield::init: [oracle_contract] account does not exists");
    check( is_account( admin_contract ), "yield::init: [admin_contract] account does not exists");

    // validate token
    const asset supply = eosio::token::get_supply( rewards.get_contract(), rewards.get_symbol().code() );
    check( supply.symbol == rewards.get_symbol(),  "yield::init: [supply.symbol] does not match [rewards]");

    // cannot modify existing values
    if ( config.rewards.get_contract() ) check( config.rewards == rewards, "yield::init: [rewards] cannot be modified once initialized");

    // set values
    config.rewards = rewards;
    config.oracle_contract = oracle_contract;
    config.admin_contract = admin_contract;
    config.min_tvl_report.symbol = EOS;
    config.max_tvl_report.symbol = EOS;
    _config.set(config, get_self());
}

// @protocol
[[eosio::action]]
void yield::unregister( const name protocol )
{
    const auto config = get_config();
    if ( !has_auth( config.admin_contract )) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::unregister: [protocol] does not exists");
    if ( itr.balance.quantity.amount > 0 ) claim( protocol, ""_n ); // claim if any balance remaining
    _protocols.erase( itr );
    remove_active_protocol( protocol );
}

// @oracle.yield
[[eosio::action]]
void yield::report( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd )
{
    const auto config = get_config();
    require_auth(config.oracle_contract);

    // tables
    yield::protocols_table _protocols( get_self(), get_self().value );

    // config
    const time_point_sec now = current_time_point();
    auto & itr = _protocols.get(protocol.value, "yield::report: [protocol] does not exists");
    check( config.min_tvl_report.amount, "yield::report: [min_tvl_report] not configured");
    check( config.max_tvl_report.amount, "yield::report: [max_tvl_report] not configured");

    // prevents double report
    check( itr.period_at != period, "yield::report: [period] already updated");
    check( period <= now, "yield::report: [period] cannot be in the future");
    check( period > itr.period_at, "yield::report: [period] must be ahead of last");
    check( period == get_current_period( period_interval ), "yield::report: [period] current period does not match");

    // validate TVL
    check( tvl.symbol == EOS, "yield::report: [tvl] does not match EOS symbol");
    check( usd.symbol == USD, "yield::report: [usd] does not match USD symbol");

    // update protocol's TVL
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.tvl = tvl;
        row.usd = usd;
        row.period_at = period;
        row.updated_at = current_time_point();
    });

    // determine if project is eligible for rewards
    if ( tvl <= config.min_tvl_report ) return; // skip if below min TVL value
    if ( itr.status != "active"_n ) return; // skip if not active (denied or pending)

    // set to maximum value if exceeds max TVL value
    const int64_t tvl_amount = (tvl > config.max_tvl_report) ? config.max_tvl_report.amount : tvl.amount;

    // calculate rewards based on 5% APY
    // TVL * 5% / 365 days / 10 minute interval
    const int64_t rewards_amount = uint128_t(tvl_amount) * config.annual_rate * period_interval / 10000 / YEAR;
    const asset rewards = { rewards_amount, config.rewards.get_symbol() };

    // before balance used for report logging
    const asset balance_before = itr.balance.quantity;

    // update rewards to protocol's balance
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity += rewards;
    });

    // log report
    yield::rewardslog_action rewardslog( get_self(), { get_self(), "active"_n });
    rewardslog.send( protocol, period, period_interval, tvl, usd, rewards, balance_before, itr.balance.quantity );
}

[[eosio::action]]
void yield::rewardslog( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance_before, const asset balance_after )
{
    require_auth( get_self() );
}

// @protocol
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> contracts )
{
    const auto config = get_config();
    const bool is_admin = has_auth( config.admin_contract );
    if ( !is_admin ) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::setcontracts: [protocol] does not exists");

    // require authority of all EOS contracts linked to protocol
    for ( const name contract : contracts ) {
        check( is_account( contract ), "yield::setcontracts: [eos.contract] account does not exists");
        if ( !is_admin ) require_auth( contract );
    }

    // modify contracts
    const set<name> before_contracts = itr.contracts;
    const name ram_payer = is_admin ? config.admin_contract : protocol;
    _protocols.modify( itr, ram_payer, [&]( auto& row ) {
        row.status = "pending"_n; // must be re-approved if contracts changed
        row.contracts = contracts;
        row.contracts.insert(protocol); // always include EOS protocol account
        row.updated_at = current_time_point();
        check( row.contracts != before_contracts, "yield::setcontracts: [contract] was not modified");
    });
    remove_active_protocol( protocol );
}

// @protocol
[[eosio::action]]
void yield::setevm( const name protocol, const set<string> evm )
{
    require_auth( protocol );

    auto config = get_config();

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::setevm: [protocol] does not exists");

    // require authority of all EVM contracts linked to protocol
    for ( const string contract : evm ) {
        check(false, "NOT IMPLEMENTED");
    }

    // modify contracts
    const set<string> before_evm = itr.evm;
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.status = "pending"_n; // must be re-approved if contracts changed
        row.evm = evm;
        row.contracts.insert(protocol); // always include EOS protocol account
        row.updated_at = current_time_point();
        check( row.evm != before_evm, "yield::setevm: [evm] was not modified");
    });
}

[[eosio::on_notify("*::transfer")]]
void yield::on_transfer( const name from, const name to, const asset quantity, const std::string memo )
{
    auto config = get_config();
    require_recipient(config.admin_contract);
}

void yield::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}

void yield::add_active_protocol( const name protocol )
{
    yield::state_table _state( get_self(), get_self().value );
    auto state = _state.get_or_default();
    state.active_protocols.insert( protocol );
    _state.set(state, get_self());
}

void yield::remove_active_protocol( const name protocol )
{
    yield::state_table _state( get_self(), get_self().value );
    auto state = _state.get_or_default();
    state.active_protocols.erase( protocol );
    _state.set(state, get_self());
}

yield::config_row yield::get_config()
{
    yield::config_table _config( get_self(), get_self().value );
    check( _config.exists(), "yield::get_config: contract is not initialized");
    return _config.get();
}

time_point_sec yield::get_current_period( const uint32_t period_interval )
{
    const uint32_t now = current_time_point().sec_since_epoch();
    return time_point_sec((now / period_interval) * period_interval);
}

// @debug
template <typename T>
void yield::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void yield::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    yield::config_table _config( get_self(), value );
    yield::protocols_table _protocols( get_self(), value );
    yield::state_table _state( get_self(), value );

    if (table_name == "protocols"_n) clear_table( _protocols, rows_to_clear );
    else if (table_name == "config"_n) _config.remove();
    else if (table_name == "state"_n) _state.remove();
    else check(false, "yield::cleartable: [table_name] unknown table to clear" );
}
