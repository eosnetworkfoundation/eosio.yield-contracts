[[eosio::on_notify("eosio.yield::approve")]]
void oracle::on_approve( const name protocol )
{
    // required if `on_notify` uses * wildcard for contract
    check( get_first_receiver() == YIELD_CONTRACT, "oracle::on_unregister: [get_first_receiver] is invalid");
    register_protocol( protocol );
}

[[eosio::on_notify("eosio.yield::deny")]]
void oracle::on_deny( const name protocol )
{
    // required if `on_notify` uses * wildcard for contract
    check( get_first_receiver() == YIELD_CONTRACT, "oracle::on_unregister: [get_first_receiver] is invalid");
    erase_protocol( protocol );
}

[[eosio::on_notify("eosio.yield::unregister")]]
void oracle::on_unregister( const name protocol )
{
    // required if `on_notify` uses * wildcard for contract
    check( get_first_receiver() == YIELD_CONTRACT, "oracle::on_unregister: [get_first_receiver] is invalid");
    erase_protocol( protocol );
}

void oracle::erase_protocol( const name protocol )
{
    oracle::tvl_table _tvl( get_self(), get_self().value );
    auto itr = _tvl.find( protocol.value );
    if ( itr != _tvl.end() ) _tvl.erase( itr );
}

void oracle::register_protocol( const name protocol )
{
    oracle::tvl_table _tvl( get_self(), get_self().value );
    auto itr = _tvl.find( protocol.value );
    if ( itr != _tvl.end() ) return; // skip already exists

    // create initial protocol
    _tvl.emplace( get_self(), [&]( auto& row ) {
        row.protocol = protocol;
    });
}