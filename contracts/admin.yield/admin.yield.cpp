// eosio
#include <eosio.token/eosio.token.hpp>

// core
#include <admin.yield/admin.yield.hpp>

// used to assert checks based on logging events
#include "src/notifiers.cpp"

// DEBUG (used to help testing)
#ifdef DEBUG
#include "src/debug.cpp"
#endif

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

void admin::check_metadata_keys( const name category, map<name, string> metadata )
{
    admin::metakeys_table _metakeys( get_self(), get_self().value );

    // validate all keys in metadata
    for ( const auto item : metadata ) {
        const name key = item.first;
        const string value = item.second;

        // validate key/value
        check_metakey( key, value );
    }
    // validate token
    const string code = metadata["token.code"_n];
    const string symcode = metadata["token.symcode"_n];
    check_token(code, symcode);

    // ignore remaining validation for oracle
    if ( category == "oracle"_n) return;

    // check for missing required keys
    for ( const auto row : _metakeys ) {
        if ( !row.required ) continue;
        check( metadata.find(row.key) != metadata.end(), "admin.yield::check_metadata_keys: [key=" + row.key.to_string() + "] is required and missing");
    }
}

void admin::check_value( const name key, const name type, const string value )
{
    // validate value based on types
    if ( type == "symcode"_n ) check( parse_symbol_code( value ).raw(), "admin.yield::check_value: invalid symcode value [metadata_key=" + key.to_string() + "]");
    else if ( type == "name"_n ) check( parse_name( value ).value, "admin.yield::check_value: invalid name value [metadata_key=" + key.to_string() + "]");
    else if ( type == "integer"_n ) check( parse_integer( value ) >= 0, "admin.yield::check_value: invalid integer value [metadata_key=" + key.to_string() + "]");

    // validate length of value
    int maxsize = 256;
    if ( type == "text"_n || type == "urls"_n ) maxsize = 10240;
    check( value.size() <= maxsize, "admin.yield::check_value: value exceeds " + std::to_string(maxsize) + " bytes [metadata_key=" + key.to_string() + "]");
}

void admin::check_token( const string code, const string symcode )
{
    if ( !code.size() || !symcode.size() ) return; // skip if no values provided
    const asset supply = eosio::token::get_supply( parse_name( code ), parse_symbol_code( symcode ) );
    check( supply.amount > 0, "admin.yield::check_token: token has no supply");
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
    if ( category == "oracle"_n) return; // skip if oracle
    check( itr != _categories.end(), "admin.yield::check_category: [category=" + category.to_string() + "] is not valid (ex: `dexes`)");
}

name admin::parse_name(const string& str)
{
    if (str.length() == 0 || str.length() > 12) return {};
    int i=0;
    for (const auto c: str) {
        if ((c >= 'a' && c <= 'z') || ( c >= '0' && c <= '5') || c == '.') {
            if(i == 0 && ( c >= '0' && c <= '5') ) return {};   //can't start with a digit
            if(i == 11 && c == '.') return {};                  //can't end with a .
        }
        else return {};
        i++;
    }
    return name{str};
}

int64_t admin::parse_integer(const string& str)
{
    if (str.length() == 0) return {};
    for (const auto c: str) {
        if ( c >= '0' && c <= '9' ) {}
        else return -1;
    }
    return std::stoi( str );
}

symbol_code admin::parse_symbol_code(const string& str)
{
    if (str.size() > 7) return {};
    for (const auto c: str ) {
        if( c < 'A' || c > 'Z') return {};
    }
    const symbol_code sym_code = symbol_code{ str };

    return sym_code.is_valid() ? sym_code : symbol_code{};
}
