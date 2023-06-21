# Yield+ Rewards

# Overview

Yield+ is a Rewards system that incentivizes DeFi protocols to retain long-term TVL within their application.

# Audits

- <a href="https://s3.eu-central-1.wasabisys.com/audit-certificates/Smart%20Contract%20Audit%20Certificate%20-%20%20EOS%20Yield+.pdf"><img height=30px src="https://user-images.githubusercontent.com/550895/132641907-6425e632-1b1b-4015-9b84-b7f26a25ec58.png" /> Sentnl Audit</a> (2022-08)

## Actions

- Apply to Yield+
- Claim Yield+ rewards
- Update rewards by TVL Oracle

## Quickstart

### `USER` (DeFi application protocol)

```bash
# register protocol
$ cleos push action eosio.yield regprotocol '[protocol, dexes, [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}]]' -p protocol

# set additional protocol contracts
# > action can only be called during [status="pending"]
$ cleos push action eosio.yield setcontracts '[protocol, [a.protocol, b.protocol]]' -p protocol -p a.protocol -p b.protocol

# claim rewards
# > after 24 hours of being approved
# > claimable every 10 minutes interval
$ cleos push action eosio.yield claim '[myprotocol, null]' -p myprotocol
# //=> rewards sent to myprotocol

$ cleos push action eosio.yield claim '[myprotocol, myreceiver]' -p myprotocol
# //=> rewards sent to myreceiver
```

### `ADMIN` (Operators)

```bash
# approve protocol
$ cleos push action eosio.yield approve '[protocol]' -p eosio.yield@admin

# deny protocol
$ cleos push action eosio.yield deny '[protocol]' -p eosio.yield@admin
```

### `ORACLE` (TVL Oracle)

```bash
# report protocol TVL to Yield+
cleos push action eosio.yield report '[protocol, "2022-05-13T00:00:00", 600, "200000.0000 EOS", "300000.0000 USD"]' -p oracle.yield
```

## Table of Content

- [TABLE `config`](#table-config)
- [TABLE `state`](#table-state)
- [TABLE `protocols`](#table-protocols)
- [ACTION `init`](#action-init)
- [ACTION `setrate`](#action-setrate)
- [ACTION `regprotocol`](#action-regprotocol)
- [ACTION `setmetakey`](#action-setmetakey)
- [ACTION `unregister`](#action-unregister)
- [ACTION `setcontracts`](#action-setcontracts)
- [ACTION `setevm`](#action-setevm)
- [ACTION `approve`](#action-approve)
- [ACTION `setcategory`](#action-setcategory)
- [ACTION `deny`](#action-deny)
- [ACTION `report`](#action-report)
- [ACTION `rewardslog`](#action-rewardslog)
- [ACTION `claim`](#action-claim)
- [ACTION `claimlog`](#action-claimlog)
- [ACTION `statuslog`](#action-statuslog)
- [ACTION `contractslog`](#action-contractslog)
- [ACTION `createlog`](#action-createlog)
- [ACTION `eraselog`](#action-eraselog)
- [ACTION `metadatalog`](#action-metadatalog)

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
    "rewards": {"sym": "4,EOS", "contract": "eosio.token"},
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
- `{name} category` - protocol category (ex: `dexes/lending/staking`)
- `{set<name>} contracts` - additional supporting EOS contracts
- `{set<string>} evm` - additional supporting EVM contracts
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
    "category": "dexes",
    "contracts": ["myprotocol", "mytreasury"],
    "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD",
    "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
    "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}],
    "created_at": "2022-05-13T00:00:00",
    "updated_at": "2022-05-13T00:00:00",
    "claimed_at": "1970-01-01T00:00:00",
    "period_at": "1970-01-01T00:00:00"
}
```

## ACTION `init`

> Initialize the rewards contract

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

> Set TVL rewards rate at {{annual_rate}} basis points.

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

> Register the {{protocol}} protocol.

- **authority**: `protocol`

### params

- `{name} protocol` - protocol main contract
- `{name} category` - protocol category (dexes/lending/yield)
- `{map<name, string>} metadata` - (optional) key/value

### Example

```bash
$ cleos push action eosio.yield regprotocol '[myprotocol, dexes, [{"key": "website", "value":"https://myprotocol.com"}]]' -p myprotocol
```

## ACTION `setmetadata`

> Set the metadata for the {{protocol}} protocol.

- **authority**: `protocol` OR `admin.yield`

### params

- `{name} protocol` - protocol main contract
- `{map<name, string>} metadata` - (optional) key/value

### Example

```bash
$ cleos push action eosio.yield setmetadata '[myprotocol, [{"key": "website", "value":"https://myprotocol.com"}]]' -p myprotocol
```

## ACTION `setmetakey`

> Set the {{key}} metadata key to {{value}}.

- **authority**: `protocol` OR `admin.yield`

### params

- `{name} protocol` - protocol main contract
- `{name} key` - metakey (ex: name/website/description)
- `{string} [value=null]` - (optional) metakey value (if empty, will erase metakey)

### Example

```bash
$ cleos push action eosio.yield setmetakey '[myprotocol, website, "https://myprotocol.com"]' -p myprotocol
```

## ACTION `unregister`

> Unregister the {{protocol}} protocol.

- **authority**: `protocol` or `admin.yield`

### params

- `{name} protocol` - protocol

### Example

```bash
$ cleos push action eosio.yield unregister '[myprotocol]' -p myprotocol
```

## ACTION `setcontracts`

> Sets the smart contracts for the {{protocol}} protocol.

- **authority**: (`protocol` AND `contracts`) OR `admin.yield`

### params

- `{name} protocol` - protocol (will be included in EOS contracts)
- `{set<name>} contracts` - EOS smart contracts
- `{set<string>} evm_contracts` - EOS EVM smart contracts

### Example

```bash
$ cleos push action eosio.yield setcontracts '[myprotocol, [myvault], []]' -p myprotocol
$ cleos push action eosio.yield setcontracts '[myprotocol, [], ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]]' -p myprotocol
```

## ACTION `approve`

> Approves the {{protocol}} protocol for the Yield+ rewards program.

- **authority**: `admin.yield`

### params

- `{name} protocol` - protocol to approve

### Example

```bash
$ cleos push action eosio.yield approve '[myprotocol]' -p admin.yield
```

## ACTION `setcategory`

> Sets the category of the {{protocol}} protocol.

- **authority**: `admin.yield`

### params

- `{name} protocol` - protocol to approve
- `{name} category` - protocol category (eligible categories in `admin.yield`)

### Example

```bash
$ cleos push action eosio.yield setcategory '[myprotocol, dexes]' -p admin.yield
```

## ACTION `deny`

> Denies the {{protocol}} protocol for the Yield+ rewards program.

- **authority**: `admin.yield`

### params

- `{name} protocol` - protocol to deny

### Example

```bash
$ cleos push action eosio.yield deny '[myprotocol]' -p admin.yield
```

## ACTION `claim`

> Claims the Yield+ rewards for the {{protocol}} protocol.

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

> Generates a log each time Yield+ rewards are claimed.

- **authority**: `get_self()`

### params

- `{name} protocol` - protocol
- `{name} category` - protocol category
- `{name} receiver` - receiver of rewards
- `{asset} claimed` - claimed rewards

### Example

```json
{
    "protocol": "myprotocol",
    "category": "dexes",
    "receiver": "myreceiver",
    "claimed":"1.5500 EOS"
}
```

## ACTION `report`

> Generates a report of the current TVL from the {{protocol}} protocol.

- **authority**: `oracle.yield`

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

> Generates a log when rewards are generated from reports.

- **authority**: `get_self()`

### params

- `{name} protocol` - protocol
- `{name} category` - protocol category
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
    "category": "dexes",
    "period": "2022-05-13T00:00:00",
    "period_interval": 600,
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD",
    "rewards": "2.5500 EOS",
    "balance": "10.5500 EOS"
}
```

## ACTION `statuslog`

> Generates a log when a protocol's status is modified.

- **authority**: `get_self()`

### params

- `{name} protocol` - primary protocol contract
- `{name} status="pending"` - status (`pending/active/denied`)

### example

```json
{
    "protocol": "myprotocol",
    "status": "active",
}
```

## ACTION `contractslog`

> Generates a log when a protocol's contracts are modified.

- **authority**: `get_self()`

### params

- `{name} protocol` - primary protocol contract
- `{name} status` - status (`pending/active/denied`)
- `{set<name>} contracts.eos` - additional supporting EOS contracts
- `{set<string>} contracts.evm` - additional supporting EVM contracts

### example

```json
{
    "protocol": "myprotocol",
    "status": "pending",
    "contracts": ["myprotocol", "mytreasury"],
    "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
}
```

## ACTION `createlog`

> Generates a log when a protocol is created.

- **authority**: `get_self()`

### params

- `{name} protocol` - primary protocol contract
- `{name} status` - status (`pending/active/denied`)
- `{name} category` - protocol category (dexes/lending/yield)
- `{map<string, string>} metadata` - metadata

### example

```json
{
    "protocol": "myprotocol",
    "status": "pending",
    "category": "dexes",
    "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}]
}
```

## ACTION `eraselog`

> Generates a log when a protocol is erased.

- **authority**: `get_self()`

### params

- `{name} protocol` - primary protocol contract

### example

```json
{
    "protocol": "myprotocol"
}
```

## ACTION `metadatalog`

> When protocol metadata is modified

- **authority**: `get_self()`

### params

- `{name} protocol` - primary protocol contract
- `{name} status` - status (`pending/active/denied`)
- `{name} category` - protocol category (ex: `dexes/lending/staking`)
- `{map<name, string>} metadata` - metadata

### example

```json
{
    "protocol": "myprotocol",
    "status": "pending",
    "category": "dexes",
    "metadata": [{"key": "name", "value": "My Protocol"}, {"key": "website", "value": "https://myprotocol.com"}]
}
```