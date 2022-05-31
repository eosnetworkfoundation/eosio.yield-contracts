#include "eosio.system.hpp"

namespace eosio {

[[eosio::action]]
void system_contract::abihash( const name owner, const checksum256 hash )
{
    // require_auth( get_self() );

    eosio::system_contract::abihash_table _abihash( "eosio"_n, "eosio"_n.value );
    check(_abihash.find( owner.value) == _abihash.end(), "eosio::setabi: owner already exists" );

    _abihash.emplace( get_self(), [&]( auto & row ) {
        row.owner = owner;
        row.hash = hash;
    });
};

}