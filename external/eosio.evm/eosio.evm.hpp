#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>

#include <string>

using namespace eosio;

constexpr name code = "eosio.evm"_n;

typedef std::vector<uint8_t> bytes;

namespace evm_runtime {

class [[eosio::contract("eosio.evm")]] evm_contract : public eosio::contract {
public:
    using contract::contract;

    struct [[eosio::table("account")]] account {
        uint64_t                        id;
        bytes                           eth_address;
        uint64_t                        nonce;
        bytes                           balance;
        std::optional<uint64_t>         code_id;

        uint64_t primary_key() const { return id; }

        checksum256 by_eth_address() const {
            return make_key(eth_address);
        }
    };

    typedef multi_index< "account"_n, account,
        indexed_by<"by.address"_n, const_mem_fun<account, checksum256, &account::by_eth_address>>
    > account_table;

    static checksum256 to_checksum( string address )
    {
        if ( address.length() > 40 ) address = address.substr(2);
        return sha256(address.c_str(), address.length());
    }

    [[eosio::action]]
    void exec(const exec_input& input, const std::optional<exec_callback>& callback);
    using exec_action = eosio::action_wrapper<"exec"_n, &evm::exec>;
};
} // namespace evm_runtime