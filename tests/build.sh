#!/bin/bash


echo "compiling... [oracle.yield]"
cd contracts/oracle.yield
blanc++ oracle.yield.cpp -I ../ -I ../../external

echo "compiling... [eosio.yield]"
cd ../eosio.yield
blanc++ eosio.yield.cpp -I ../ -I ../../external

echo "compiling... [admin.yield]"
cd ../admin.yield
blanc++ admin.yield.cpp -I ../ -I ../../external