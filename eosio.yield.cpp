// eosio
#include <eosio.token/eosio.token.hpp>

#include "./eosio.yield.hpp"

// @protocol
[[eosio::action]]
void yield::regprotocol( const name protocol, const map<name, string> metadata )
{
    if ( !has_auth( get_self() ) ) require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );

    // validate
    require_recipient( NOTIFY_CONTRACT );
    check_metadata_keys( metadata );

    // TO-DO
    // CHECK protocol must have ABI

    auto insert = [&]( auto& row ) {
        // status => "pending" by default
        row.protocol = protocol;
        row.metadata = metadata;
        row.balance.contract = TOKEN_CONTRACT;
        row.balance.quantity.symbol = TOKEN_SYMBOL;
        row.claimed.symbol = TOKEN_SYMBOL;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    auto itr = _protocols.find( protocol.value );
    if ( itr == _protocols.end() ) _protocols.emplace( get_self(), insert );
    else _protocols.modify( itr, get_self(), insert );
}

void yield::check_metadata_keys(const map<name, string> metadata )
{
    const auto config = get_config();
    const set<name> metadata_keys = config.metadata_keys;
    for ( const auto item : metadata ) {
        const name key = item.first;
        check( metadata_keys.find(key) != metadata_keys.end(), "yield::check_metadata_keys: invalid [metadata_key=" + key.to_string() + "]");
    }
}

// @protocol
[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver )
{
    if ( !has_auth( get_self() )) require_auth( protocol );
    require_recipient(NOTIFY_CONTRACT);

    yield::protocols_table _protocols( get_self(), get_self().value );

    if ( receiver ) check( is_account( *receiver ), "yield::claim: [receiver] does not exists");

    // validate
    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    const extended_asset claimable = itr.balance;
    check( itr.status == "active"_n, "yield::claim: [status] must be `active`");
    check( claimable.quantity.amount > 0, "yield::claim: nothing to claim");

    // transfer funds to receiver
    const name to = receiver ? *receiver : protocol;
    transfer( get_self(), to, claimable, "Yield+ TVL reward");

    // modify balances
    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.claimed += claimable.quantity;
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
    require_recipient(NOTIFY_CONTRACT);
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
    require_auth( get_self() );
    set_status( protocol, "active"_n);
    require_recipient(ORACLE_CONTRACT);
}

// @system
[[eosio::action]]
void yield::deny( const name protocol )
{
    require_auth( get_self() );
    set_status( protocol, "denied"_n);
    require_recipient(ORACLE_CONTRACT);
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

// @system
[[eosio::action]]
void yield::setmetakeys( const set<name> metadata_keys )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    config.metadata_keys = metadata_keys;
    _config.set(config, get_self());
}

// @protocol
[[eosio::action]]
void yield::unregister( const name protocol )
{
    if ( !has_auth( get_self() )) require_auth( protocol );
    require_recipient(NOTIFY_CONTRACT);

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::unregister: [protocol] does not exists");
    _protocols.erase( itr );

    // notify oracle
    require_recipient(ORACLE_CONTRACT);
}

// @oracle.yield
[[eosio::action]]
void yield::report( const name protocol, const time_point_sec period, const TVL tvl )
{
    require_auth(ORACLE_CONTRACT);
    require_recipient(NOTIFY_CONTRACT);

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
    // TO-DO ADD period does not match
    // check( period == get_current_period(), "yield::report: [period] current period does not match");

    // validate
    // skip if does not meet minimum TVL report threshold
    check( tvl.eos.symbol == EOS, "yield::report: [tvl.eos] does not match EOS symbol");
    check( tvl.usd.symbol == USD, "yield::report: [tvl.usd] does not match USD symbol");
    check( tvl.eos >= config.min_tvl_report, "yield::report: [eos] does not meet minimum TVL report threshold");

    // limit TVL to maximum report threhsold
    const uint128_t eos = ((tvl.eos > config.max_tvl_report) ? config.max_tvl_report : tvl.eos).amount;
    const int64_t rewards_amount = eos * config.annual_rate * PERIOD_INTERVAL / 10000 / YEAR;
    const asset rewards = { rewards_amount, EOS };

    // before balance used for report logging
    const asset balance_before = itr.balance.quantity;

    // modify contracts
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.tvl = tvl;
        row.balance.quantity += rewards;
        row.period_at = period;
    });

    // log report
    yield::reportlog_action reportlog( get_self(), { get_self(), "active"_n });
    reportlog.send( protocol, period, tvl, rewards, balance_before, itr.balance.quantity );
}

[[eosio::action]]
void yield::reportlog( const name protocol, const time_point_sec period, const TVL tvl, const asset rewards, const asset balance_before, const asset balance_after )
{
    require_auth( get_self() );
    require_recipient(NOTIFY_CONTRACT);
}

// @protocol
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> eos, const set<string> evm )
{
    require_recipient(NOTIFY_CONTRACT);

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::setcontracts: [protocol] does not exists");

    // skip additional checks if system is not authorized
    if ( !has_auth( get_self() ) ) {
        require_auth( protocol );

        check(itr.status == "pending"_n, "yield::setcontracts: [status] must be `pending` to modify");

        // require authority of all EOS contracts linked to protocol
        for ( const name contract : eos ) {
            require_auth( contract );
        }
        // require authority of all EVM contracts linked to protocol
        for ( const string contract : evm ) {
            check(false, "NOT IMPLEMENTED");
        }
    }

    // modify contracts
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.contracts.eos = eos;
        row.contracts.evm = evm;
        row.updated_at = current_time_point();
    });

    // notify oracle
    require_recipient(ORACLE_CONTRACT);
}

[[eosio::on_notify("*::transfer")]]
void yield::on_transfer( const name from, const name to, const asset quantity, const std::string memo )
{
    require_recipient(NOTIFY_CONTRACT);
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

time_point_sec yield::get_current_period()
{
    const uint32_t now = current_time_point().sec_since_epoch();
    return time_point_sec((now / PERIOD_INTERVAL) * PERIOD_INTERVAL);
}