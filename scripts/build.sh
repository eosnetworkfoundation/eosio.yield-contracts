#!/bin/bash

eosio-cpp oracle.yield.cpp -I include -I ../

if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    eosio-cpp ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm
fi

if [ ! -f "./include/oracle.defi/oracle.defi.wasm" ]; then
    eosio-cpp ./include/oracle.defi/oracle.defi.cpp -I include -o include/oracle.defi/oracle.defi.wasm
fi

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract oracle.yield . oracle.yield.wasm oracle.yield.abi
