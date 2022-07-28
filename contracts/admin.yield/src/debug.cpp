// @debug
template <typename T>
void admin::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void admin::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    admin::metakeys_table _metakeys( get_self(), value );
    admin::categories_table _categories( get_self(), value );

    if (table_name == "metakeys"_n) clear_table( _metakeys, rows_to_clear );
    else if (table_name == "categories"_n) clear_table( _categories, rows_to_clear );
    else check(false, "admin.yield::cleartable: [table_name] unknown table to clear" );
}
