#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <eosio.yield/eosio.yield.hpp>

#include <math.h>

using namespace eosio;
using namespace std;

class [[eosio::contract("oracle.yield")]] oracle : public eosio::contract {
public:
    using contract::contract;

    // EXTERNAL CONTRACTS
    const name EVM_CONTRACT = "eosio.evm"_n;
    const name DELPHI_ORACLE_CONTRACT = "delphioracle"_n;
    const name DEFIBOX_ORACLE_CONTRACT = "oracle.defi"_n;

    // TOKEN
    const symbol EOS = {"EOS", 4};
    const symbol USD = {"USD", 4};
    const symbol USDT = {"USDT", 4};
    const name USDT_CONTRACT = "tethertether"_n;

    // CONSTANTS
    const set<name> ORACLE_STATUS_TYPES = set<name>{"pending"_n, "active"_n, "denied"_n};
    const uint32_t TEN_MINUTES = 600; // 10 minutes (600 seconds)
    const uint64_t EIGHT_HOURS = 28800; // 8 hours (28800 seconds)
    const uint32_t BUCKET_PERIODS = 48; // 8 hours (48 periods);
    const uint32_t MIN_BUCKET_PERIODS = 42; // 7 hours (42 periods);
    const uint32_t MAX_PERIODS_REPORT = 144; // 24 hours (144 periods)
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;
    const uint8_t PRECISION = 4;
    const double MAX_PRICE_DEVIATION = 1000; // 10% (below & above average price)

    /**
     * ## TABLE `config`
     *
     * - `{extended_asset} reward_per_update` - reward per update (ex: "0.0200 EOS")
     * - `{name} yield_contract` - Yield+ core contract
     * - `{name} admin_contract` - Yield+ admin contract
     *
     * ### example
     *
     * ```json
     * {
     *     "reward_per_update": {"contract": "eosio.token", "quantity": "0.0200 EOS"},
     *     "yield_contract": "eosio.yield",
     *     "admin_contract": "admin.yield"
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        extended_asset          reward_per_update;
        name                    yield_contract = "eosio.yield"_n;
        name                    admin_contract = "admin.yield"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `tokens`
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) token symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} [defibox_oracle_id=null]` - (optional) Defibox oracle ID
     * - `{name} [delphi_oracle_id=null]` - (optional) Delphi oracle ID
     * - `{uint64_t} [extra_oracle_id=null]` - (optional) extra oracle ID
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,EOS",
     *     "contract": "eosio.token",
     *     "defibox_oracle_id": 1,
     *     "delphi_oracle_id": "eosusd",
     *     "extra_oracle_id": null
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol                  sym;
        name                    contract;
        optional<uint64_t>      defibox_oracle_id;
        optional<name>          delphi_oracle_id;
        optional<uint64_t>      extra_oracle_id;

        uint64_t primary_key() const { return sym.code().raw(); }
    };
    typedef eosio::multi_index< "tokens"_n, tokens_row> tokens_table;

    /**
     * ## TABLE `periods`
     *
     * - scope: `{name} protocol`
     *
     * ### params
     *
     * - `{time_point_sec} period` - (primary key) period at time
     * - `{name} protocol` - protocol contract
     * - `{name} category` - protocol category
     * - `{set<name>} contracts.eos` - additional supporting EOS contracts
     * - `{set<string>} contracts.evm` - additional supporting EVM contracts
     * - `{vector<asset>} balances` - asset balances
     * - `{vector<asset>} prices` - currency prices
     * - `{asset} tvl` - reported TVL averaged value in EOS
     * - `{asset} usd` - reported TVL averaged value in USD
     *
     * ### example
     *
     * ```json
     * {
     *     "period": "2022-05-13T00:00:00",
     *     "protocol": "myprotocol",
     *     "contracts": ["myprotocol", "mytreasury"],
     *     "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
     *     "balances": ["1000.0000 EOS", "1500.0000 USDT"],
     *     "prices": ["1.5000 USD", "1.0000 USD"],
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD"
     * }
     * ```
     */
    struct [[eosio::table("periods")]] periods_row {
        time_point_sec          period;
        name                    protocol;
        name                    category;
        set<name>               contracts;
        set<string>             evm;
        vector<asset>           balances;
        vector<asset>           prices;
        asset                   tvl;
        asset                   usd;

        uint64_t primary_key() const { return period.sec_since_epoch(); }
    };
    typedef eosio::multi_index< "periods"_n, periods_row> periods_table;

    /**
     * ## TABLE `oracles`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     * - `{name} status="pending"` - status (`pending/active/denied`)
     * - `{extended_asset} balance` - balance available to be claimed
     * - `{map<string, string} metadata` - metadata
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} claimed_at` - claimed at time
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "status": "active",
     *     "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
     *     "metadata": [{"key": "url", "value": "https://myoracle.com"}],
     *     "created_at": "2022-05-13T00:00:00",
     *     "updated_at": "2022-05-13T00:00:00",
     *     "claimed_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("oracles")]] oracles_row {
        name                    oracle;
        name                    status = "pending"_n;
        extended_asset          balance;
        map<name, string>       metadata;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          claimed_at;

        uint64_t primary_key() const { return oracle.value; }
    };
    typedef eosio::multi_index< "oracles"_n, oracles_row> oracles_table;

    /**
     * ## ACTION `init`
     *
     * > Initializes the Yield+ oracle contract
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{extended_symbol} rewards` - Yield+ oracle rewards token
     * - `{name} yield_contract` - Yield+ core contract
     * - `{name} admin_contract` - Yield+ admin contract
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield init '[["4,EOS", "eosio.token"], rewards.yield, admin.yield]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void init( const extended_symbol rewards, const name yield_contract, const name admin_contract );

    /**
     * ## ACTION `addtoken`
     *
     * - **authority**: `get_self()`
     *
     * > Add {{symcode}} token as supported asset.
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
     * $ cleos push action oracle.yield addtoken '["EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void addtoken( const symbol_code symcode, const name contract, const optional<uint64_t> defibox_oracle_id, const optional<name> delphi_oracle_id );

    /**
     * ## ACTION `deltoken`
     *
     * - **authority**: `get_self()`
     *
     * > Delete {{symcode}} token as supported asset
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
     * ## ACTION `setreward`
     *
     * > Set oracle rewards
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{asset} reward_per_update` - reward per update
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield setreward '["0.0200 EOS"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void setreward( const asset reward_per_update );

    /**
     * ## ACTION `regoracle`
     *
     * > Registers the {{oracle}} oracle with the Yield+ oracle contract
     *
     * - **authority**: `oracle`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     * - `{map<string, string} metadata` - metadata
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value": "https://myoracle.com"}]]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void regoracle( const name oracle, const map<name, string> metadata );

    /**
     * ## ACTION `unregister`
     *
     * > Unregisters the {{oracle}} oracle from the Yield+ oracle contract
     *
     * - **authority**: `oracle`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield unregister '[myoracle]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void unregister( const name oracle );

    /**
     * ## ACTION `setmetadata`
     *
     * > Set metadata for the {{oracle}} oracle
     *
     * - **authority**: `oracle` OR `admin.yield`
     *
     * ### params
     *
     * - `{name} oracle` - oracle main contract
     * - `{map<name, string>} metadata` - (optional) key/value
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setmetadata '[myoracle, [{"key": "website", "value":"https://myoracle.com"}]]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void setmetadata( const name oracle, const map<name, string> metadata );

    /**
     * ## ACTION `setmetakey`
     *
     * > Set specific metadata key-value pairs
     *
     * - **authority**: `oracle` OR `admin.yield`
     *
     * ### params
     *
     * - `{name} oracle` - oracle main contract
     * - `{name} key` - metakey (ex: name/website/description)
     * - `{string} [value=null]` - (optional) metakey value (if empty, will erase metakey)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action eosio.yield setmetakey '[myoracle, website, "https://myoracle.com"]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void setmetakey( const name oracle, const name key, const optional<string> value );

    /**
     * ## ACTION `approve`
     *
     * > Approve the {{oracle}} oracle for Yield+ rewards
     *
     * - **authority**: `admin.yield`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account to approve
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield approve '[myoracle]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void approve( const name oracle );

    /**
     * ## ACTION `deny`
     *
     * > Deny the {{oracle}} oracle for Yield+ rewards
     *
     * - **authority**: `admin.yield`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account to deny
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield deny '[myoracle]' -p admin.yield
     * ```
     */
    [[eosio::action]]
    void deny( const name oracle );

    /**
     * ## ACTION `update`
     *
     * > Update TVL for a specific protocol
     *
     * - **authority**: `oracle`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account (must be approved)
     * - `{name} protocol` - protocol account to update
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield update '[myoracle, myprotocol]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void update( const name oracle, const name protocol );

    /**
     * ## ACTION `updateall`
     *
     * > Update the TVL for all protocols
     *
     * - **authority**: `oracle`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account (must be approved)
     * - `{uint16_t} [max_rows=20]` - (optional) maximum rows to process
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield updateall '[myoracle, 20]' -p myoracle
     * ```
     */
    [[eosio::action]]
    void updateall( const name oracle, const optional<uint16_t> max_rows );

    /**
     * ## ACTION `updatelog`
     *
     * > Generates a log when an oracle updates its smart contracts
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle initiated update
     * - `{name} protocol` - protocol updated
     * - `{name} category` - protocol category
     * - `{set<name>} contracts` - EOS contracts
     * - `{set<string>} evm` - EVM contracts
     * - `{time_point_sec} period` - time period
     * - `{vector<asset>} balances` - balances in all contracts
     * - `{vector<asset>} prices` - prices of assets
     * - `{asset} tvl` - overall TVL
     * - `{asset} usd` - overall TVL in USD
     *
     * ### Example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "protocol": "myprotocol",
     *     "category": "dexes",
     *     "contracts": ["myprotocol"],
     *     "evm": [],
     *     "period": "2022-06-16T01:40:00",
     *     "balances": ["200000.0000 EOS"],
     *     "prices": ["1.5000 USD"],
     *     "tvl": "200000.0000 EOS",
     *     "usd": "300000.0000 USD"
     * }
     * ```
     */
    [[eosio::action]]
    void updatelog( const name oracle, const name protocol, const name category, const set<name> contracts, const set<string> evm, const time_point_sec period, const vector<asset> balances, const vector<asset> prices, const asset tvl, const asset usd );

    /**
     * ## ACTION `claim`
     *
     * > Claims Yield+ rewards for an oracle
     *
     * - **authority**: `oracle`
     *
     * ### params
     *
     * - `{name} oracle` - oracle
     * - `{name} [receiver=""]` - (optional) receiver of rewards (default=oracle)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action oracle.yield claim '[myoracle, null]' -p myoracle
     * //=> rewards sent to myoracle
     *
     * $ cleos push action oracle.yield claim '[myoracle, myreceiver]' -p myoracle
     * //=> rewards sent to myreceiver
     * ```
     */
    [[eosio::action]]
    void claim( const name oracle, const optional<name> receiver );

    /**
     * ## ACTION `claimlog`
     *
     * > Generates a log when Yield+ rewards are claimed.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle
     * - `{name} [category=oracle]` - oracle category type
     * - `{name} receiver` - receiver of rewards
     * - `{asset} claimed` - claimed rewards
     * - `{asset} balance` - balance available to be claimed
     *
     * ### Example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "category": "oracle",
     *     "receiver": "myreceiver",
     *     "claimed": "1.5500 EOS",
     *     "balance": "0.0000 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void claimlog( const name oracle, const name category, const name receiver, const asset claimed, const asset balance );

    /**
     * ## ACTION `statuslog`
     *
     * > Generates a log when oracle status is modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     * - `{name} status="pending"` - status (`pending/active/denied`)
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "status": "active",
     * }
     * ```
     */
    [[eosio::action]]
    void statuslog( const name oracle, const name status );

    /**
     * ## ACTION `createlog`
     *
     * > Generates a log when an oracle is created in the Yield+ oracle contract.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     * - `{name} [category=oracle]` - oracle category type
     * - `{map<string, string>} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "category": "oracle",
     *     "metadata": [{"key": "name", "value": "My oracle"}, {"key": "website", "value": "https://myoracle.com"}]
     * }
     * ```
     */
    [[eosio::action]]
    void createlog( const name oracle, const name category, const map<name, string> metadata );

    /**
     * ## ACTION `eraselog`
     *
     * > Generates a log when an oracle is erased from the Yield+ oracle contract.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle"
     * }
     * ```
     */
    [[eosio::action]]
    void eraselog( const name oracle );

    /**
     * ## ACTION `metadatalog`
     *
     * > Generates a log when oracle metadata is modified.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle account
     * - `{map<string, string>} metadata` - metadata
     *
     * ### example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "metadata": [{"key": "name", "value": "My oracle"}, {"key": "website", "value": "https://myoracle.com"}]
     * }
     * ```
     */
    [[eosio::action]]
    void metadatalog( const name oracle, const map<name, string> metadata );

    /**
     * ## ACTION `rewardslog`
     *
     * > Generates a log when rewards are generated from update.
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} oracle` - oracle
     * - `{asset} rewards` - Oracle push reward
     * - `{asset} balance` - current claimable balance
     *
     * ### Example
     *
     * ```json
     * {
     *     "oracle": "myoracle",
     *     "rewards": "2.5500 EOS",
     *     "balance": "10.5500 EOS"
     * }
     * ```
     */
    [[eosio::action]]
    void rewardslog( const name oracle, const asset rewards, const asset balance );

    // @debug
    [[eosio::action]]
    void addbalance( const name oracle, const asset quantity );

    // @debug
    [[eosio::action]]
    void cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows );

    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const std::string memo );

    // action wrappers
    using update_action = eosio::action_wrapper<"update"_n, &oracle::update>;
    using updateall_action = eosio::action_wrapper<"updateall"_n, &oracle::updateall>;
    using regoracle_action = eosio::action_wrapper<"regoracle"_n, &oracle::regoracle>;
    using unregister_action = eosio::action_wrapper<"unregister"_n, &oracle::unregister>;
    using approve_action = eosio::action_wrapper<"approve"_n, &oracle::approve>;
    using deny_action = eosio::action_wrapper<"deny"_n, &oracle::deny>;
    using addtoken_action = eosio::action_wrapper<"addtoken"_n, &oracle::addtoken>;
    using deltoken_action = eosio::action_wrapper<"deltoken"_n, &oracle::deltoken>;
    using setreward_action = eosio::action_wrapper<"setreward"_n, &oracle::setreward>;
    using claim_action = eosio::action_wrapper<"claim"_n, &oracle::claim>;

    using updatelog_action = eosio::action_wrapper<"updatelog"_n, &oracle::updatelog>;
    using claimlog_action = eosio::action_wrapper<"claimlog"_n, &oracle::claimlog>;
    using rewardslog_action = eosio::action_wrapper<"rewardslog"_n, &oracle::rewardslog>;
    using statuslog_action = eosio::action_wrapper<"statuslog"_n, &oracle::statuslog>;
    using createlog_action = eosio::action_wrapper<"createlog"_n, &oracle::createlog>;
    using eraselog_action = eosio::action_wrapper<"eraselog"_n, &oracle::eraselog>;
    using metadatalog_action = eosio::action_wrapper<"metadatalog"_n, &oracle::metadatalog>;

private:
    // utils
    time_point_sec get_current_period( const uint32_t period_interval );
    time_point_sec get_last_period( const uint32_t last );
    oracle::config_row get_config();
    void set_status( const name oracle, const name status );
    void check_oracle_active( const name oracle );
    void generate_report( const name protocol, const time_point_sec period );
    void allocate_oracle_rewards( const name oracle );
    void transfer( const name from, const name to, const extended_asset value, const string& memo );
    void prune_protocol_periods( const name protocol );
    void notify_admin();
    void require_auth_admin();
    void require_auth_admin( const name account );
    bool is_contract( const name contract );

    // getters
    asset get_balance_quantity( const name token_contract_account, const name owner, const symbol sym );
    asset get_eos_staked( const name owner );
    periods_row get_median( const name protocol, const uint64_t period_start, const uint64_t period_end );

    // calculate prices
    int64_t calculate_usd_value( const asset quantity );
    int64_t convert_usd_to_eos( const int64_t usd );
    int64_t get_oracle_price( const symbol_code symcode );
    int64_t normalize_price( const int64_t price, const uint8_t precision );
    int64_t get_delphi_price( const name delphi_oracle_id );
    int64_t get_defibox_price( const uint64_t defibox_oracle_id );

    // debug
    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};