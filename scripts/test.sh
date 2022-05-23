# #!/bin/bash

# config
cleos push action oracle.yield setreward '["0.0200 EOS"]' -p oracle.yield
cleos push action oracle.yield setmetakeys '[["name", "url"]]' -p oracle.yield

# add tokens
cleos push action oracle.yield addtoken '["EOS", "eosio.token", 1, null]' -p oracle.yield
cleos push action oracle.yield addtoken '["USDT", "tethertether", null, null]' -p oracle.yield

# register oracle
cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value":"https://myoracle.com"}]]' -p myoracle

# approve
cleos push action oracle.yield approve '[myoracle]' -p oracle.yield

# updateall
cleos push action oracle.yield updateall '[myoracle, null]' -p myoracle
