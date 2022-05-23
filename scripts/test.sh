# #!/bin/bash

# config
cleos push action eosio.yield setrate '[500, ["300000.0000 USD", "200000.0000 EOS"], ["9000000.0000 USD", "6000000.0000 EOS"]]' -p eosio.yield
cleos push action eosio.yield setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p eosio.yield

# register protocol
cleos push action eosio.yield regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
cleos push action eosio.yield setcontracts '[myprotocol, ["myprotocol"], []]' -p myprotocol

# approve protocol
cleos push action eosio.yield approve '[myprotocol]' -p eosio.yield

# report
cleos push action eosio.yield report '["myprotocol", "2022-05-13T00:00:00", ["300000.0000 USD", "200000.0000 EOS"]]' -p oracle.yield
