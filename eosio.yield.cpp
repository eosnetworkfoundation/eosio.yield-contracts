#include "./eosio.yield.hpp"

[[eosio::action]]
void yield::regprotocol( const name protocol, const map<string, string> metadata )
{
    require_auth( protocol );

    yield::protocols_table _protocols( get_self(), get_self().value );
    const auto configs = get_configs();
    const set<name> metadata_keys = configs.metadata_keys;

    // validate input
    check(configs.status == "active"_n, "yield::regprotocol: [status] must be `active`");
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
    });

    // modify or create
    auto itr = _protocols.find( protocol.value );
    if ( itr == _protocols.end() ) _protocols.emplace( get_self(), insert );
    else _protocols.modify( itr, get_self(), insert );
}

[[eosio::action]]
void yield::setstatus( const name protocol, const name status )
{
    require_auth( get_self() );

    auto & itr = _protocols.get(protocol.value, "yield::approve: [protocol] does not exists");
    check( PROTOCOL_STATUS_TYPES.find( status ) != PROTOCOL_STATUS_TYPES.end(), "yield::approve: [status] is invalid");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.status = status;
    });
}

[[eosio::action]]
void yield::claim( const name protocol, const optional<name> receiver )
{
    require_auth( protocol );

    if ( receiver ) check( is_account( *receiver ), "yield::claim: [receiver] does not exists");

    auto & itr = _protocols.get(protocol.value, "yield::claim: [protocol] does not exists");
    check( itr.status == "active"_n, "yield::claim: [status] must be `active`");
    check( itr.balance.quantity.amount > 0, "yield::claim: [balance] is empty");

    // transfer funds to receiver
    const name to = receiver ? *receiver : protocol;
    transfer( get_self(), to, itr.balance, "Yield+ TVL reward");

    _protocols.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity.amount = 0;
        row.last_claim = current_time_point();
    });
}

[[eosio::action]]
void setrate( const int64 annual_rate )
{
    yield::configs_table _configs( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    config.annual_rate = annual_rate;
    _config.set(config, get_self());
}

[[eosio::action]]
void setrate( const int64 annual_rate )
{
    yield::configs_table _configs( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( annual_rate <= MAX_ANNUAL_RATE, "yield::setrate: [annual_rate] exceeds maximum annual rate");
    config.annual_rate = annual_rate;
    _config.set(config, get_self());
}

[[eosio::action]]
void setmetakeys( const map<string, string> metadata_keys )
{
    yield::configs_table _configs( get_self(), get_self().value );
    auto config = _config.get_or_default();
    config.metadata_keys = metadata_keys;
    _config.set(config, get_self());
}

void yield::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}

yield::configs_row yield::get_configs()
{
    yield::configs_table _configs( get_self(), get_self().value );
    check( _configs.exists(), "yield::get_configs: contract is not initialized");
    return _configs.get();
}