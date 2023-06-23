// @system
[[eosio::action]]
void oracle::addevmtoken( const bytes address, const uint8_t decimals, const symbol sym )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );
    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );
    evm_contract::account_table _account( get_self(), get_self().value );

    // validate
    const uint64_t account_id = evm_contract::get_account_id(address);

    if (!is_stable(sym)) {
        const auto token = _tokens.get( sym.code().raw(), "oracle::addevmtoken: [sym] token not found" );
        check( token.sym == sym, "oracle::addevmtoken: [sym] does not match with existing token");
    }

    // add supported token
    auto insert = [&]( auto& row ) {
        row.token_id = account_id;
        row.address = address;
        row.decimals = decimals;
        row.sym = sym;
    };

    // modify or create
    auto itr = _evm_tokens.find( account_id );
    if ( itr == _evm_tokens.end() ) _evm_tokens.emplace( get_self(), insert );
    else _evm_tokens.modify( itr, get_self(), insert );
}

// @system
[[eosio::action]]
void oracle::delevmtoken( const bytes address )
{
    require_auth( get_self() );

    const uint64_t token_id = evm_contract::get_account_id( address );
    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );
    auto & itr = _evm_tokens.get( token_id, "oracle::delevmtoken: [address] does not exists" );
    _evm_tokens.erase( itr );
}


// @callback
[[eosio::action]]
void oracle::setbalance( const bytes contract, const bytes address, const asset balance )
{
    require_auth( get_self() );

    oracle::evm_tokens_table _evm_tokens( get_self(), get_self().value );

    // validate
    const uint64_t token_id = evm_contract::get_account_id(contract);
    const uint64_t address_id = evm_contract::get_account_id(address);
    _evm_tokens.get( token_id, "oracle::setbalance: [token_id] token not found" );

    // add supported token
    auto insert = [&]( auto& row ) {
        row.address_id = address_id;
        row.address = address;
        row.balance = balance;
    };

    // modify or create
    oracle::evm_balances_table _evm_balances( get_self(), token_id );
    auto itr = _evm_balances.find( address_id );
    if ( itr == _evm_balances.end() ) _evm_balances.emplace( get_self(), insert );
    else _evm_balances.modify( itr, get_self(), insert );
}

asset oracle::get_evm_balance_quantity(const uint64_t token_id, const string evm_contract, const symbol sym )
{
    oracle::evm_balances_table _evm_balances( get_self(), token_id );
    const uint64_t address_id = evm_contract::get_account_id( *silkworm::from_hex(evm_contract) );

    const auto itr = _evm_balances.find( address_id );
    if ( itr == _evm_balances.end() ) return { 0, sym };
    check( itr->balance.symbol == sym, "oracle::get_evm_balance_quantity: [sym] does not match");
    return itr->balance;
}