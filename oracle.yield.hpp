#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <math.h>

using namespace eosio;

namespace eosio {

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

    // STRUCTS
    struct TVL {
        vector<asset>   balances;
        int64_t         usd;
        int64_t         eos;
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
     * - `{set<name>} contracts` - additional supporting contracts
     * - `{time_point_sec} period` - updated at time
     * - `{int64_t} usd` - USD TVL averaged value
     * - `{int64_t} eos` - EOS TVL averaged value
     * - `{map<time_point_sec, Balances>} balances` - total assets balances in custody of protocol contracts
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "mydapp",
     *     "contracts": ["mydapp"],
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
        set<name>                       contracts;
        time_point_sec                  updated_at;
        int64_t                         usd;
        int64_t                         eos;
        map<time_point_sec, TVL>        tvl;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "tvl"_n, snapshot> tvl;

    // @oracle
    [[eosio::action]]
    void update( const name protocol );

    // @oracle
    [[eosio::action]]
    void updateall();

    /**
     * ## ACTION `setcontracts`
     *
     * - **authority**: `get_self()`
     *
     * Set contracts for protocol
     *
     * ### params
     *
     * - `{name} protocol` - (primary key) protocol
     * - `{set<name>} contracts` - token contracts
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield setcontracts '["mydapp", ["mydapp", "a.mydapp", "b.mydapp"]]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> contracts );

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
     * - `{symbol} sym` - (primary key) token symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} defibox_oracle_id` - Defibox oracle ID
     * - `{name} delphi_oracle_id` - Delphi oracle ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action oracle.yield addtoken '["4,EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield
     * ```
     */
    [[eosio::action]]
    void addtoken( const symbol sym, const name contract, const uint64_t defibox_oracle_id, const name delphi_oracle_id );

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

    [[eosio::action]]
    void report( const name protocol, const int64_t eos, const int64_t usd, const time_point_sec period );

private:
    time_point_sec get_current_period();
    asset get_balance_amount( const name& token_contract_account, const name& owner, const symbol& sym );
    int64_t calculate_usd_value( const asset quantity )
    int64_t convert_usd_to_eos( const int64_t usd )

    int64_t normalize_price( const int64_t price, const uint8_t precision );
    int64_t get_delphi_price( const name delphi_oracle_id );
    int64_t get_defibox_price( const uint64_t defibox_oracle_id );

    //   //INTERNAL FUNCTIONS DEFINITION

    //   asset get_contract_balance(name account, std::pair<name, symbol> token);

    //   asset get_oracle_rate();

    //   asset get_rex_in_eos( const asset& rex_quantity );

    //   void update_global_snapshots(snapshot report);

};