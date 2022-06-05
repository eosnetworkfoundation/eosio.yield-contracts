# #!/bin/bash

# contracts
CONTRACT="d.e.yield"

# config
cleos push action $CONTRACT setrate '[500, "200000.0000 EOS", "6000000.0000 EOS"]' -p $CONTRACT
cleos push action $CONTRACT setmetakeys '[["name", "url", "defillama", "dappradar", "recover"]]' -p $CONTRACT

# register protocol
cleos push action $CONTRACT regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
cleos push action $CONTRACT regprotocol '[protocol1, [{"key": "url", "value":"https://protocol1.com"}]]' -p protocol1
cleos push action $CONTRACT regprotocol '[protocol2, [{"key": "url", "value":"https://protocol2.com"}]]' -p protocol2

# set contracts
cleos push action $CONTRACT setcontracts '[myprotocol, ["myvault"], []]' -p myprotocol -p myvault

# approve protocol
cleos push action $CONTRACT approve '[myprotocol]' -p "$CONTRACT"

# claim
cleos push action $CONTRACT claim '[myprotocol, null]' -p myprotocol
