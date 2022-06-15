#include <admin.yield/admin.yield.hpp>

// @system
[[eosio::action]]
void admin::setmetakey( const name key, const bool required, const string description )
{
    require_auth( get_self() );

    // admin::config_table _config( get_self(), get_self().value );
    // auto config = _config.get_or_default();
    // config.metadata_keys = metadata_keys;
    // _config.set(config, get_self());
}

[[eosio::action]]
void admin::setcategory( const name category, const string description )
{
    require_auth( get_self() );
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

void admin::check_metadata_keys( const map<name, string> metadata )
{
    admin::metakeys_table _metakeys( get_self(), get_self().value );
    // const auto config = get_config();
    // const set<name> metadata_keys = config.metadata_keys;

    // validate all keys in metadata
    for ( const auto item : metadata ) {
        const name key = item.first;
        const string value = item.second;

        // validate length of value
        const int maxsize = key == "description"_n ? 10240 : 256;
        check( value.size() <= maxsize, get_self().to_string() + "::check_metadata_keys: value exceeds " + std::to_string(maxsize) + " bytes [metadata_key=" + key.to_string() + "]");

        // validate key value
        get_metakey( key );
        if ( key == "category"_n ) check( is_category(value), get_self().to_string() + "::check_metadata_keys: [value=" + value + "] is not valid `category`");
    }

    // check for missing required keys
    for ( const auto row : _metakeys ) {
        if ( !row.required ) continue;
        check( metadata.at(row.key).size(), get_self().to_string() + "::check_metadata_keys: [key=" + row.key.to_string() + "] is required and missing");
    }
}

admin::metakeys_row admin::get_metakey( const name key )
{
    admin::metakeys_table _metakeys( get_self(), get_self().value );
    auto itr = _metakeys.find( key.value );
    check( itr != _metakeys.end(), get_self().to_string() + "::get_metakey: [key=" + key.to_string() + "] is not valid");
    return *itr;
}

bool admin::is_category( const string category )
{
    // admin::categories_table _categories( get_self(), get_self().value );
    // check( name{category}.is_valid(), get_self().to_string() + "::is_category: [key=" + key.to_string() + "] is not valid `name` type");

    // auto itr = _categories.find( name{category}.value );
    // check( itr != _categories.end(), get_self().to_string() + "::is_category: [key=" + key.to_string() + "] is not valid");
    return true;
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
    admin::metakeys_table _metakeys( get_self(), value );
    admin::categories_table _categories( get_self(), value );

    if (table_name == "metakeys"_n) clear_table( _metakeys, rows_to_clear );
    else if (table_name == "categories"_n) clear_table( _categories, rows_to_clear );
    else check(false, get_self().to_string() + "::cleartable: [table_name] unknown table to clear" );
}
