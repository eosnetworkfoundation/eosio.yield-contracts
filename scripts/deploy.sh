#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# contracts
CONTRACT="d.e.yield"

# create accounts
cleos create account eosio $CONTRACT EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myprotocol EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myvault EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio protocol1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio protocol2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract $CONTRACT . eosio.yield.wasm eosio.yield.abi

# permissions
cleos set account permission $CONTRACT active --add-code

# setup ABI
cleos set contract eosio ./include/eosio.system eosio.system.wasm eosio.system.abi
cleos push action eosio abihash '[myprotocol, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
cleos push action eosio abihash '[protocol1, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
cleos push action eosio abihash '[protocol2, fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3]' -p eosio
