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

    // CONTRACTS
    const name ORACLE_CONTRACT = "oracle.yield"_n;
    const name CHECK_CONTRACT = "check.yield"_n;
    const name EVM_CONTRACT = "eosio.evm"_n;

    // TOKEN
    const name TOKEN_CONTRACT = "eosio.token"_n;
    const symbol TOKEN_SYMBOL = symbol{"EOS", 4};

    // CONSTANTS
    const set<name> PROTOCOL_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const uint16_t MAX_ANNUAL_RATE = 1000; // maximum rate of 10%
    const uint32_t YEAR = 31536000; // 365 days in seconds
    const uint32_t TEN_MINUTES = 600; // 10 minutes in seconds
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;

    // ERROR MESSAGES
    const string ERROR_CONFIG_NOT_EXISTS = "yield::error: contract is under maintenance";

    // STRUCTS
    struct Contracts {
        set<name>       eos;
        set<string>     evm;
    };

    /**
     * ## TABLE `config`
     *
     * - `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
     * - `{int64_t} min_eos_tvl_report` - minimum EOS TVL report (precision 4)
     * - `{int64_t} max_eos_tvl_report` - maximum EOS TVL report (precision 4)
     * - `{set<name>} metadata_keys` - list of allowed metadata keys
     *
     * ### example
     *
     * ```json
     * {
     *     "annual_rate": 5000,
     *     "min_eos_tvl_report": 200'000'0000,
     *     "max_eos_tvl_report": 6'000'000'0000,
     *     "metadata_keys": ["name", "url", "defillama", "dappradar", "recover"]
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
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
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{set<name>} contracts.eos` - additional supporting EOS contracts
     * - `{set<string>} contracts.evm` - additional supporting EVM contracts
     * - `{extended_asset} balance` - balance available to be claimed
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
     *     "status": "active",
     *     "contracts": {
     *       "eos": ["myprotocol", "mytreasury"],
     *       "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
     *     },
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00",
     *     "period_at": "1970-01-01T00:00:00",
     *     "metadata": [{"key": "url", "value": "https://myprotocol.com"}]
     * }
     * ```
     */
    struct [[eosio::table("protocols")]] protocols_row {
        name                    protocol;
        name                    status = "pending"_n;
        Contracts               contracts;
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
     * $ cleos push action eosio.yield regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void regprotocol( const name protocol, const map<name, string> metadata );

    /**
     * ## ACTION `unregister`
     *
     * > Un-registry protocol
     *
     * - **authority**: `protocol`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield unregister '[myprotocol]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void unregister( const name protocol );

    /**
     * ## ACTION `setcontracts`
     *
     * > Set contracts
     *
     * - **authority**: `protocol`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{set<name>} eos` - EOS contracts
     * - `{set<string>} evm` - EVM contracts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setcontracts '[myprotocol, ["myprotocol"], ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> eos, const set<string> evm );

    /**
     * ## ACTION `claim`
     *
     * > Claim TVL rewards
     *
     * - **authority**: `protocol`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{name} [receiver=""]` - (optional) receiver of rewards (default=protocol)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield claim '[myprotocol, "myreceiver"]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void claim( const name protocol, const optional<name> receiver );

    /**
     * ## ACTION `approve`
     *
     * > Approve protocol
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol to approve
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield approve '["myprotocol"]' -p eosio.yield@admin
     * ```
     */
    [[eosio::action]]
    void approve( const name protocol );

    /**
     * ## ACTION `deny`
     *
     * > Deny protocol
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol to deny
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield deny '["myprotocol"]' -p eosio.yield@admin
     * ```
     */
    [[eosio::action]]
    void deny( const name protocol );

    /**
     * ## ACTION `setrate`
     *
     * > Set rewards rate
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
     * - `{int64_t} min_eos_tvl_report` - minimum EOS TVL report (precision 4)
     * - `{int64_t} max_eos_tvl_report` - maximum EOS TVL report (precision 4)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setrate '[5000, 2000000000, 60000000000]' -p eosio.yield
     * ```
     */
    [[eosio::action]]
    void setrate( const int16_t annual_rate, const int64_t min_eos_tvl_report, const int64_t max_eos_tvl_report );

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
     * $ cleos push action eosio.yield setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p eosio.yield@admin
     * ```
     */
    [[eosio::action]]
    void setmetakeys( const set<name> metadata_keys );

    /**
     * ## ACTION `report`
     *
     * - **authority**: `oracle.yield`
     *
     * Report TVL from oracle
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{time_point_sec} period` - period time
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     *
     * ### example
     *
     * ```bash
     * $ cleos push action eosio.yield report '["myprotocol", "2022-05-13T00:00:00", 30000000, 20000000]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void report( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos );

    /**
     * ## ACTION `claimlog`
     *
     * > Claim logging
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{name} receiver` - receiver of rewards
     * - `{asset} claimed` - claimed funds
     *
     * ### Example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "receiver": "myreceiver",
     *     "claimed": "1.5500 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void claimlog( const name protocol, const name receiver, const asset claimed );

    /**
     * ## ACTION `reportlog`
     *
     * > Balance logging
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{time_point_sec} period` - period time
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     * - `{asset} rewards` - TVL rewards
     * - `{asset} balance_before` - balance before
     * - `{asset} balance_after` - balance after
     *
     * ### Example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "period", "2022-05-13T00:00:00",
     *     "usd": 30000000,
     *     "eos": 20000000,
     *     "rewards": "2.5500 EOS"
     *     "balance_before": "1.0000 EOS",
     *     "balance_after": "1.5500 EOS",
     * }
     * ```
     */
    [[eosio::action]]
    void reportlog( const name protocol, const time_point_sec period, const int64_t usd, const int64_t eos, const asset rewards, const asset before, const asset after );

    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const std::string memo );

    // action wrappers
    using regprotocol_action = eosio::action_wrapper<"regprotocol"_n, &yield::regprotocol>;
    using unregister_action = eosio::action_wrapper<"unregister"_n, &yield::unregister>;
    using setcontracts_action = eosio::action_wrapper<"setcontracts"_n, &yield::setcontracts>;
    using claim_action = eosio::action_wrapper<"claim"_n, &yield::claim>;
    using approve_action = eosio::action_wrapper<"approve"_n, &yield::approve>;
    using deny_action = eosio::action_wrapper<"deny"_n, &yield::deny>;
    using setrate_action = eosio::action_wrapper<"setrate"_n, &yield::setrate>;
    using setmetakeys_action = eosio::action_wrapper<"setmetakeys"_n, &yield::setmetakeys>;
    using report_action = eosio::action_wrapper<"report"_n, &yield::report>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &yield::claimlog>;
    using reportlog_action = eosio::action_wrapper<"reportlog"_n, &yield::reportlog>;

private :
    // utils
    config_row get_config();
    void set_status( const name protocol, const name status );
    void transfer( const name from, const name to, const extended_asset value, const string& memo );
    void check_metadata_keys(const map<name, string> metadata );
};