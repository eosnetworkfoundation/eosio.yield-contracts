# #!/bin/bash

# config
cleos push action oracle.yield init '[["4,EOS", "eosio.token"], eosio.yield, admin.yield]' -p oracle.yield
cleos push action oracle.yield setreward '["0.0200 EOS"]' -p oracle.yield

# add tokens
cleos push action oracle.yield addtoken '["EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield
cleos push action oracle.yield addtoken '["USDT", "tethertether", null, null]' -p oracle.yield

# register oracle
cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value":"https://myoracle"}]]' -p myoracle

# approve
cleos push action oracle.yield approve '[myoracle]' -p admin.yield

# updateall
cleos -v push action oracle.yield updateall '[myoracle, null]' -p myoracle

# add & claim
cleos -v push action oracle.yield addbalance '[myoracle, "0.1000 EOS"]' -p myoracle
cleos -v push action oracle.yield claim '[myoracle, null]' -p myoracle