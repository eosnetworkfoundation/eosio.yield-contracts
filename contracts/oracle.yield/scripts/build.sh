#!/bin/bash

# contracts
CONTRACT="d.o.yield"

# compile
blanc++ oracle.yield.cpp -I ../ -I ../../external

# # external contracts
# if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
#     eosio-cpp ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm
# fi

# if [ ! -f "./include/oracle.defi/oracle.defi.wasm" ]; then
#     eosio-cpp ./include/oracle.defi/oracle.defi.cpp -I include -o include/oracle.defi/oracle.defi.wasm
# fi

# if [ ! -f "./include/delphioracle/delphioracle.wasm" ]; then
#     eosio-cpp ./include/delphioracle/delphioracle.cpp -I include -o include/delphioracle/delphioracle.wasm
# fi

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract $CONTRACT . oracle.yield.wasm oracle.yield.abi
