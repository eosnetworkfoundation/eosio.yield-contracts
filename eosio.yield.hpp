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

namespace eosio {

class [[eosio::contract("eosio.yield")]] yield : public eosio::contract {
public:
    using contract::contract;

    // const uint8_t TOKEN_PRECISION = 6;
    // const uint16_t MAX_MARKET_FEE = 600;
    // const uint16_t MAX_PROTOCOL_FEE = 200;
    // const int64_t MAX_SUPPLY = 1'000'000'000'000000; // 1M
    // const double ANNUAL_YIELD = 0.0005;

    // const name ORACLE_YIELD_CONTRACT = "testorayield"_n;

    const name TOKEN_CONTRACT = "eosio.token"_n;
    const symbol TOKEN_SYMBOL = symbol{"EOS", 4};
    const set<name> SYSTEM_STATUS_TYPES = set<name>{"maintenance"_n, "active"_n};
    const set<name> PROTOCOL_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const int64_t MAX_ANNUAL_RATE = 1000;

    // CONSTANTS
    const int32_t ANNUAL_YIELD = 500;
    const uint32_t YEAR = 31536000;
    const uint32_t DAY = 86400;
    const uint32_t MINUTE = 60;

    // ERROR MESSAGES
    const string ERROR_CONFIG_NOT_EXISTS = "yield::error: contract is under maintenance";

    /**
     * ## TABLE `status`
     *
     * ### params
     *
     * - `{vector<uint32_t>} counters` - counters
     *   - `{uint32_t} counters[0]` - total protocols
     *   - `{uint32_t} counters[1]` - total approved
     *   - `{uint32_t} counters[2]` - total claims
     * - `{extended_asset} claimed` - total assets claimed
     * - `{time_point_sec} last_updated`
     *
     * ### example
     *
     * ```json
     * {
     *     "counters": [100, 12, 30],
     *     "claimed": {"quantity": "102.5000 EOS", "contract": "eosio.token"},
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
     * - `{set<name>} metadata_keys` - list of keys allowed to include in bounty Metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "status": "ok",
     *     "annual_rate": 5000,
     *     "metadata_keys": ["name", "url", "defillama", "dappradar", "recover"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name            status = "testing"_n;
        uint16_t        annual_rate = 5000;
        name            metadata_keys = {"url"_n};
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `protocols`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{set<name>} contracts` - additional supporting contracts
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{asset} balance` - active balance available to be claimed
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} claimed_at` - claimed at time
     * - `{map<string, string} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "owner": "contract",
     *     "contracts": ["contract", "a.contract", "b.contract"],
     *     "status": "active",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00"
     *     "metadata": [{"key": "url", "value": "https://mywebsite.com"}],
     * }
     * ```
     */
    struct [[eosio::table("protocols")]] protocols_row {
        name                    protocol;
        set<name>               contracts;
        name                    status = "pending"_n;
        extended_asset          balance;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          claimed_at;
        map<string, string>     metadata;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "protocols"_n, protocol> protocols;

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
     * - `{map<string, string>} metadata` - (optional) key/value
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield regprotocol '[mycontract, [{"key": "url", "value":"https://mycontract.com"}]]' -p mycontract
     * ```
     */
    [[eosio::action]]
    void regprotocol( const name protocol, const map<string, string> metadata );

    // @protocol
    [[eosio::action]]
    void unregister( const name protocol );

    // @protocol
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> contracts );

    // @protocol
    [[eosio::action]]
    void claim( const name protocol, const optional<name> receiver );

    // @admin
    [[eosio::action]]
    void setstatus( const name protocol, const name status );

    // @system
    [[eosio::action]]
    void setrate( const int64 annual_rate );

    // @system
    [[eosio::action]]
    void setmetakeys( const map<string, string> metadata_keys );

    // @system
    [[eosio::action]]
    void updatetvl( const name protocol, const asset tvl );

private :

    yield::configs_row get_configs();

    // //INTERNAL FUNCTIONS DEFINITION

    // asset get_oracle_tvl(name contract);
    // //tier get_tier_from_tvl(asset tvl );
    // asset get_contract_balance();
    // asset calculate_incentive_reward(asset tvl);

    // // double asset_to_double(asset a);


};