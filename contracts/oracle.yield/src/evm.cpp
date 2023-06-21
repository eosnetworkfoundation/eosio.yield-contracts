// @system
[[eosio::action]]
void oracle::addevmtoken( const symbol_code symcode, const bytes address, const uint8_t decimals )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );
    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );
    evm_contract::account_table _account( get_self(), get_self().value );

    // validate
    const auto token = _tokens.get( symcode.raw(), "oracle::addevmtoken: [symcode] token not found" );
    const uint64_t account_id = get_account_id(address);

    // add supported token
    auto insert = [&]( auto& row ) {
        row.token_id = account_id;
        row.sym = token.sym;
        row.address = address;
        row.decimals = decimals;
    };

    // modify or create
    auto itr = _evm_tokens.find( account_id );
    if ( itr == _evm_tokens.end() ) _evm_tokens.emplace( get_self(), insert );
    else _evm_tokens.modify( itr, get_self(), insert );
}

uint64_t oracle::get_account_id( const bytes address )
{
    evm_contract::account_table _account( "eosio.evm"_n, "eosio.evm"_n.value );

    // bytes address_bytes = *silkworm::from_hex(address);
    auto idx = _account.get_index<"by.address"_n>();
    auto it = idx.find(make_key(address));
    auto itr = _account.find( it->id );
    check( it != idx.end(), "evm_contract::get_account_id: [address=" + silkworm::to_hex(address, true) + "] account not found" );
    return itr->id;
}