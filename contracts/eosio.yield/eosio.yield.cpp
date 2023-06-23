// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// EOS EVM
#include <eosio.evm/eosio.evm.hpp>
#include <eosio.evm/silkworm.hpp>

// core
#include <eosio.yield/eosio.yield.hpp>

// logging (used for backend syncing)
#include "src/logs.cpp"

// DEBUG (used to help testing)
#ifdef DEBUG
#include "src/debug.cpp"
#endif

// @protocol OR @admin
[[eosio::action]]
void yield::regprotocol( const name protocol, const name category, const map<name, string> metadata )
{
    require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );

    // protocol must be smart contract that includes ABI
    check( is_contract( protocol ), "yield::regprotocol: [protocol] must be a smart contract");

    const auto config = get_config();
    auto insert = [&]( auto& row ) {
        row.protocol = protocol;
        row.category = category;
        row.tvl.symbol = EOS;
        row.usd.symbol = USD;
        row.contracts.insert( protocol );
        row.balance.contract = config.rewards.get_contract();
        row.balance.quantity.symbol = config.rewards.get_symbol();
        row.metadata = metadata;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    auto itr = _protocols.find( protocol.value );
    const bool is_exists = itr != _protocols.end();
    if ( is_exists ) _protocols.modify( itr, protocol, insert );
    else _protocols.emplace( protocol, insert );

    // if denied revert back to pending
    if ( itr->status == "denied"_n ) set_status(protocol, "pending"_n);

    // logging
    yield::createlog_action createlog( get_self(), { get_self(), "active"_n });
    yield::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });

    if ( !is_exists ) createlog.send( protocol, "pending"_n, category, metadata );
    else  metadatalog.send( protocol, itr->status, category, metadata );
}

// @protocol OR @admin
[[eosio::action]]
void yield::setmetadata( const name protocol, const map<name, string> metadata )
{
    require_auth_admin( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get( protocol.value, "yield::setmetadata: [protocol] does not exists");

    _protocols.modify( itr, get_ram_payer(protocol), [&]( auto& row ) {
        row.metadata = metadata;
        row.updated_at = current_time_point();
    });

    // if denied revert back to pending
    if ( itr.status == "denied"_n ) set_status(protocol, "pending"_n);

    // logging
    yield::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });
    metadatalog.send( protocol, itr.status, itr.category, metadata );
}

// @protocol OR @admin
[[eosio::action]]
void yield::setmetakey( const name protocol, const name key, const optional<string> value )
{
    require_auth_admin(protocol);

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get( protocol.value, "yield::setmetakey: [protocol] does not exists");

    _protocols.modify( itr, get_ram_payer(protocol), [&]( auto& row ) {
        if ( value ) row.metadata[key] = *value;
        else row.metadata.erase(key);
        row.updated_at = current_time_point();
    });

    // if denied revert back to pending
    if ( itr.status == "denied"_n ) set_status(protocol, "pending"_n);

    // logging
    yield::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });
    metadatalog.send( protocol, itr.status, itr.category, itr.metadata );
}

// @protocol
[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver, const optional<string> evm_receiver )
{
    require_auth_admin(protocol);

    yield::protocols_table _protocols( get_self(), get_self().value );

    if ( receiver ) check( is_account( *receiver ), "yield::claim: [receiver] does not exists");

    // validate
    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    const extended_asset claimable = itr.balance;
    check( claimable.quantity.amount > 0, "yield::claim: nothing to claim");

    // check eosio.yield balance
    const asset balance = eosio::token::get_balance( claimable.contract, get_self(), claimable.quantity.symbol.code() );
    check( balance >= claimable.quantity, "yield::claim: contract has insuficient balance, please contact administrator");

    // transfer funds to receiver
    if ( evm_receiver ) {
        transfer( get_self(), "eosio.evm"_n, claimable, *evm_receiver);
    } else {
        const name to = receiver ? *receiver : protocol;
        transfer( get_self(), to, claimable, "Yield+ TVL reward");
    }

    // modify balances
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity.amount = 0;
        row.claimed_at = current_time_point();
    });

    // logging
    yield::claimlog_action claimlog( get_self(), { get_self(), "active"_n });
    claimlog.send( protocol, itr.category, *receiver, *evm_receiver, claimable.quantity, itr.balance.quantity );
}

void yield::set_status( const name protocol, const name status )
{
    yield::protocols_table _protocols( get_self(), get_self().value );

    auto & itr = _protocols.get(protocol.value, "yield::set_status: [protocol] does not exists");
    check( PROTOCOL_STATUS_TYPES.find( status ) != PROTOCOL_STATUS_TYPES.end(), "yield::set_status: [status] is invalid");

    if ( itr.status == status ) return; // no status change
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.status = status;
    });

    // logging
    yield::statuslog_action statuslog( get_self(), { get_self(), "active"_n });
    statuslog.send( protocol, status );
}

void yield::set_category( const name protocol, const name category )
{
    yield::protocols_table _protocols( get_self(), get_self().value );

    auto & itr = _protocols.get(protocol.value, "yield::set_category: [protocol] does not exists");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        check( row.category != category, "yield::set_category: [category] not modified");
        row.category = category;
    });

    // if denied revert back to pending
    if ( itr.status == "denied"_n ) set_status(protocol, "pending"_n);

    // logging
    yield::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });
    metadatalog.send( protocol, itr.status, category, itr.metadata );
}

// @admin
[[eosio::action]]
void yield::approve( const name protocol )
{
    require_auth_admin();
    set_status( protocol, "active"_n );
    add_active_protocol( protocol );
}

// @admin
[[eosio::action]]
void yield::setcategory( const name protocol, const name category )
{
    require_auth_admin( protocol );
    set_category( protocol, category );
}

// @admin
[[eosio::action]]
void yield::deny( const name protocol )
{
    require_auth_admin();
    set_status( protocol, "denied"_n);
    remove_active_protocol( protocol );
}

// @system
[[eosio::action]]
void yield::setrate( const optional<int16_t> annual_rate, const optional<asset> min_tvl_report, const optional<asset> max_tvl_report )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    check( _config.exists(), "yield::setrate: contract must first call [init] action");
    auto config = get_config();

    if ( annual_rate ) config.annual_rate = *annual_rate;
    if ( min_tvl_report ) config.min_tvl_report = *min_tvl_report;
    if ( max_tvl_report ) config.max_tvl_report = *max_tvl_report;

    check( config.annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    check( config.min_tvl_report <= config.max_tvl_report, "yield::setrate: [min_tvl_report] must be less than [max_tvl_report]");
    check( config.min_tvl_report.symbol == EOS, "yield::setrate: [min_tvl_report] invalid EOS symbol");
    check( config.max_tvl_report.symbol == EOS, "yield::setrate: [min_tvl_report] invalid EOS symbol");

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

// @protocol or @admin
[[eosio::action]]
void yield::unregister( const name protocol )
{
    require_auth_admin(protocol);

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::unregister: [protocol] does not exists");
    check( itr.balance.quantity.amount == 0, "yield::unregister: protocol has " + itr.balance.quantity.to_string() + " remaining balance, must execute `claim` ACTION before `unregister`");
    _protocols.erase( itr );
    remove_active_protocol( protocol );

    // logging
    yield::eraselog_action eraselog( get_self(), { get_self(), "active"_n });
    eraselog.send( protocol );
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

    // set to maximum value if exceeds max TVL value
    const int64_t tvl_amount = (tvl > config.max_tvl_report) ? config.max_tvl_report.amount : tvl.amount;

    // calculate rewards based on 5% APY
    // TVL * 5% / 365 days / 10 minute interval
    int64_t rewards_amount = uint128_t(tvl_amount) * config.annual_rate * period_interval / 10000 / YEAR;

    // determine if project is eligible for rewards
    // set rewards to 0
    if ( tvl <= config.min_tvl_report ) rewards_amount = 0; // TVL must be above minimum TVL requirement
    if ( itr.status != "active"_n ) rewards_amount = 0; // protocol must be active to receive rewards (denied or pending)

    // update rewards to protocol's balance
    const asset rewards = { rewards_amount, config.rewards.get_symbol() };
    if ( rewards.amount ) {
        _protocols.modify( itr, same_payer, [&]( auto& row ) {
            row.balance.quantity += rewards;
        });

        // log report
        yield::rewardslog_action rewardslog( get_self(), { get_self(), "active"_n });
        rewardslog.send( protocol, itr.category, period, period_interval, tvl, usd, rewards, itr.balance.quantity );
    }
}

// @protocol or @admin
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> contracts, const set<string> evm_contracts )
{
    // override permission is required to allow certain protocols with nulled permission to be added to Yield+
    require_auth_admin(protocol);

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::setcontracts: [protocol] does not exists");

    // Only support EOS or EVM contracts (not both)
    if ( evm_contracts.size() && contracts.size() ) check( false, "yield::setcontracts: cannot include both [evm_contracts] with [contracts]");

    // validate contracts
    for ( const name contract : contracts ) {
        check( is_account( contract ), "yield::setcontracts: [contract=" + contract.to_string() + "] account does not exists");
    }
    for ( const string evm_contract : evm_contracts ) {
        check( evm_contract::is_account( *silkworm::from_hex(evm_contract) ), "yield::setcontracts: [evm_contract=" + evm_contract + "] account ID does not exists");
    }

    // modify contracts
    _protocols.modify( itr, get_ram_payer(protocol), [&]( auto& row ) {
        // prevent modification if no changes
        if ( contracts.size() ) check( row.contracts != contracts, "yield::setcontracts: [contracts] was not modified");
        if ( evm_contracts.size() ) check( row.evm_contracts != evm_contracts, "yield::setcontracts: [evm_contracts] was not modified");

        row.contracts = contracts;
        row.evm_contracts = evm_contracts;
        row.updated_at = current_time_point();
    });

    // protocol must be re-approved if `setcontracts` action is called
    set_status( protocol, "pending"_n );
    remove_active_protocol( protocol );

    // logging
    yield::contractslog_action contractslog( get_self(), { get_self(), "active"_n });
    contractslog.send( protocol, itr.status, itr.contracts, itr.evm_contracts );
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
    check( _config.exists(), "yield::get_config: [config] is not initialized");
    return _config.get();
}

time_point_sec yield::get_current_period( const uint32_t period_interval )
{
    const uint32_t now = current_time_point().sec_since_epoch();
    return time_point_sec((now / period_interval) * period_interval);
}

bool yield::is_contract( const name contract )
{
    // TO-DO: upgrade CDT to v4.0.0
    // eosio::get_code_hash( contract );
    // https://github.com/AntelopeIO/cdt/releases/tag/v4.0.0
    return true;
}

void yield::require_auth_admin()
{
    require_auth( get_config().admin_contract );
}

void yield::require_auth_admin( const name account )
{
    if ( has_auth( get_config().admin_contract ) ) return;
    require_auth( account );
}

name yield::get_ram_payer( const name account )
{
    const name admin = get_config().admin_contract;
    if ( has_auth( admin ) ) return admin;
    return account;
}