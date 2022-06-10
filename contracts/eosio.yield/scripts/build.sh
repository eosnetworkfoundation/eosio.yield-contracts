#!/bin/bash

# compile
blanc++ eosio.yield.cpp -I ../ -I ../../external

# # external contracts
# if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
#     eosio-cpp ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm
# fi

# if [ ! -f "./include/eosio.system/eosio.system.wasm" ]; then
#     eosio-cpp ./include/eosio.system/eosio.system.cpp -I include -o include/eosio.system/eosio.system.wasm
# fi

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract eosio.yield . eosio.yield.wasm eosio.yield.abi
