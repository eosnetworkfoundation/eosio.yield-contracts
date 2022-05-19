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

    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    check( itr.status == "active"_n, "yield::claim: [status] must be `active`");
    check( itr.balance.quantity.amount > 0, "yield::claim: [balance] is empty");

    // transfer funds to receiver
    const name to = receiver ? *receiver : protocol;
    transfer( get_self(), to, itr.balance, "Yield+ TVL reward");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity.amount = 0;
        row.claimed_at = current_time_point();
    });
}

// @admin
[[eosio::action]]
void yield::setstatus( const name protocol, const name status )
{
    require_auth( get_self() );

    yield::protocols_table _protocols( get_self(), get_self().value );

    auto & itr = _protocols.get(protocol.value, "yield::approve: [protocol] does not exists");
    check( PROTOCOL_STATUS_TYPES.find( status ) != PROTOCOL_STATUS_TYPES.end(), "yield::approve: [status] is invalid");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.status = status;
    });
}

// @system
[[eosio::action]]
void yield::setrate( const int64_t annual_rate )
{
    require_auth( get_self() );

    yield::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    config.annual_rate = annual_rate;
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
    check(false, "TO-DO");
}

[[eosio::on_notify("oracle.yield::report")]]
void yield::on_report( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos )
{
    check(false, "TO-DO");
}

// @protocol
[[eosio::action]]
void yield::setcontracts( const name protocol, const set<name> contracts )
{
    check(false, "TO-DO");
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