# #!/bin/bash

# config
cleos push action eosio.yield setrate '[500, 2000000000, 60000000000]' -p eosio.yield
cleos push action eosio.yield setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p eosio.yield

# register protocol
cleos push action eosio.yield regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
cleos push action eosio.yield setcontracts '[myprotocol, ["myprotocol"], []]' -p myprotocol

# report
cleos push action eosio.yield report '["myprotocol", "2022-05-13T00:00:00", 30000000, 20000000]' -p oracle.yield
