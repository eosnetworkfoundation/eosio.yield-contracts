# #!/bin/bash

# config
cleos push action eosio.yield init '[["4,EOS", "eosio.token"], oracle.yield, admin.yield, eosio.evm]' -p eosio.yield
cleos push action eosio.yield setrate '[500, "200000.0000 EOS", "6000000.0000 EOS"]' -p eosio.yield
cleos push action eosio.yield setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p eosio.yield

# register protocol
cleos push action eosio.yield regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
cleos push action eosio.yield regprotocol '[protocol1, [{"key": "url", "value":"https://protocol1.com"}]]' -p protocol1
cleos push action eosio.yield regprotocol '[protocol2, [{"key": "url", "value":"https://protocol2.com"}]]' -p protocol2

# set contracts
cleos push action eosio.yield setcontracts '[myprotocol, ["myvault"], []]' -p myprotocol -p myvault

# approve protocol
cleos push action eosio.yield approve '[myprotocol]' -p "eosio.yield"

# claim
cleos push action eosio.yield claim '[myprotocol, null]' -p myprotocol
