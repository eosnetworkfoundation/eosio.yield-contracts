#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <cmath>

#include <optional>
#include <string>

using namespace eosio;
using namespace std;

#include <math.h>

class [[eosio::contract("admin.yield")]] admin : public eosio::contract {
public:
    using contract::contract;

    // CONSTANTS
    const set<name> VALUE_TYPES = set<name>{"string"_n, "integer"_n, "boolean"_n, "ipfs"_n, "url"_n, "urls"_n};

    /**
     * ## TABLE `metakeys`
     *
     * - `{name} key` - metadata key
     * - `{name} type` - value type (ex: string/boolean/ipfs/url)
     * - `{bool} required` - is required (true/false)
     * - `{string} description` - metadata description
     *
     * ### example
     *
     * ```json
     * {
     *     "key": "name",
     *     "type": "string",
     *     "required": true,
     *     "description": "Name of protocol"
     * }
     * ```
     */
    struct [[eosio::table("metakeys")]] metakeys_row {
        name            key;
        name            type;
        bool            required;
        string          description;

        uint64_t primary_key() const { return key.value; }
    };
    typedef eosio::multi_index< "metakeys"_n, metakeys_row > metakeys_table;

    /**
     * ## TABLE `categories`
     *
     * - `{name} category` - category [metadata.type] value
     * - `{string} description` - category description
     *
     * ### example
     *
     * ```json
     * {
     *     "category": "dexes",
     *     "description": "Protocols where you can swap/trade cryptocurrency"
     * }
     * ```
     */
    struct [[eosio::table("categories")]] categories_row {
        name            category;
        string          description;

        uint64_t primary_key() const { return category.value; }
    };
    typedef eosio::multi_index< "categories"_n, categories_row > categories_table;

    /**
     * ## ACTION `setmetakeys`
     *
     * > Set metakey
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} key` - metadata key
     * - `{name} type` - value type (ex: string/boolean/ipfs/url)
     * - `{bool} required` - is required (true/false)
     * - `{string} description` - metadata description
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield setmetakey '[website, url, true, "Protocol website"]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void setmetakey( const name key, const name type, const bool required, const string description );

    /**
     * ## ACTION `setcategory`
     *
     * > Set category
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} category` - category
     * - `{string} description` - category description
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield setcategory '[dexes, "Protocols where you can swap/trade cryptocurrency"]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void setcategory( const name category, const string description );

    /**
     * ## ACTION `delcategory`
     *
     * > Delete category
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} category` - category
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield delcategory '[dexes]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void delcategory( const name category );

    /**
     * ## ACTION `delmetakeys`
     *
     * > Delete metakey
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} key` - metadata key
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield delmetakey '[url]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void delmetakey( const name key );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    [[eosio::on_notify("*::regprotocol")]]
    void on_regprotocol( const name protocol, const map<name, string> metadata );

    [[eosio::on_notify("*::regoracle")]]
    void on_regoracle( const name oracle, const map<name, string> metadata );

    [[eosio::on_notify("*::setcategory")]]
    void on_setcategory( const name protocol, const name category );

    // action wrappers
    using setmetakey_action = eosio::action_wrapper<"setmetakey"_n, &admin::setmetakey>;
    using setcategory_action = eosio::action_wrapper<"setcategory"_n, &admin::setcategory>;
    using delcategory_action = eosio::action_wrapper<"delcategory"_n, &admin::delcategory>;
    using delmetakey_action = eosio::action_wrapper<"delmetakey"_n, &admin::delmetakey>;

private :
    // admin
    void check_metadata_keys(const map<name, string> metadata );
    void check_category( const name category );
    void check_metakey( const name key );
    void check_value( const name key, const string value );
    name parse_name( const string& str );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};