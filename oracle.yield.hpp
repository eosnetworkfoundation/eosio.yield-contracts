#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <math.h>

using namespace eosio;
using namespace std;

class [[eosio::contract("oracle.yield")]] oracle : public eosio::contract {
public:
    using contract::contract;

    // CONTRACTS
    const name YIELD_CONTRACT = "eosio.yield"_n;
    const name DELPHI_ORACLE_CONTRACT = "delphioracle"_n;
    const name DEFIBOX_ORACLE_CONTRACT = "oracle.defi"_n;

    // TOKEN
    const name TOKEN_CONTRACT = "eosio.token"_n;
    const symbol TOKEN_SYMBOL = symbol{"EOS", 4};

    // CONSTANTS
    const uint32_t TEN_MINUTES = 600;
    const uint32_t ONE_DAY = 86400;
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;
    const uint8_t PRECISION = 4;
    const symbol EOS = symbol{"EOS", 4};
    const symbol USD = symbol{"USD", 4};

    // STRUCTS
    struct TVL {
        vector<asset>   balances;
        int64_t         usd;
        int64_t         eos;
    };
    struct Contracts {
        set<name>       eos;
        set<string>     evm;
    };

    /**
     * ## TABLE `status`
     *
     * ### params
     *
     * - `{vector<uint32_t>} counters` - counters
     *   - `{uint32_t} counters[0]` - total updates
     * - `{asset} claimed` - total assets claimed
     * - `{time_point_sec} last_updated`
     *
     * ### example
     *
     * ```json
     * {
     *     "counters": [100],
     *     "claimed": "102.5000 EOS",
     *     "last_updated": "2021-04-12T12:23:42"
     * }
     * ```
     */
    struct [[eosio::table("status")]] status_row {
        vector<uint32_t>        counters;
        asset                   claimed;
        time_point_sec          last_updated;
    };
    typedef eosio::singleton< "status"_n, status_row > status_table;

    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status ("ok", "testing", "maintenance")
     * - `{set<name>} metadata_keys` - list of keys allowed to include in bounty Metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "metadata_keys": ["name", "url", "defillama", "dappradar", "recover"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name                    status = "testing"_n;
        set<name>               metadata_keys = {"url"_n};
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `tokens`
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) token symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} defibox_oracle_id` - Defibox oracle ID
     * - `{name} delphi_oracle_id` - Delphi oracle ID
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,EOS",
     *     "contract": "eosio.token"
     *     "defibox_oracle_id": 1,
     *     "delphi_oracle_id": "eosusd"
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol          sym;
        name            contract;
        uint64_t        defibox_oracle_id;
        name            delphi_oracle_id;

        uint64_t primary_key() const { return sym.code().raw(); }
    };
    typedef eosio::multi_index< "tokens"_n, tokens_row> tokens_table;

    /**
     * ## TABLE `tvl`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{time_point_sec} period_at` - last period at time
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     * - `{map<time_point_sec, Balances>} balances` - total assets balances in custody of protocol contracts
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "period_at": "2022-05-13T00:00:00",
     *     "usd": 30000000,
     *     "eos": 20000000,
     *     "tvl": [{
     *         "key": "2022-05-13T00:00:00", {
     *             "balances": ["1000.0000 EOS", "1500.0000 USDT"],
     *             "usd": 30000000,
     *             "eos": 20000000
     *          }
     *      }]
     * }
     * ```
     */
    struct [[eosio::table("tvl")]] tvl_row {
        name                            protocol;
        time_point_sec                  period_at;
        int64_t                         usd;
        int64_t                         eos;
        map<time_point_sec, TVL>        tvl;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "tvl"_n, tvl_row> tvl_table;

    /**
     * ## TABLE `oracles`
     *
     * ### params
     *
     * - `{name} oracle` - primary oracle contract
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{extended_asset} balance` - balance available to be claimed
     * - `{asset} claimed` - total claimed amount
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} claimed_at` - claimed at time
     * - `{map<string, string} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "status": "active",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "claimed": "0.0000 EOS",
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00",
     *     "metadata": [{"key": "url", "value": "https://myoracle.com"}]
     * }
     * ```
     */
    struct [[eosio::table("oracles")]] oracles_row {
        name                    oracle;
        name                    status = "pending"_n;
        extended_asset          balance;
        asset                   claimed;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          claimed_at;
        map<name, string>       metadata;

        uint64_t primary_key() const { return oracle.value; }
    };
    typedef eosio::multi_index< "oracles"_n, oracles_row> oracles_table;

    // @oracle
    [[eosio::action]]
    void update( const name oracle, const name protocol );

    // @oracle
    [[eosio::action]]
    void updateall( const name oracle, const optional<uint16_t> max_rows );

    // @oracle
    [[eosio::action]]
    void regoracle( const name oracle, const map<name, string> metadata );

    // @oracle
    [[eosio::action]]
    void unregister( const name oracle );

    // @admin
    [[eosio::action]]
    void approve( const name oracle );

    // @admin
    [[eosio::action]]
    void deny( const name oracle );

    /**
     * ## ACTION `addtoken`
     *
     * - **authority**: `get_self()`
     *
     * Add token as supported asset
     *
     * ### params
     *
     * - `{symbol_code} symcode` - token symbol code
     * - `{name} contract` - token contract
     * - `{uint64_t} [defibox_oracle_id=""]` - (optional) Defibox oracle ID
     * - `{name} [delphi_oracle_id=""]` - (optional) Delphi oracle ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield addtoken '["4,EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void addtoken( const symbol_code symcode, const name contract, const optional<uint64_t> defibox_oracle_id, const optional<name> delphi_oracle_id );

    /**
     * ## ACTION `deltoken`
     *
     * - **authority**: `get_self()`
     *
     * Delete token as supported asset
     *
     * ### params
     *
     * - `{symbol_code} symcode` - token symbol code
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield deltoken '["EOS"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void deltoken( const symbol_code symcode );

    /**
     * ## ACTION `report`
     *
     * - **authority**: `get_self()`
     *
     * Delete token as supported asset
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{time_point_sec} period` - period time
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield report '["mydapp", "2022-05-13T00:00:00", 30000000, 20000000]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void report( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos );

    [[eosio::on_notify("eosio.yield::approve")]]
    void on_approve( const name protocol );

    [[eosio::on_notify("eosio.yield::deny")]]
    void on_deny( const name protocol );

    [[eosio::on_notify("eosio.yield::unregister")]]
    void on_unregister( const name protocol );

    // action wrappers
    using report_action = eosio::action_wrapper<"report"_n, &oracle::report>;

private:
    // get time periods
    time_point_sec get_current_period();

    // get balances
    asset get_balance_quantity( const name token_contract_account, const name owner, const symbol sym );
    asset get_eos_staked( const name owner );

    // notifiers
    void erase_protocol( const name protocol );
    void register_protocol( const name protocol );

    // calculate prices
    int64_t calculate_usd_value( const asset quantity );
    int64_t convert_usd_to_eos( const int64_t usd );
    int64_t get_oracle_price( const symbol_code symcode );
    int64_t normalize_price( const int64_t price, const uint8_t precision );
    int64_t get_delphi_price( const name delphi_oracle_id );
    int64_t get_defibox_price( const uint64_t defibox_oracle_id );
};