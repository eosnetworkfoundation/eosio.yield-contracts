#!/bin/bash

# compile
blanc++ admin.yield.cpp -I ../ -I ../../external

# unlock wallet & deploy
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)
cleos set contract admin.yield . admin.yield.wasm admin.yield.abi
