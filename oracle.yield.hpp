#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <math.h>

using namespace eosio;

namespace eosio {

class [[eosio::contract("oracle.yield")]] oracle : public eosio::contract {
public:
    using contract::contract;

    // delphi oracle constants
    const name ORACLE_CONTRACT = "delphioracle"_n;
    const name EOS_USD = "eosusd"_n;

    // CONSTANTS
    const uint32_t TEN_MINUTES = minutes(10).to_seconds();
    const uint32_t ONE_DAY = days(1).to_seconds();
    const uint32_t PERIOD_INTERVAL = TEN_MINUTES;

    struct Balances {
        vector<asset>   tokens;
        asset           usd;
        asset           eos;
    };

    /**
     * ## TABLE `tokens`
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) symbol
     * - `{name} contract` - token contract
     * - `{vector<uint64_t>} defibox_oracle_id` - Defibox oracle ID
     * - `{vector<name>} delphi_oracle_id` - Delphi oracle ID
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,EOS",
     *     "contract": "eosio.token",
     *     "defibox_oracle_id": 1,
     *     "delphi_oracle_id": "eosusd"
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol              sym;
        name                contract;
        uint64_t            defibox_oracle_id;
        name                delphi_oracle_id;

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
     * - `{time_point_sec} updated_at` - updated at time
     * - `{asset} usd` - USD TVL averaged value
     * - `{asset} eos` - EOS TVL averaged value
     * - `{map<time_point_sec, Balances>} balances` - total assets balances in custody of protocol contracts
     *
     * ### example
     *
     * ```json
     * {
     *     "protocol": "mydapp",
     *     "contracts": ["mydapp"],
     *     "updated_at": "2022-05-13T00:00:00",
     *     "usd": "2000 USD",
     *     "eos": "2000 EOS",
     *     "balances": [{
     *         "key": "2022-05-13T00:00:00", {
     *             "tokens": ["1000.0000 EOS", "1500.0000 USDT"],
     *             "usd": "3000.0000 USD",
     *             "eos": "2000.0000 EOS"
     *          }
     *      }]
     * }
     * ```
     */
    struct [[eosio::table("tvl")]] tvl_row {
        name                            protocol;
        set<name>                       contracts;
        time_point_sec                  updated_at;
        asset                           usd;
        asset                           eos;
        map<time_point_sec, Balances>   balances;

        uint64_t primary_key() const { return protocol.value; }
    };
    typedef eosio::multi_index< "tvl"_n, snapshot> tvl;

    [[eosio::action]]
    void updatetvl( const name protocol );

    // @protocol
    [[eosio::action]]
    void setcontracts( const name protocol, const set<name> contracts );

    [[eosio::action]]
    void report( const name protocol, const asset tvl );

private:

      //INTERNAL FUNCTIONS DEFINITION

      asset get_contract_balance(name account, std::pair<name, symbol> token);

      asset get_oracle_rate();

      asset get_rex_in_eos( const asset& rex_quantity );

      void update_global_snapshots(snapshot report);

};