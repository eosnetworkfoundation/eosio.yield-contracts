#!/bin/bash

# compile
eosio-cpp eosio.yield.cpp -I include -I ../

# external contracts
if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    eosio-cpp ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm
fi

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract eosio.yield . eosio.yield.wasm eosio.yield.abi
