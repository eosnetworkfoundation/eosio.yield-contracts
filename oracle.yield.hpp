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

    // EXTERNAL CONTRACTS
    const name DELPHI_ORACLE_CONTRACT = "delphioracle"_n;
    const name DEFIBOX_ORACLE_CONTRACT = "oracle.defi"_n;

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
     * - `{time_point_sec} period` - updated at time
     * - `{set<name>} contracts.eos` - additional supporting EOS contracts
     * - `{set<string>} contracts.evm` - additional supporting EVM contracts
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     * - `{map<time_point_sec, Balances>} balances` - total assets balances in custody of protocol contracts
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
     *     "period": "2022-05-13T00:00:00",
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
        Contracts                       contracts;
        time_point_sec                  period;
        int64_t                         usd;
        int64_t                         eos;
        map<time_point_sec, TVL>        tvl;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "tvl"_n, tvl_row> tvl_table;

    // @oracle
    [[eosio::action]]
    void update( const name oracle, const name protocol );

    // @oracle
    [[eosio::action]]
    void updateall( const name oracle, const optional<uint16_t> max_rows );

    // /**
    //  * ## ACTION `setcontracts`
    //  *
    //  * - **authority**: `get_self()`
    //  *
    //  * Set contracts for protocol
    //  *
    //  * ### params
    //  *
    //  * - `{name} protocol` - (primary key) protocol
    //  * - `{set<name>} contracts` - token contracts
    //  *
    //  * ### example
    //  *
    //  * ```bash
    //  * $ cleos push action oracle.yield setcontracts '["mydapp", ["mydapp", "a.mydapp", "b.mydapp"]]' -p oracle.yield
    //  * ```
    //  */
    // [[eosio::action]]
    // void setcontracts( const name protocol, const set<name> contracts );

    /**
     * ## ACTION `delprotocol`
     *
     * - **authority**: `get_self()`
     *
     * Delete protocol
     *
     * ### params
     *
     * - `{name} protocol` - protocol
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield delprotocol '["mydapp"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void delprotocol( const name protocol );

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

    // action wrappers
    using report_action = eosio::action_wrapper<"report"_n, &oracle::report>;

private:
    // get time periods
    time_point_sec get_current_period();

    // get balances
    asset get_balance_quantity( const name token_contract_account, const name owner, const symbol sym );
    asset get_eos_staked( const name owner );

    // calculate prices
    int64_t calculate_usd_value( const asset quantity );
    int64_t convert_usd_to_eos( const int64_t usd );
    int64_t get_oracle_price( const symbol_code symcode );
    int64_t normalize_price( const int64_t price, const uint8_t precision );
    int64_t get_delphi_price( const name delphi_oracle_id );
    int64_t get_defibox_price( const uint64_t defibox_oracle_id );
};