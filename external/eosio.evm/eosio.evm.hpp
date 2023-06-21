#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>

#include <string>

using namespace eosio;

typedef std::vector<uint8_t> bytes;

checksum256 make_key(const uint8_t* ptr, size_t len) {
    uint8_t buffer[32]={0};
    check(len <= sizeof(buffer), "invalida size");
    memcpy(buffer, ptr, len);
    return checksum256(buffer);
}

checksum256 make_key(bytes data){
    return make_key((const uint8_t*)data.data(), data.size());
}

class [[eosio::contract("eosio.evm")]] evm_contract : public eosio::contract {
public:
    using contract::contract;

    const name code = "eosio.evm"_n;

    struct exec_callback {
        eosio::name contract;
        eosio::name action;
    };

   struct exec_input {
        std::optional<bytes> context;
        std::optional<bytes> from;
        bytes                to;
        bytes                data;
        std::optional<bytes> value;
   };

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

    [[eosio::action]]
    void exec(const exec_input& input, const std::optional<exec_callback>& callback);
    using exec_action = eosio::action_wrapper<"exec"_n, &evm_contract::exec>;
};