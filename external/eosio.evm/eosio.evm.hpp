#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>

#include <string>

using namespace eosio;

typedef std::vector<uint8_t> bytes;

class [[eosio::contract("eosio.evm")]] evm_contract : public eosio::contract {
public:
    using contract::contract;

    const name code = "eosio.evm"_n;

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

    static uint64_t get_account_id( const string address, const name code )
    {
        evm_contract::account_table _account( code, code.value );

        auto idx = _account.get_index<"by.address"_n>();
        auto it = idx.find(evm_contract::to_checksum(address));
        auto itr = _ratelimit.find( it->id );
        check( it != idx.end(), "evm_contract::get_account_id: [address=" + address + "] account not found" );
        return itr->id;
    }

    [[eosio::action]]
    void exec(const exec_input& input, const std::optional<exec_callback>& callback);
    using exec_action = eosio::action_wrapper<"exec"_n, &evm::exec>;
};