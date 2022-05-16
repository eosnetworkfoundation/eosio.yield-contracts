# EOSIO Yield+ - Oracle

# Overview

Yield+ Oracle system will be supporting Yield+ Rewards to provide reliable TVL data metrics of DeFi protocols and leveraging decentralized data price feeds from existing oracle providers on EOS mainnet.

## Actions

- Add supported tokens
- Compute TVL protocol
- Report TVL to Rewards

## Quickstart

### `USER` (DeFi application protocol)

```bash
# add token
$ cleos push action oracle.yield addtoken '["4,EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield

# delete token
cleos push action oracle.yield deltoken '["EOS"]' -p oracle.yield
```

### `ORACLE` (TVL Oracle)

```bash
# report TVL
$ cleos push action oracle.yield report '["mydapp", "2022-05-13T00:00:00", 30000000, 20000000]' -p oracle.yield
```

## Table of Content

- [TABLE `tokens`](#table-tokens)
- [TABLE `tvl`](#table-tvl)
- [ACTION `addtoken`](#action-addtoken)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `report`](#action-report)
