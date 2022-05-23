#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio oracle.yield EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myprotocol EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myoracle EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio tethertether EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio oracle.defi EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract oracle.yield . oracle.yield.wasm oracle.yield.abi
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract tethertether ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract oracle.defi ./include/oracle.defi oracle.defi.wasm oracle.defi.abi

# permissions
cleos set account permission oracle.yield active --add-code

# create tokens
cleos push action eosio.token create '["eosio", "100000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "5000000.0000 EOS", "init"]' -p eosio

cleos push action tethertether create '["tethertether", "100000000.0000 USDT"]' -p tethertether
cleos push action tethertether issue '["tethertether", "5000000.0000 USDT", "init"]' -p tethertether

# transfer tokens
cleos transfer eosio myprotocol "300000.0000 EOS"
cleos transfer tethertether myprotocol "300000.0000 USDT" --contract tethertether

# setup oracle
cleos push action oracle.defi setprice '[1, "eosio.token", "4,EOS", 13869]' -p oracle.defi