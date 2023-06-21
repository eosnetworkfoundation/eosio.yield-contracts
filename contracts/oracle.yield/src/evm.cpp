// @system
[[eosio::action]]
void oracle::addevmtoken( const symbol_code symcode, const string address, const uint8_t decimals );
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );
    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );
    evm_contract::account_table _account( get_self(), get_self().value );

    // validate
    const auto token = _tokens.get( symcode.raw(), "oracle::addevmtoken: [symcode] token not found" );
    const account_id = evm_contract::get_account_id(address);

    // add supported token
    auto insert = [&]( auto& row ) {
        row.account_id = account_id;
        row.sym = token.sym;
        row.address = address;
        row.decimals = decimals;
    };

    // modify or create
    auto itr = _evm_tokens.find( account_id );
    if ( itr == _evm_tokens.end() ) _evm_tokens.emplace( get_self(), insert );
    else _evm_tokens.modify( itr, get_self(), insert );
}

