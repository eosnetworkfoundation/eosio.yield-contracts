#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

constexpr extended_symbol value_symbol { symbol{"USDT",4}, "tethertether"_n };
constexpr name oracle_code = "oracle.defi"_n;

namespace defi {

class [[eosio::contract("oracle.defi")]] oracle : public eosio::contract {
public:
    using contract::contract;

    struct [[eosio::table]] oracle_row {
        uint64_t                id;
        eosio::name             contract;
        eosio::symbol_code      coin;
        uint8_t                 precision;
        uint64_t                acc_price;
        uint64_t                last_price;
        uint64_t                avg_price;
        eosio::time_point_sec   last_update;

        uint64_t primary_key()const { return id; }
        uint128_t get_by_extsym() const { return static_cast<uint128_t>(contract.value) << 64 | (uint64_t) coin.to_string().length() << 48 | coin.raw(); }
    };
    typedef eosio::multi_index< "prices"_n, oracle_row,
        eosio::indexed_by< "byextsym"_n, eosio::const_mem_fun<oracle_row, uint128_t, &oracle_row::get_by_extsym>>
    > prices;

    /**
     * ## STATIC `get_value`
     *
     * Given an input amount of an asset and Defibox oracle id, return USD value based on Defibox oracle
     *
     * ### params
     *
     * - `{extended_asset} in` - input tokens
     * - `{uint64_t} oracle_id` - oracle id
     *
     * ### example
     *
     * ```c++
     * // Inputs
     * const extended_asset in = asset { 10000, "EOS" };
     * const symbol oracle_id = 1;
     *
     * // Calculation
     * const asset out = defilend::get_value( in, oracle_id );
     * // => 4.0123
     * ```
     */
    static double get_value( const extended_asset in, const uint64_t oracle_id )
    {
        if (in.get_extended_symbol() == value_symbol)
            return static_cast<double>(in.quantity.amount) / pow(10, in.quantity.symbol.precision());

        prices prices_tbl( oracle_code, oracle_code.value);
        const auto row = prices_tbl.get(oracle_id, "defilend: no oracle");
        print("\nUsing ", row.avg_price, " rate");

        return static_cast<double>(in.quantity.amount) / pow(10, in.quantity.symbol.precision()) * (static_cast<double>(row.avg_price) / pow(10, row.precision));
    }

    [[eosio::action]]
    void setprice( uint64_t id, eosio::extended_symbol ext_sym, uint8_t precision, uint64_t avg_price );
    using setprice_action = eosio::action_wrapper<"setprice"_n, &defi::oracle::setprice>;
};
}