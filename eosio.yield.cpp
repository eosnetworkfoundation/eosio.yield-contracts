// eosio
#include <eosio.token/eosio.token.hpp>

#include "./eosio.yield.hpp"

// @protocol
[[eosio::action]]
void yield::regprotocol( const name protocol, const map<name, string> metadata )
{
    require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    const auto config = get_config();
    const set<name> metadata_keys = config.metadata_keys;

    // validate input
    check(config.status == "active"_n, "yield::regprotocol: [status] must be `active`");
    for ( const auto item : metadata ) {
        check( metadata_keys.find(item.first) != metadata_keys.end(), "yield::regprotocol: invalid [metadata_keys]");
    }

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

// @protocol
[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver )
{
    require_auth( protocol );

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
    claimlog.send( protocol, to, claimable.quantity );
}

// @eosio.code
[[eosio::action]]
void yield::claimlog( const name protocol, const name receiver, const asset claimed )
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
void yield::setrate( const int16_t annual_rate, const int64_t min_eos_tvl_report, const int64_t max_eos_tvl_report )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    check( min_eos_tvl_report <= max_eos_tvl_report, "yield::setrate: [min_eos_tvl_report] must be less than [max_eos_tvl_report]");

    config.annual_rate = annual_rate;
    config.min_eos_tvl_report = min_eos_tvl_report;
    config.max_eos_tvl_report = max_eos_tvl_report;
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
    require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::unregister: [protocol] does not exists");
    _protocols.erase( itr );

    // notify oracle
    require_recipient(ORACLE_CONTRACT);
}

[[eosio::on_notify("oracle.yield::report")]]
void yield::on_report( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos )
{
    yield::protocols_table _protocols( get_self(), get_self().value );
    auto & itr = _protocols.get(protocol.value, "yield::on_report: [protocol] does not exists");

    // required if `on_notify` uses * wildcard for contract
    check( get_first_receiver() == ORACLE_CONTRACT, "yield::on_report: [get_first_receiver] is invalid");
    check( itr.period_at == period, "yield::on_report: [period] already updated"); // prevents double report

    // calculate rewards
    const auto config = get_config();
    const int64_t rewards = static_cast<uint128_t>(eos) * config.annual_rate * PERIOD_INTERVAL / 10000 / YEAR;

    // modify contracts
    _protocols.modify( itr, protocol, [&]( auto& row ) {
        row.balance.quantity.amount += rewards;
        row.period_at = period;
    });
}

// @protocol
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> eos, const set<string> evm )
{
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

    // notify oracle on update
    require_recipient(ORACLE_CONTRACT);
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