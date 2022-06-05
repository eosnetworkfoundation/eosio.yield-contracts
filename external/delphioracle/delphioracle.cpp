#include "delphioracle.hpp"

[[eosio::action]]
void delphioracle::setprice( const name pair, const uint64_t value )
{
    datapointstable _datapointstable( get_self(), pair.value );

    // create
    _datapointstable.emplace( get_self(), [&]( auto & row ) {
        row.id = _datapointstable.available_primary_key();
        row.owner = get_self();
        row.value = value;
        row.median = value;
        row.timestamp = eosio::current_time_point();
    });
};

[[eosio::action]]
void delphioracle::setpair( const name pair, const symbol base_symbol, const name base_contract, const uint64_t quoted_precision )
{
    pairstable _pairstable( get_self(), get_self().value );
    check( _pairstable.find( pair.value ) == _pairstable.end(), "pair already exists");

    // create
    _pairstable.emplace( get_self(), [&]( auto & row ) {
        row.active = true;
        row.name = pair;

        row.base_symbol = base_symbol;
        row.base_contract = base_contract;

        row.quote_symbol = symbol{"USD", 2};
        row.quote_contract = ""_n;

        row.quoted_precision = quoted_precision;
    });
};
