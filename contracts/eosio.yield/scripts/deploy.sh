#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio eosio.yield EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio oracle.yield EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio admin.yield EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myprotocol EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myvault EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio protocol1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio protocol2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract eosio.yield . eosio.yield.wasm eosio.yield.abi
cleos set contract eosio.token ../../external/eosio.token eosio.token.wasm eosio.token.abi

# permissions
cleos set account permission eosio.yield active --add-code

# create tokens
cleos push action eosio.token create '["eosio", "1000000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "1000000000.0000 EOS", "init"]' -p eosio

# setup ABI
cleos set contract eosio ../../external/eosio.system eosio.system.wasm eosio.system.abi
cleos push action eosio abihash '[myprotocol, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
cleos push action eosio abihash '[protocol1, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
cleos push action eosio abihash '[protocol2, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
