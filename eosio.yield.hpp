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

class [[eosio::contract("eosio.yield")]] yield : public eosio::contract {
public:
    using contract::contract;

    // CONSTANTS
    const name ORACLE_CONTRACT = "oracle.yield"_n;
    const name EVM_CONTRACT = "eosio.evm"_n;
    const name TOKEN_CONTRACT = "eosio.token"_n;
    const symbol TOKEN_SYMBOL = symbol{"EOS", 4};
    const set<name> SYSTEM_STATUS_TYPES = set<name>{"maintenance"_n, "active"_n};
    const set<name> PROTOCOL_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const int64_t MAX_ANNUAL_RATE = 1000;
    const int32_t ANNUAL_YIELD = 500;
    const uint32_t YEAR = 31536000;
    const uint32_t DAY = 86400;
    const uint32_t MINUTE = 60;
    const uint32_t TEN_MINUTES = 600;
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;

    // ERROR MESSAGES
    const string ERROR_CONFIG_NOT_EXISTS = "yield::error: contract is under maintenance";

    // STRUCTS
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
     *   - `{uint32_t} counters[0]` - total protocols
     *   - `{uint32_t} counters[1]` - total approved
     *   - `{uint32_t} counters[2]` - total claims
     * - `{asset} claimed` - total assets claimed
     * - `{time_point_sec} last_updated`
     *
     * ### example
     *
     * ```json
     * {
     *     "counters": [100, 12, 30],
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
     * - `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
     * - `{int64_t} min_eos_tvl_report` - minimum EOS TVL report (precision 4)
     * - `{int64_t} max_eos_tvl_report` - maximum EOS TVL report (precision 4)
     * - `{set<name>} metadata_keys` - list of keys allowed to include in bounty Metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "annual_rate": 5000,
     *     "min_eos_tvl_report": 200'000'0000,
     *     "max_eos_tvl_report": 6'000'000'0000,
     *     "metadata_keys": ["name", "url", "defillama", "dappradar", "recover"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name                    status = "testing"_n;
        uint16_t                annual_rate = 5000;
        int64_t                 min_eos_tvl_report = 200'000'0000;
        int64_t                 max_eos_tvl_report = 6'000'000'0000;
        set<name>               metadata_keys = {"url"_n};
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `protocols`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{set<name>} contracts.eos` - additional supporting EOS contracts
     * - `{set<string>} contracts.evm` - additional supporting EVM contracts
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{extended_asset} balance` - balance available to be claimed
     * - `{asset} claimed` - total claimed amount
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} claimed_at` - claimed at time
     * - `{time_point_sec} period_at` - period at time
     * - `{map<string, string} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "contracts": {
     *       "eos": ["myprotocol", "mytreasury"],
     *       "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
     *     },
     *     "status": "active",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "claimed": "0.0000 EOS",
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00",
     *     "period_at": "1970-01-01T00:00:00",
     *     "metadata": [{"key": "url", "value": "https://mywebsite.com"}],
     * }
     * ```
     */
    struct [[eosio::table("protocols")]] protocols_row {
        name                    protocol;
        Contracts               contracts;
        name                    status = "pending"_n;
        extended_asset          balance;
        asset                   claimed;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          claimed_at;
        time_point_sec          period_at;
        map<name, string>       metadata;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "protocols"_n, protocols_row> protocols_table;

    /**
     * ## ACTION `regprotocol`
     *
     * > Registry protocol
     *
     * - **authority**: `protocol`
     *
     * ### params
     *
     * - `{name} protocol` - protocol main contract
     * - `{map<name, string>} metadata` - (optional) key/value
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield regprotocol '[mycontract, [{"key": "url", "value":"https://mycontract.com"}]]' -p mycontract
     * ```
     */
    [[eosio::action]]
    void regprotocol( const name protocol, const map<name, string> metadata );

    // @protocol
    [[eosio::action]]
    void unregister( const name protocol );

    // @protocol or @system
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> eos, const set<string> evm );

    // @protocol
    [[eosio::action]]
    void claim( const name protocol, const optional<name> receiver );

    // @admin
    [[eosio::action]]
    void approve( const name protocol );

    // @admin
    [[eosio::action]]
    void deny( const name protocol );

    // @system
    [[eosio::action]]
    void setrate( const int16_t annual_rate, const int64_t min_eos_tvl_report, const int64_t max_eos_tvl_report );

    // @system
    [[eosio::action]]
    void setmetakeys( const set<name> metadata_keys );

    // @system
    [[eosio::action]]
    void claimlog( const name protocol, const name receiver, const asset claimed );

    [[eosio::on_notify("oracle.yield::report")]]
    void on_report( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos );

    // action wrappers
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &yield::claimlog>;

private :

    config_row get_config();
    void set_status( const name protocol, const name status );
    void transfer( const name from, const name to, const extended_asset value, const string& memo );

    // //INTERNAL FUNCTIONS DEFINITION

    // asset get_oracle_tvl(name contract);
    // //tier get_tier_from_tvl(asset tvl );
    // asset get_contract_balance();
    // asset calculate_incentive_reward(asset tvl);

    // // double asset_to_double(asset a);


};