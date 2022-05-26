# #!/bin/bash

# contracts
CONTRACT="d.o.yield"

# config
cleos push action $CONTRACT setreward '[["0.0200 EOS", "eosio.token"]]' -p $CONTRACT
cleos push action $CONTRACT setmetakeys '[["name", "url"]]' -p $CONTRACT

# add tokens
cleos push action $CONTRACT addtoken '["EOS", "eosio.token", 1, null]' -p $CONTRACT
cleos push action $CONTRACT addtoken '["USDT", "tethertether", null, null]' -p $CONTRACT

# register oracle
cleos push action $CONTRACT regoracle '[myoracle, [{"key": "url", "value":"https://myoracle.com"}]]' -p myoracle

# approve
cleos push action $CONTRACT approve '[myoracle]' -p $CONTRACT

# updateall
cleos push action $CONTRACT updateall '[myoracle, null]' -p myoracle
