#include "oracle.defi.hpp"

namespace defi {

[[eosio::action]]
void oracle::setprice( const uint64_t id, const name contract, const symbol sym, const uint64_t avg_price ) {
    prices _prices( get_self(), get_self().value );
    eosio::check( _prices.find(id) == _prices.end(), "oracle.defi: pair already exists" );

    // create user row
    _prices.emplace( get_self(), [&]( auto & row ) {
        row.id = id;
        row.contract = contract;
        row.coin = sym.code();
        row.precision = sym.precision();
        row.acc_price = 0;
        row.last_price = 0;
        row.avg_price = avg_price;
        row.last_update = eosio::current_time_point();
    });
};

}