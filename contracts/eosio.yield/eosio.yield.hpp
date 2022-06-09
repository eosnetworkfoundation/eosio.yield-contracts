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

    // BASE SYMBOLS
    const symbol EOS = symbol{"EOS", 4};
    const symbol USD = symbol{"USD", 4};

    // CONSTANTS
    const set<name> PROTOCOL_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const uint16_t MAX_ANNUAL_RATE = 1000; // maximum rate of 10%
    const uint32_t YEAR = 31536000; // 365 days in seconds

    // ERROR MESSAGES
    const string ERROR_CONFIG_NOT_EXISTS = "yield::error: contract is under maintenance";

    /**
     * ## TABLE `config`
     *
     * - `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
     * - `{asset} min_tvl_report` - minimum TVL report
     * - `{asset} max_tvl_report` - maximum TVL report
     * - `{name} oracle_contract` - Yield+ Oracle contract
     * - `{name} admin_contract` - Yield+ admin contract
     * - `{name} evm_contract` - Trust EVM contract
     * - `{extended_symbol} rewards` - Yield+ Rewards reward token
     *
     * ### example
     *
     * ```json
     * {
     *     "annual_rate": 500,
     *     "min_tvl_report": "200000.0000 EOS",
     *     "max_tvl_report": "6000000.0000 EOS",
     *     "rewards": {"symbol": "4,EOS", "contract": "eosio.token"}
     *     "oracle_contract": "oracle.yield",
     *     "admin_contract": "admin.yield",
     *     "evm_contract": "eosio.evm",
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        uint16_t                annual_rate = 500;
        asset                   min_tvl_report;
        asset                   max_tvl_report;
        extended_symbol         rewards;
        name                    oracle_contract;
        name                    admin_contract;
        name                    evm_contract;
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
     * - `{asset} tvl` - reported TVL averaged value in EOS
     * - `{asset} usd` - reported TVL averaged value in USD
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
     *     "contracts": ["myprotocol", "mytreasury"],
     *     "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00",
     *     "period_at": "1970-01-01T00:00:00",
     *     "metadata": [{"key": "type", "value": "swap"}, {"key": "url", "value": "https://myprotocol.com"}]
     * }
     * ```
     */
    struct [[eosio::table("protocols")]] protocols_row {
        name                    protocol;
        name                    status = "pending"_n;
        set<name>               contracts;
        set<string>             evm;
        asset                   tvl;
        asset                   usd;
        extended_asset          balance;
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
     * - **authority**: `protocol` OR `admin.yield`
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
     * - **authority**: `protocol` AND `contracts`
     *
     * ### params
     *
     * - `{name} protocol` - protocol (will be included in EOS contracts)
     * - `{set<name>} contracts` - additional EOS contracts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setcontracts '[myprotocol, ["myvault"]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> contracts );

    /**
     * ## ACTION `setcontracts`
     *
     * > Set contracts
     *
     * - **authority**: `protocol` AND `evm`
     *
     * ### params
     *
     * - `{name} protocol` - protocol (will be included in EOS contracts)
     * - `{set<string>} evm` - additional EVM contracts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setcontracts '[myprotocol, ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setevm( const name protocol, const set<string> evm );

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
     * - **authority**: `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol to approve
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield approve '["myprotocol"]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void approve( const name protocol );

    /**
     * ## ACTION `deny`
     *
     * > Deny protocol
     *
     * - **authority**: `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol to deny
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield deny '["myprotocol"]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void deny( const name protocol );

    /**
     * ## ACTION `init`
     *
     * > Initialize Yield+ rewards contract
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{extended_symbol} rewards` - Yield+ Rewards rewards token
     * - `{name} oracle_contract` - Yield+ Oracle contract
     * - `{name} admin_contract` - Yield+ admin contract
     * - `{name} [evm_contract=""]` - Trust EVM contract
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield init '[["4,EOS", "eosio.token"], oracle.yield, admin.yield, eosio.evm]' -p eosio.yield
     * ```
     */
    [[eosio::action]]
    void init( const extended_symbol rewards, const name oracle_contract, const name admin_contract, const optional<name> evm_contract );

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
     * - `{asset} min_tvl_report` - minimum TVL report
     * - `{asset} max_tvl_report` - maximum TVL report
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setrate '[500, "200000.0000 EOS", "6000000.0000 EOS"]' -p eosio.yield
     * ```
     */
    [[eosio::action]]
    void setrate( const int16_t annual_rate, const asset min_tvl_report, const asset max_tvl_report );

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
     * - `{uint32_t} period_interval` - period interval (in seconds)
     * - `{asset} tvl` - TVL averaged value in EOS
     * - `{asset} usd` - TVL averaged value in USD
     *
     * ### example
     *
     * ```bash
     * $ cleos push action eosio.yield report '["myprotocol", "2022-05-13T00:00:00", 600, "200000.0000 EOS", "300000.0000 USD"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void report( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd );

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
     * - `{extended_asset} claimed` - claimed funds
     *
     * ### Example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "receiver": "myreceiver",
     *     "claimed": {"contract": "eosio.token", "quantity": "1.5500 EOS"}
     * }
     * ```
     */
    [[eosio::action]]
    void claimlog( const name protocol, const name receiver, const extended_asset claimed );

    /**
     * ## ACTION `rewardslog`
     *
     * > Rewards logging
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{time_point_sec} period` - period time
     * - `{uint32_t} period_interval` - period interval (in seconds)
     * - `{asset} tvl` - TVL averaged value in EOS
     * - `{asset} usd` - TVL averaged value in USD
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
     *     "period_interval": 600,
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD",
     *     "rewards": "2.5500 EOS"
     *     "balance_before": "1.0000 EOS",
     *     "balance_after": "1.5500 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void rewardslog( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance_before, const asset balance_after );

    [[eosio::action]]
    void init( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance_before, const asset balance_after );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

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
    using report_action = eosio::action_wrapper<"report"_n, &yield::report>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &yield::claimlog>;
    using rewardslog_action = eosio::action_wrapper<"rewardslog"_n, &yield::rewardslog>;
    using cleartable_action = eosio::action_wrapper<"cleartable"_n, &yield::cleartable>;

private :
    // utils
    time_point_sec get_current_period( const uint32_t period_interval );
    config_row get_config();
    void set_status( const name protocol, const name status );
    void transfer( const name from, const name to, const extended_asset value, const string& memo );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};