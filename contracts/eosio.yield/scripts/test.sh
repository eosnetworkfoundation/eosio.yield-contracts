# #!/bin/bash

# config
cleos push action eosio.yield init '[["4,EOS", "eosio.token"], oracle.yield, admin.yield]' -p eosio.yield
cleos push action eosio.yield setrate '[500, "200000.0000 EOS", "6000000.0000 EOS"]' -p eosio.yield

# register protocol
cleos push action eosio.yield regprotocol '[myprotocol, dexes, [{"key": "name", "value": "myprotocol"}, {"key": "website", "value":"https://myprotocol.com"}]]' -p myprotocol
cleos push action eosio.yield regprotocol '[protocol1, dexes, [{"key": "name", "value": "myprotocol1"}, {"key": "website", "value":"https://protocol1.com"}]]' -p protocol1
cleos push action eosio.yield regprotocol '[protocol2, dexes, [{"key": "name", "value": "myprotocol2"}, {"key": "website", "value":"https://protocol2.com"}]]' -p protocol2

# set metadata
cleos push action eosio.yield setmetadata '[protocol2, [{"key": "name", "value": "myprotocol2"}, {"key": "website", "value":"https://protocol2.com"}]]' -p protocol2
cleos push action eosio.yield setmetakey '[protocol2, "name", "myprotocol2"]' -p protocol2

# set contracts
cleos push action eosio.yield setcontracts '[myprotocol, ["myvault"]]' -p myprotocol -p myvault
cleos push action eosio.yield setevm '[myprotocol, ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol

# approve protocol
cleos push action eosio.yield approve '[myprotocol]' -p admin.yield

# @DEBUG add balance
cleos push action eosio.yield addbalance '[myprotocol, "1.0000 EOS"]' -p admin.yield

# claim
cleos push action eosio.yield claim '[myprotocol, null]' -p myprotocol

# unregister
cleos push action eosio.yield unregister '[myprotocol]' -p myprotocol