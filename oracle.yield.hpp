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
    const name NOTIFY_CONTRACT = "notify.yield"_n;
    const name EVM_CONTRACT = "eosio.evm"_n;

    // TOKEN
    const symbol EOS = symbol{"EOS", 4};
    const symbol USD = symbol{"USD", 4};
    const name TOKEN_CONTRACT = "eosio.token"_n;
    const symbol TOKEN_SYMBOL = EOS;

    // CONSTANTS
    const set<name> SYSTEM_STATUS_TYPES = set<name>{"maintenance"_n, "active"_n};
    const set<name> ORACLE_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const uint32_t TEN_MINUTES = 600;
    const uint32_t ONE_DAY = 86400;
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;
    const uint8_t PRECISION = 4;

    // STRUCTS
    struct TVL {
        asset           usd;
        asset           eos;
    };
    struct Contracts {
        set<name>       eos;
        set<string>     evm;
    };
    struct History {
        vector<asset>   balances;
        TVL             tvl;
    };

    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status ("ok", "maintenance")
     * - `{asset} reward_per_update` - reward per update (ex: "0.0200 EOS")
     * - `{set<name>} metadata_keys` - list of allowed metadata keys
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "reward_per_update": "0.0200 EOS",
     *     "metadata_keys": ["name", "url"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name                status = "maintenance"_n;
        asset               reward_per_update = {200, symbol{"EOS", 4}};
        set<name>           metadata_keys = {"url"_n};
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
     * - `{map<time_point_sec, TVL>} history` - historical assets balances by timestamp
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "period_at": "2022-05-13T00:00:00",
     *     "history": [{
     *         "key": "2022-05-13T00:00:00", {
     *             "balances": ["1000.0000 EOS", "1500.0000 USDT"],
     *             "tvl": {
     *                 "usd": "300000.0000 USD",
     *                 "eos": "200000.0000 EOS"
     *              }
     *          }
     *      }]
     * }
     * ```
     */
    struct [[eosio::table("tvl")]] tvl_row {
        name                            protocol;
        time_point_sec                  period_at;
        map<time_point_sec, History>    history;

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

    // @eosio.code
    [[eosio::action]]
    void updatelog( const name oracle, const name protocol, const time_point_sec period, const vector<asset> balances, const TVL tvl );

    // @system
    [[eosio::action]]
    void setreward( const asset reward_per_update );

    // @system
    [[eosio::action]]
    void setmetakeys( const set<name> metadata_keys );

    [[eosio::on_notify("eosio.yield::approve")]]
    void on_approve( const name protocol );

    [[eosio::on_notify("eosio.yield::deny")]]
    void on_deny( const name protocol );

    [[eosio::on_notify("eosio.yield::unregister")]]
    void on_unregister( const name protocol );

    // action wrappers
    using update_action = eosio::action_wrapper<"update"_n, &oracle::update>;
    using updateall_action = eosio::action_wrapper<"updateall"_n, &oracle::updateall>;
    using regoracle_action = eosio::action_wrapper<"regoracle"_n, &oracle::regoracle>;
    using unregister_action = eosio::action_wrapper<"unregister"_n, &oracle::unregister>;
    using approve_action = eosio::action_wrapper<"approve"_n, &oracle::approve>;
    using deny_action = eosio::action_wrapper<"deny"_n, &oracle::deny>;
    using addtoken_action = eosio::action_wrapper<"addtoken"_n, &oracle::addtoken>;
    using deltoken_action = eosio::action_wrapper<"deltoken"_n, &oracle::deltoken>;
    using updatelog_action = eosio::action_wrapper<"updatelog"_n, &oracle::updatelog>;
    using setreward_action = eosio::action_wrapper<"setreward"_n, &oracle::setreward>;
    using setmetakeys_action = eosio::action_wrapper<"setmetakeys"_n, &oracle::setmetakeys>;

private:
    // utils
    time_point_sec get_current_period();
    oracle::config_row get_config();
    void set_status( const name oracle, const name status );
    void check_oracle_active( const name oracle );
    void generate_report( const name protocol, const time_point_sec period, const map<time_point_sec, History> history );

    // getters
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

    int64_t compute_average_tvl( );
};