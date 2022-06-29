// @debug
template <typename T>
void oracle::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void oracle::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    oracle::config_table _config( get_self(), value );
    oracle::tokens_table _tokens( get_self(), value );
    oracle::periods_table _periods( get_self(), value );
    oracle::oracles_table _oracles( get_self(), value );

    if (table_name == "tokens"_n) clear_table( _tokens, rows_to_clear );
    else if (table_name == "periods"_n) clear_table( _periods, rows_to_clear );
    else if (table_name == "oracles"_n) clear_table( _oracles, rows_to_clear );
    else if (table_name == "config"_n) _config.remove();
    else check(false, "oracle::cleartable: [table_name] unknown table to clear" );
}

// @debug
[[eosio::action]]
void oracle::addbalance( const name oracle, const asset quantity )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get(oracle.value, "oracle::addbalance: [oracle] does not exists");
    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity += quantity;
    });
}