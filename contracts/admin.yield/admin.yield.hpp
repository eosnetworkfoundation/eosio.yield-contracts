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

    /**
     * ## TABLE `metakeys`
     *
     * - `{name} key` - metadata key
     * - `{bool} required` - is required (true/false)
     * - `{string} description` - metadata description
     *
     * ### example
     *
     * ```json
     * {
     *     "key": "name",
     *     "required": true,
     *     "description": "Name of protocol"
     * }
     * ```
     */
    struct [[eosio::table("metakeys")]] metakeys_row {
        name            key;
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
     * - `{bool} required` - is required (true/false)
     * - `{string} description` - metadata description
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield setmetakey '[url, true, "Protocol URL"]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void setmetakey( const name key, const bool required, const string description );

    [[eosio::action]]
    void setcategory( const name category, const string description );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    [[eosio::on_notify("*::regprotocol")]]
    void on_regprotocol( const name protocol, const map<name, string> metadata );

    [[eosio::on_notify("*::regoracle")]]
    void on_regoracle( const name oracle, const map<name, string> metadata );

    // action wrappers
    using setmetakey_action = eosio::action_wrapper<"setmetakey"_n, &admin::setmetakey>;
    using setcategory_action = eosio::action_wrapper<"setcategory"_n, &admin::setcategory>;

private :
    // admin
    void check_metadata_keys(const map<name, string> metadata );
    bool is_category( const string category );
    metakeys_row get_metakey( const name key );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};