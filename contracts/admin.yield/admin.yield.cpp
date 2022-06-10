// self
#include <admin.yield/admin.yield.hpp>

// @system
[[eosio::action]]
void admin::setmetakeys( const set<name> metadata_keys )
{
    require_auth( get_self() );

    admin::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    config.metadata_keys = metadata_keys;
    _config.set(config, get_self());
}

[[eosio::on_notify("*::regprotocol")]]
void admin::on_regprotocol( const name protocol, const map<name, string> metadata )
{
    check_metadata_keys( metadata );
}

[[eosio::on_notify("*::regoracle")]]
void admin::on_regoracle( const name oracle, const map<name, string> metadata )
{
    check_metadata_keys( metadata );
}

void admin::check_metadata_keys(const map<name, string> metadata )
{
    const auto config = get_config();
    const set<name> metadata_keys = config.metadata_keys;
    for ( const auto item : metadata ) {
        const name key = item.first;
        const string value = item.second;
        const int maxsize = key == "description"_n ? 10240 : 256;
        check( value.size() <= maxsize, get_self().to_string() + "::check_metadata_keys: value exceeds " + std::to_string(maxsize) + " bytes [metadata_key=" + key.to_string() + "]");
        check( metadata_keys.find(key) != metadata_keys.end(), get_self().to_string() + "::check_metadata_keys: invalid [metadata_key=" + key.to_string() + "]");
    }
}

admin::config_row admin::get_config()
{
    admin::config_table _config( get_self(), get_self().value );
    check( _config.exists(), get_self().to_string() + "::get_config: contract is not initialized");
    return _config.get();
}

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
    admin::config_table _config( get_self(), value );
    else if (table_name == "config"_n) _config.remove();
    else check(false, get_self().to_string() + "::cleartable: [table_name] unknown table to clear" );
}
