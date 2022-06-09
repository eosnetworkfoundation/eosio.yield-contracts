// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// self
#include <eosio.yield/eosio.yield.hpp>

// @protocol
[[eosio::action]]
void yield::regprotocol( const name protocol, const map<name, string> metadata )
{
    if ( !has_auth( ADMIN_CONTRACT ) ) require_auth( protocol );

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
        row.balance.contract = TOKEN_CONTRACT;
        row.balance.quantity.symbol = TOKEN_SYMBOL;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    auto itr = _protocols.find( protocol.value );
    if ( itr == _protocols.end() ) _protocols.emplace( protocol, insert );
    else _protocols.modify( itr, protocol, insert );

    // validate via admin contract
    require_recipient( ADMIN_CONTRACT );
}

// @protocol
[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver )
{
    if ( !has_auth( get_self() )) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );

    if ( receiver ) check( is_account( *receiver ), "yield::claim: [receiver] does not exists");

    // validate
    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    const extended_asset claimable = itr.balance;
    check( itr.status == "active"_n, "yield::claim: [status] must be `active`");
    check( claimable.quantity.amount > 0, "yield::claim: nothing to claim");

    // check eosio.yield balance
    const asset balance = eosio::token::get_balance( TOKEN_CONTRACT, get_self(), claimable.quantity.symbol.code() );
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
    require_auth( ADMIN_CONTRACT );
    set_status( protocol, "active"_n);
    auto config = get_config();
}

// @system
[[eosio::action]]
void yield::deny( const name protocol )
{
    require_auth( ADMIN_CONTRACT );
    set_status( protocol, "denied"_n);
}

// @system
[[eosio::action]]
void yield::setrate( const int16_t annual_rate, const asset min_tvl_report, const asset max_tvl_report )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
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

// @protocol
[[eosio::action]]
void yield::unregister( const name protocol )
{
    if ( !has_auth( get_self() )) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::unregister: [protocol] does not exists");
    _protocols.erase( itr );
}

// @oracle.yield
[[eosio::action]]
void yield::report( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd )
{
    require_auth(ORACLE_CONTRACT);

    // tables
    yield::protocols_table _protocols( get_self(), get_self().value );

    // config
    const auto config = get_config();
    const time_point_sec now = current_time_point();
    auto & itr = _protocols.get(protocol.value, "yield::report: [protocol] does not exists");
    check( itr.status == "active"_n, "yield::report: [status] is not active");

    // prevents double report
    check( itr.period_at != period, "yield::report: [period] already updated");
    check( period <= now, "yield::report: [period] must be in the past");
    check( period > itr.period_at, "yield::report: [period] must be ahead of last");
    check( period == get_current_period( period_interval ), "yield::report: [period] current period does not match");

    // validate TVL
    check( tvl.symbol == EOS, "yield::report: [tvl] does not match EOS symbol");
    check( usd.symbol == USD, "yield::report: [usd] does not match USD symbol");

    // set TVL value
    uint128_t eos = tvl.amount;
    if ( tvl > config.max_tvl_report ) eos = config.max_tvl_report.amount; // set to maximum value if exceeds max TVL value
    if ( tvl <= config.min_tvl_report ) eos = 0; // set to zero if below min TVL value

    // calculate rewards based on 5% APY
    // TVL * 5% / 365 days / 10 minute interval
    const int64_t rewards_amount = eos * config.annual_rate * period_interval / 10000 / YEAR;
    const asset rewards = { rewards_amount, EOS };

    // before balance used for report logging
    const asset balance_before = itr.balance.quantity;

    // modify contracts
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.tvl = tvl;
        row.usd = usd;
        row.balance.quantity += rewards;
        row.period_at = period;
        row.updated_at = current_time_point();
    });

    // log report
    yield::reportlog_action reportlog( get_self(), { get_self(), "active"_n });
    reportlog.send( protocol, period, period_interval, tvl, usd, rewards, balance_before, itr.balance.quantity );
}

[[eosio::action]]
void yield::reportlog( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance_before, const asset balance_after )
{
    require_auth( get_self() );
}

// @protocol
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> contracts )
{
    require_auth( protocol );

    auto config = get_config();

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::setcontracts: [protocol] does not exists");

    // require authority of all EOS contracts linked to protocol
    for ( const name contract : contracts ) {
        check( is_account( contract ), "yield::setcontracts: [eos.contract] account does not exists");
        require_auth( contract );
    }

    // modify contracts
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.status = "pending"_n; // must be re-approved if contracts changed
        row.contracts = contracts;
        row.contracts.insert(protocol); // always include EOS protocol account
        row.updated_at = current_time_point();
    });
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
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.status = "pending"_n; // must be re-approved if contracts changed
        row.evm = evm;
        row.contracts.insert(protocol); // always include EOS protocol account
        row.updated_at = current_time_point();
    });
}

[[eosio::on_notify("*::transfer")]]
void yield::on_transfer( const name from, const name to, const asset quantity, const std::string memo )
{
    require_recipient("notify.yield"_n);
}

void yield::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
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

    if (table_name == "protocols"_n) clear_table( _protocols, rows_to_clear );
    else if (table_name == "config"_n) _config.remove();
    else check(false, "yield::cleartable: [table_name] unknown table to clear" );
}
