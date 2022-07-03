#!/bin/bash

echo "compiling... [admin.yield]"
cd contracts/admin.yield
blanc++ admin.yield.cpp -I ../ -I ../../external

echo "compiling... [oracle.yield]"
cd ../oracle.yield
blanc++ oracle.yield.cpp -I ../ -I ../../external

echo "compiling... [eosio.yield]"
cd ../eosio.yield
blanc++ eosio.yield.cpp -I ../ -I ../../external