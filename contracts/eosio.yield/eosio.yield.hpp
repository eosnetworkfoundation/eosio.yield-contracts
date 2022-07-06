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

    // EXTERNAL CONTRACTS
    const name EVM_CONTRACT = "eosio.evm"_n;

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
     * - `{extended_symbol} rewards` - rewards token
     * - `{name} oracle_contract` - Yield+ Oracle contract
     * - `{name} admin_contract` - Yield+ admin contract
     *
     * ### example
     *
     * ```json
     * {
     *     "annual_rate": 500,
     *     "min_tvl_report": "200000.0000 EOS",
     *     "max_tvl_report": "6000000.0000 EOS",
     *     "rewards": {"sym": "4,EOS", "contract": "eosio.token"},
     *     "oracle_contract": "oracle.yield",
     *     "admin_contract": "admin.yield"
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        uint16_t                annual_rate = 500;
        asset                   min_tvl_report;
        asset                   max_tvl_report;
        extended_symbol         rewards;
        name                    oracle_contract = "oracle.yield"_n;
        name                    admin_contract = "admin.yield"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `state`
     *
     * - `{set<name>} active_protocols` - array of active protocols
     *
     * ### example
     *
     * ```json
     * {
     *     "active_protocols": ["myprotocol"]
     * }
     * ```
     */
    struct [[eosio::table("state")]] state_row {
        set<name>           active_protocols;
    };
    typedef eosio::singleton< "state"_n, state_row > state_table;

    /**
     * ## TABLE `protocols`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{name} category` - protocol category (ex: `dexes/lending/staking`)
     * - `{set<name>} contracts` - additional supporting EOS contracts
     * - `{set<string>} evm` - additional supporting EVM contracts
     * - `{asset} tvl` - reported TVL averaged value in EOS
     * - `{asset} usd` - reported TVL averaged value in USD
     * - `{extended_asset} balance` - balance available to be claimed
     * - `{map<string, string} metadata` - metadata
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} claimed_at` - claimed at time
     * - `{time_point_sec} period_at` - period at time
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "status": "active",
     *     "category": "dexes",
     *     "contracts": ["myprotocol", "mytreasury"],
     *     "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}],
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00",
     *     "period_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("protocols")]] protocols_row {
        name                    protocol;
        name                    status = "pending"_n;
        name                    category;
        set<name>               contracts;
        set<string>             evm;
        asset                   tvl;
        asset                   usd;
        extended_asset          balance;
        map<name, string>       metadata;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          claimed_at;
        time_point_sec          period_at;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "protocols"_n, protocols_row> protocols_table;

    /**
     * ## ACTION `init`
     *
     * > Initialize the rewards contract
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{extended_symbol} rewards` - Yield+ rewards token
     * - `{name} oracle_contract` - Yield+ oracle contract
     * - `{name} admin_contract` - Yield+ admin contract
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield init '[["4,EOS", "eosio.token"], oracle.yield, admin.yield]' -p eosio.yield
     * ```
     */
    [[eosio::action]]
    void init( const extended_symbol rewards, const name oracle_contract, const name admin_contract );

    /**
     * ## ACTION `setrate`
     *
     * > Set TVL rewards rate at {{annual_rate}} basis points.
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
     * ## ACTION `regprotocol`
     *
     * > Register the {{protocol}} protocol.
     *
     * - **authority**: `protocol`
     *
     * ### params
     *
     * - `{name} protocol` - protocol main contract
     * - `{name} category` - protocol category (dexes/lending/yield)
     * - `{map<name, string>} metadata` - (optional) key/value
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield regprotocol '[myprotocol, dexes, [{"key": "website", "value":"https://myprotocol.com"}]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void regprotocol( const name protocol, const name category, const map<name, string> metadata );

    /**
     * ## ACTION `setmetadata`
     *
     * > Set the metadata for the {{protocol}} protocol.
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
     * $ cleos push action eosio.yield setmetadata '[myprotocol, [{"key": "website", "value":"https://myprotocol.com"}]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setmetadata( const name protocol, const map<name, string> metadata );

    /**
     * ## ACTION `setmetakey`
     *
     * > Set the {{key}} metadata key to {{value}}.
     *
     * - **authority**: `protocol` OR `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol main contract
     * - `{name} key` - metakey (ex: name/website/description)
     * - `{string} [value=null]` - (optional) metakey value (if empty, will erase metakey)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setmetakey '[myprotocol, website, "https://myprotocol.com"]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setmetakey( const name protocol, const name key, const optional<string> value );

    /**
     * ## ACTION `unregister`
     *
     * > Unregister the {{protocol}} protocol.
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
     * > Sets the smart contracts for the {{protocol}} protocol.
     *
     * - **authority**: (`protocol` AND `contracts`) OR `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol (will be included in EOS contracts)
     * - `{set<name>} contracts` - additional EOS contracts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setcontracts '[myprotocol, [myvault]]' -p myprotocol -p myvault
     * ```
     */
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> contracts );

    /**
     * ## ACTION `setevm`
     *
     * > Sets EVM contracts for the {{protocol}} protocol.
     *
     * - **authority**: (`protocol` AND `evm`) OR `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol (will be included in EOS contracts)
     * - `{set<string>} evm` - additional EVM contracts
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setevm '[myprotocol, ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol
     * ```
     */
    [[eosio::action]]
    void setevm( const name protocol, const set<string> evm );

    /**
     * ## ACTION `approve`
     *
     * > Approves the {{protocol}} protocol for the Yield+ rewards program.
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
     * $ cleos push action eosio.yield approve '[myprotocol]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void approve( const name protocol );

    /**
     * ## ACTION `setcategory`
     *
     * > Sets the category of the {{protocol}} protocol.
     *
     * - **authority**: `admin.yield`
     *
     * ### params
     *
     * - `{name} protocol` - protocol to approve
     * - `{name} category` - protocol category (eligible categories in `admin.yield`)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setcategory '[myprotocol, dexes]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void setcategory( const name protocol, const name category );

    /**
     * ## ACTION `deny`
     *
     * > Denies the {{protocol}} protocol for the Yield+ rewards program.
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
     * $ cleos push action eosio.yield deny '[myprotocol]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void deny( const name protocol );

    /**
     * ## ACTION `claim`
     *
     * > Claims the Yield+ rewards for the {{protocol}} protocol.
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
     * $ cleos push action eosio.yield claim '[myprotocol, null]' -p myprotocol
     * //=> rewards sent to myprotocol
     *
     * $ cleos push action eosio.yield claim '[myprotocol, myreceiver]' -p myprotocol
     * //=> rewards sent to myreceiver
     * ```
     */
    [[eosio::action]]
    void claim( const name protocol, const optional<name> receiver );

    /**
     * ## ACTION `report`
     *
     * > Generates a report of the current TVL from the {{protocol}} protocol.
     *
     * - **authority**: `oracle.yield@eosio.code`
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
     * $ cleos push action eosio.yield report '[myprotocol, "2022-05-13T00:00:00", 600, "200000.0000 EOS", "300000.0000 USD"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void report( const name protocol, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd );

    /**
     * ## ACTION `claimlog`
     *
     * > Generates a log each time Yield+ rewards are claimed.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{name} category` - protocol category
     * - `{name} [receiver=""]` - (optional) receiver of rewards
     * - `{asset} claimed` - claimed rewards
     * - `{asset} balance` - balance available to be claimed
     *
     * ### Example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "category": "dexes",
     *     "receiver": "myreceiver",
     *     "claimed": "1.5500 EOS",
     *     "balance": 0.0000 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void claimlog( const name protocol, const name category, const name receiver, const asset claimed, const asset balance );

    /**
     * ## ACTION `rewardslog`
     *
     * > Generates a log when rewards are generated from reports.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     * - `{name} category` - protocol category
     * - `{time_point_sec} period` - period time
     * - `{uint32_t} period_interval` - period interval (in seconds)
     * - `{asset} tvl` - TVL averaged value in EOS
     * - `{asset} usd` - TVL averaged value in USD
     * - `{asset} rewards` - TVL rewards
     * - `{asset} balance` - current claimable balance
     *
     * ### Example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "category": "dexes",
     *     "period": "2022-05-13T00:00:00",
     *     "period_interval": 600,
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD",
     *     "rewards": "2.5500 EOS",
     *     "balance": "10.5500 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void rewardslog( const name protocol, const name category, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance );

    /**
     * ## ACTION `statuslog`
     *
     * > Generates a log when a protocol's status is modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{name} status="pending"` - status (`pending/active/denied`)
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "status": "active",
     * }
     * ```
     */
    [[eosio::action]]
    void statuslog( const name protocol, const name status );

    /**
     * ## ACTION `contractslog`
     *
     * > Generates a log when a protocol's contracts are modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{set<name>} contracts.eos` - additional supporting EOS contracts
     * - `{set<string>} contracts.evm` - additional supporting EVM contracts
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "contracts": ["myprotocol", "mytreasury"],
     *     "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
     * }
     * ```
     */
    [[eosio::action]]
    void contractslog( const name protocol, const set<name> contracts, const set<string> evm );

    /**
     * ## ACTION `categorylog`
     *
     * > Generates a log when a protocol's category is modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{name} category` - protocol category (ex: `dexes/lending/staking`)
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "category": "dexes"
     * }
     * ```
     */
    [[eosio::action]]
    void categorylog( const name protocol, const name category );

    /**
     * ## ACTION `createlog`
     *
     * > Generates a log when a protocol is created.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{name} category` - protocol category (dexes/lending/yield)
     * - `{map<string, string>} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "category": "dexes",
     *     "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}]
     * }
     * ```
     */
    [[eosio::action]]
    void createlog( const name protocol, const name category, const map<name, string> metadata );

    /**
     * ## ACTION `eraselog`
     *
     * > Generates a log when a protocol is erased.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol"
     * }
     * ```
     */
    [[eosio::action]]
    void eraselog( const name protocol );

    /**
     * ## ACTION `metadatalog`
     *
     * > Generates a log when a protocol's metadata is modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} protocol` - primary protocol contract
     * - `{map<string, string>} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "myprotocol",
     *     "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}]
     * }
     * ```
     */
    [[eosio::action]]
    void metadatalog( const name protocol, const map<name, string> metadata );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    // @debug
    [[eosio::action]]
    void addbalance( const name protocol, const asset quantity );

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

    using rewardslog_action = eosio::action_wrapper<"rewardslog"_n, &yield::rewardslog>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &yield::claimlog>;
    using statuslog_action = eosio::action_wrapper<"statuslog"_n, &yield::statuslog>;
    using contractslog_action = eosio::action_wrapper<"contractslog"_n, &yield::contractslog>;
    using categorylog_action = eosio::action_wrapper<"categorylog"_n, &yield::categorylog>;
    using createlog_action = eosio::action_wrapper<"createlog"_n, &yield::createlog>;
    using eraselog_action = eosio::action_wrapper<"eraselog"_n, &yield::eraselog>;
    using metadatalog_action = eosio::action_wrapper<"metadatalog"_n, &yield::metadatalog>;

private :
    // utils
    time_point_sec get_current_period( const uint32_t period_interval );
    config_row get_config();
    void set_status( const name protocol, const name status );
    void set_category( const name protocol, const name category );
    void transfer( const name from, const name to, const extended_asset value, const string& memo );
    void remove_active_protocol( const name protocol );
    void add_active_protocol( const name protocol );
    void notify_admin();
    void require_auth_admin();
    void require_auth_admin( const name account );
    bool is_contract( const name contract );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};