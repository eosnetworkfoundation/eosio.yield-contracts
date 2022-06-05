[[eosio::action]]
void yield::clear( const name table, const optional<name> scope )
{
    require_auth( get_self() );

    // multi-indexes
    protocols_table _protocols( get_self(), scope ? scope->value : get_self().value );

    // singleton
    status_table _status( get_self(), get_self().value );
    config_table _config( get_self(), get_self().value );

    if ( table == "protocols"_n ) clear_table( _protocols );
    else if ( table == "status"_n ) _status.remove();
    else if ( table == "config"_n ) _config.remove();
    else check( false, "invalid table name");
}

template <typename T>
void yield::clear_table( T& table )
{
    auto itr = table.begin();
    while ( itr != table.end() ) {
        itr = table.erase( itr );
    }
}
