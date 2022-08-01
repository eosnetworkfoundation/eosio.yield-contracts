#!/bin/bash


echo "compiling... [oracle.yield]"
cd contracts/oracle.yield
blanc++ oracle.yield.cpp -I ../ -I ../../external
shasum -a 256 oracle.yield.wasm

echo "compiling... [eosio.yield]"
cd ../eosio.yield
blanc++ eosio.yield.cpp -I ../ -I ../../external
shasum -a 256 eosio.yield.wasm

echo "compiling... [admin.yield]"
cd ../admin.yield
blanc++ admin.yield.cpp -I ../ -I ../../external
shasum -a 256 admin.yield.wasm