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
     * ## TABLE `config`
     *
     * - `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
     * - `{asset} min_tvl_report` - minimum TVL report
     * - `{asset} max_tvl_report` - maximum TVL report
     * - `{set<name>} metadata_keys` - list of allowed metadata keys
     *
     * ### example
     *
     * ```json
     * {
     *     "metadata_keys": ["name", "url", "defillama", "dappradar", "recover"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        set<name>               metadata_keys = {"url"_n};
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## ACTION `setmetakeys`
     *
     * > Set allowed metakeys
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{set<name>} metadata_keys` - list of allowed metadata keys
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action admin.yield setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void setmetakeys( const set<name> metadata_keys );

    [[eosio::on_notify("*::regprotocol")]]
    void on_regprotocol( const name protocol, const map<name, string> metadata );

    // action wrappers
    using setmetakeys_action = eosio::action_wrapper<"setmetakeys"_n, &admin::setmetakeys>;

private :
    // admin
    check_metadata_keys(const map<name, string> metadata );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};