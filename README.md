# EOSIO Yield+ - Rewards

# Overview

Yield+ is a Rewards system that incentivizes DeFi protocols to retain long-term TVL within their application.

## Actions

- Apply to Yield+
- Claim Yield+ rewards
- Update rewards by TVL Oracle

## Quickstart

### `USER` (DeFi application protocol)

```bash
# apply protocol
$ cleos push action eosio.yield regprotocol '[protocol, [{"key": "url", "value": "https://mywebsite.com"}]]' -p protocol

# set additional protocol contracts
# > action can only be called during [status="pending"]
$ cleos push action eosio.yield setcontracts '[protocol, [a.protocol, b.protocol]]' -p protocol -p a.protocol -p b.protocol

# claim rewards
# > claimable every 10 minutes interval
$ cleos push action eosio.yield claim '[protocol, receiver]' -p protocol
```

### `ADMIN` (Operators)

```bash
# approve protocol
$ cleos push action eosio.yield approve '[protocol]' -p eosio.yield
```

### `ORACLE` (TVL Oracle)

```bash
# add balance to protocol
$ cleos push action eosio.yield addbalance '[mycontract]' -p oracle.yield
```

## Table of Content

- [TABLE `status`](#table-status)
- [TABLE `config`](#table-config)
- [ACTION `claim`](#action-claim)
