#!/bin/bash

# compile
# blanc++ oracle.yield.cpp -I ../ -I ../../external
cdt-cpp oracle.yield.cpp -I ../ -I ../../external

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract oracle.yield . oracle.yield.wasm oracle.yield.abi
