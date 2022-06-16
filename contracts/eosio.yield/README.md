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
# register protocol
$ cleos push action eosio.yield regprotocol '[protocol, [{"key": "url", "value": "https://myprotocol.com"}]]' -p protocol

# set additional protocol contracts
# > action can only be called during [status="pending"]
$ cleos push action eosio.yield setcontracts '[protocol, [a.protocol, b.protocol]]' -p protocol -p a.protocol -p b.protocol

# claim rewards
# > after 24 hours of being approved
# > claimable every 10 minutes interval
$ cleos push action eosio.yield claim '[protocol, receiver]' -p protocol
```

### `ADMIN` (Operators)

```bash
# approve protocol
$ cleos push action eosio.yield approve '[protocol]' -p eosio.yield@admin
# notifies => oracle.yield
```

### `ORACLE` (TVL Oracle)

```bash
# report protocol TVL to Yield+
$ cleos push action oracle.yield report '[protocol, "2021-04-12T12:20:00", <TVL USD>, <TVL EOS>]' -p oracle.yield
```

## Table of Content

- [TABLE `config`](#table-config)
- [TABLE `state`](#table-state)
- [TABLE `protocols`](#table-protocols)
- [ACTION `init`](#action-init)
- [ACTION `setrate`](#action-setrate)
- [ACTION `regprotocol`](#action-regprotocol)
- [ACTION `unregister`](#action-unregister)
- [ACTION `setcontracts`](#action-setcontracts)
- [ACTION `setevm`](#action-setevm)
- [ACTION `approve`](#action-approve)
- [ACTION `deny`](#action-deny)
- [ACTION `report`](#action-report)
- [ACTION `rewardslog`](#action-rewardslog)
- [ACTION `claim`](#action-claim)
- [ACTION `claimlog`](#action-claimlog)

## TABLE `config`

- `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
- `{asset} min_tvl_report` - minimum TVL report
- `{asset} max_tvl_report` - maximum TVL report
- `{extended_symbol} rewards` - rewards token
- `{name} oracle_contract` - Yield+ Oracle contract
- `{name} admin_contract` - Yield+ admin contract

### example

```json
{
    "annual_rate": 500,
    "min_tvl_report": "200000.0000 EOS",
    "max_tvl_report": "6000000.0000 EOS",
    "rewards": {"symbol": "4,EOS", "contract": "eosio.token"},
    "oracle_contract": "oracle.yield",
    "admin_contract": "admin.yield"
}
```

## TABLE `state`

- `{set<name>} active_protocols` - array of active protocols

### example

```json
{
    "active_protocols": ["myprotocol"]
}
```

## TABLE `protocols`

### params

- `{name} protocol` - primary protocol contract
- `{name} status="pending"` - status (`pending/active/denied`)
- `{set<name>} contracts.eos` - additional supporting EOS contracts
- `{set<string>} contracts.evm` - additional supporting EVM contracts
- `{asset} tvl` - reported TVL averaged value in EOS
- `{asset} usd` - reported TVL averaged value in USD
- `{extended_asset} balance` - balance available to be claimed
- `{map<string, string} metadata` - metadata
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} claimed_at` - claimed at time
- `{time_point_sec} period_at` - period at time

### example

```json
{
    "protocol": "myprotocol",
    "status": "active",
    "contracts": ["myprotocol", "mytreasury"],
    "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD",
    "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
    "metadata": [{"key": "type", "value": "swap"}, {"key": "url", "value": "https://myprotocol.com"}],
    "created_at": "2022-05-13T00:00:00",
    "updated_at": "2022-05-13T00:00:00",
    "claimed_at": "1970-01-01T00:00:00",
    "period_at": "1970-01-01T00:00:00",
}
```

## ACTION `init`

> Initialize Yield+ rewards contract

- **authority**: `get_self()`

### params

- `{extended_symbol} rewards` - Yield+ rewards token
- `{name} oracle_contract` - Yield+ oracle contract
- `{name} admin_contract` - Yield+ admin contract

### Example

```bash
$ cleos push action eosio.yield init '[["4,EOS", "eosio.token"], oracle.yield, admin.yield]' -p eosio.yield
```

## ACTION `setrate`

> Set rewards rate

- **authority**: `get_self()`

### params

- `{uint16_t} annual_rate` - annual rate (pips 1/100 of 1%)
- `{asset} min_tvl_report` - minimum TVL report
- `{asset} max_tvl_report` - maximum TVL report

### Example

```bash
$ cleos push action eosio.yield setrate '[500, "200000.0000 EOS", "6000000.0000 EOS"]' -p eosio.yield
```

## ACTION `regprotocol`

> Registry protocol

- **authority**: `protocol` OR `admin.yield`

### params

- `{name} protocol` - protocol main contract
- `{map<name, string>} metadata` - (optional) key/value

### Example

```bash
$ cleos push action eosio.yield regprotocol '[myprotocol, [{"key": "url", "value":"https://myprotocol.com"}]]' -p myprotocol
```

## ACTION `unregister`

> Un-registry protocol

- **authority**: `protocol`

### params

- `{name} protocol` - protocol

### Example

```bash
$ cleos push action eosio.yield unregister '[myprotocol]' -p myprotocol
```

## ACTION `setcontracts`

> Set EOS contracts

- **authority**: (`protocol` AND `contracts`) OR `admin.yield`

### params

- `{name} protocol` - protocol (will be included in EOS contracts)
- `{set<name>} contracts` - additional EOS contracts

### Example

```bash
$ cleos push action eosio.yield setcontracts '[myprotocol, [myvault]]' -p myprotocol -p myvault
```

## ACTION `setevm`

> Set EVM contracts

- **authority**: `protocol` AND `evm`

### params

- `{name} protocol` - protocol (will be included in EOS contracts)
- `{set<string>} evm` - additional EVM contracts

### Example

```bash
$ cleos push action eosio.yield setevm '[myprotocol, ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol
```

## ACTION `approve`

> Approve protocol

- **authority**: `admin.yield`

### params

- `{name} protocol` - protocol to approve

### Example

```bash
$ cleos push action eosio.yield approve '[myprotocol]' -p admin.yield
```

## ACTION `deny`

> Deny protocol

- **authority**: `admin.yield`

### params

- `{name} protocol` - protocol to deny

### Example

```bash
$ cleos push action eosio.yield deny '[myprotocol]' -p admin.yield
```

## ACTION `claim`

> Claim TVL rewards

- **authority**: `protocol`

### params

- `{name} protocol` - protocol
- `{name} [receiver=""]` - (optional) receiver of rewards (default=protocol)

### Example

```bash
$ cleos push action eosio.yield claim '[myprotocol, null]' -p myprotocol
//=> rewards sent to myprotocol

$ cleos push action eosio.yield claim '[myprotocol, myreceiver]' -p myprotocol
//=> rewards sent to myreceiver
```

## ACTION `claimlog`

> Claim logging

- **authority**: `get_self()`

### params

- `{name} protocol` - protocol
- `{name} receiver` - receiver of rewards
- `{extended_asset} claimed` - claimed rewards

### Example

```json
{
    "protocol": "myprotocol",
    "receiver": "myreceiver",
    "claimed": {"contract": "eosio.token", "quantity": "1.5500 EOS"}
}
```

## ACTION `report`

- **authority**: `oracle.yield`

Report TVL from oracle

### params

- `{name} protocol` - protocol
- `{time_point_sec} period` - period time
- `{uint32_t} period_interval` - period interval (in seconds)
- `{asset} tvl` - TVL averaged value in EOS
- `{asset} usd` - TVL averaged value in USD

### example

```bash
$ cleos push action eosio.yield report '[myprotocol, "2022-05-13T00:00:00", 600, "200000.0000 EOS", "300000.0000 USD"]' -p oracle.yield
```

## ACTION `rewardslog`

> Rewards logging

- **authority**: `get_self()`

### params

- `{name} protocol` - protocol
- `{time_point_sec} period` - period time
- `{uint32_t} period_interval` - period interval (in seconds)
- `{asset} tvl` - TVL averaged value in EOS
- `{asset} usd` - TVL averaged value in USD
- `{asset} rewards` - TVL rewards
- `{asset} balance` - current claimable balance

### Example

```json
{
    "protocol": "myprotocol",
    "period": "2022-05-13T00:00:00",
    "period_interval": 600,
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD",
    "rewards": "2.5500 EOS",
    "balance": "10.5500 EOS"
}
```