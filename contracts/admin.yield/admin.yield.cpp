#include <admin.yield/admin.yield.hpp>

// @system
[[eosio::action]]
void admin::setmetakey( const name key, const name type, const bool required, const string description )
{
    require_auth( get_self() );

    admin::metakeys_table _metakeys( get_self(), get_self().value );

    check( VALUE_TYPES.find( type ) != VALUE_TYPES.end(), "admin::setmetakey: [type] is invalid");

    auto insert = [&]( auto& row ) {
        row.key = key;
        row.type = type;
        row.required = required;
        row.description = description;
    };

    // modify or create
    auto itr = _metakeys.find( key.value );
    if ( itr == _metakeys.end() ) _metakeys.emplace( get_self(), insert );
    else _metakeys.modify( itr, get_self(), insert );
}

[[eosio::action]]
void admin::setcategory( const name category, const string description )
{
    require_auth( get_self() );

    admin::categories_table _categories( get_self(), get_self().value );

    auto insert = [&]( auto& row ) {
        row.category = category;
        row.description = description;
    };

    // modify or create
    auto itr = _categories.find( category.value );
    if ( itr == _categories.end() ) _categories.emplace( get_self(), insert );
    else _categories.modify( itr, get_self(), insert );
}

// @system
[[eosio::action]]
void admin::delmetakey( const name key )
{
    require_auth( get_self() );

    admin::metakeys_table _metakeys( get_self(), get_self().value );
    auto & itr  = _metakeys.get( key.value, "admin.yield::delmetakey: [key] does not exists");
    _metakeys.erase( itr );
}

// @system
[[eosio::action]]
void admin::delcategory( const name category )
{
    require_auth( get_self() );

    admin::categories_table _categories( get_self(), get_self().value );
    auto & itr  = _categories.get( category.value, "admin.yield::delcateogry: [category] does not exists");
    _categories.erase( itr );
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

[[eosio::on_notify("*::setcategory")]]
void admin::on_setcategory( const name protocol, const name category )
{
    check_category( category );
}

void admin::check_metadata_keys( const map<name, string> metadata )
{
    admin::metakeys_table _metakeys( get_self(), get_self().value );

    // validate all keys in metadata
    for ( const auto item : metadata ) {
        const name key = item.first;
        const string value = item.second;

        // validate key/value
        check_metakey( key, value );
    }

    // check for missing required keys
    for ( const auto row : _metakeys ) {
        if ( !row.required ) continue;
        check( metadata.find(row.key) != metadata.end(), "admin.yield::check_metadata_keys: [key=" + row.key.to_string() + "] is required and missing");
    }
}

void admin::check_value( const name key, const name type, const string value )
{
    // validate length of value
    int maxsize = 256;
    if ( type == "text"_n || type == "urls" ) maxsize = 10240;
    check( value.size() <= maxsize, "admin.yield::check_metadata_keys: value exceeds " + std::to_string(maxsize) + " bytes [metadata_key=" + key.to_string() + "]");
}

void admin::check_metakey( const name key, const string value )
{
    admin::metakeys_table _metakeys( get_self(), get_self().value );
    auto itr = _metakeys.find( key.value );
    check( itr != _metakeys.end(), "admin.yield::get_metakey: [key=" + key.to_string() + "] is not valid");

    check_value( key, itr->type, value );
}

void admin::check_category( const name category )
{
    admin::categories_table _categories( get_self(), get_self().value );
    check( category.value, "admin.yield::check_category: [category] must not be empty");

    auto itr = _categories.find( category.value );
    check( itr != _categories.end(), "admin.yield::check_category: [category=" + category.to_string() + "] is not valid (ex: `dexes`)");
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
    else check(false, "admin.yield::cleartable: [table_name] unknown table to clear" );
}

name admin::parse_name(const string& str)
{
    if (str.length() == 0 || str.length() > 12) return {};
    int i=0;
    for (const auto c: str) {
        if((c >= 'a' && c <= 'z') || ( c >= '0' && c <= '5') || c == '.') {
            if(i == 0 && ( c >= '0' && c <= '5') ) return {};   //can't start with a digit
            if(i == 11 && c == '.') return {};                  //can't end with a .
        }
        else return {};
        i++;
    }
    return name{str};
}