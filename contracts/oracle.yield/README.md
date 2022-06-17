# Yield+ Oracle

# Overview

Yield+ Oracle system will be supporting Yield+ Rewards to provide reliable TVL data metrics of DeFi protocols and leveraging decentralized data price feeds from existing oracle providers on EOS mainnet.

## Actions

- Add supported tokens
- Compute TVL protocol
- Report TVL to Rewards

## Quickstart

### `ORACLE` (TVL Oracle)

```bash
# register oracle
$ cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value": "https://myoracle.com"}]]' -p myoracle

# update all (once approved)
$ cleos push action oracle.yield updateall '[myoracle, 20]' -p myoracle

# claim oracle rewards
$ cleos push action oracle.yield claim '[myoracle, null]' -p myoracle
```

### `ADMIN` (Operators)

```bash
# approve oracle
$ cleos push action oracle.yield approve '[oracle]' -p admin.yield

# deny oracle
$ cleos push action oracle.yield deny '[oracle]' -p admin.yield

# add token
$ cleos push action oracle.yield addtoken '["4,EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield

# delete token
cleos push action oracle.yield deltoken '["EOS"]' -p oracle.yield
```

## Table of Content

- [TABLE `config`](#table-config)
- [TABLE `tokens`](#table-tokens)
- [TABLE `periods`](#table-periods)
- [TABLE `oracles`](#table-oracles)
- [ACTION `init`](#action-init)
- [ACTION `addtoken`](#action-addtoken)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `setreward`](#action-setreward)
- [ACTION `regoracle`](#action-regoracle)
- [ACTION `unregister`](#action-unregister)
- [ACTION `approve`](#action-approve)
- [ACTION `deny`](#action-deny)
- [ACTION `update`](#action-update)
- [ACTION `updateall`](#action-updateall)
- [ACTION `updatelog`](#action-updatelog)
- [ACTION `claim`](#action-claim)
- [ACTION `claimlog`](#action-claimlog)

## TABLE `config`

- `{extended_asset} reward_per_update` - reward per update (ex: "0.0200 EOS")
- `{name} yield_contract` - Yield+ core contract
- `{name} admin_contract` - Yield+ admin contract

### example

```json
{
    "reward_per_update": {"contract": "eosio.token", "quantity": "0.0200 EOS"},
    "yield_contract": "eosio.yield",
    "admin_contract": "admin.yield"
}
```

## TABLE `tokens`

### params

- `{symbol} sym` - (primary key) token symbol
- `{name} contract` - token contract
- `{uint64_t} [defibox_oracle_id=null]` - (optional) Defibox oracle ID
- `{name} [delphi_oracle_id=null]` - (optional) Delphi oracle ID
- `{uint64_t} [extra_oracle_id=null]` - (optional) extra oracle ID

### example

```json
{
    "sym": "4,EOS",
    "contract": "eosio.token",
    "defibox_oracle_id": 1,
    "delphi_oracle_id": "eosusd",
    "extra_oracle_id": null
}
```

## TABLE `periods`

- scope: `{name} protocol`

### params

- `{time_point_sec} period` - (primary key) period at time
- `{name} protocol` - protocol contract
- `{name} category` - protocol category
- `{set<name>} contracts.eos` - additional supporting EOS contracts
- `{set<string>} contracts.evm` - additional supporting EVM contracts
- `{vector<asset>} balances` - asset balances
- `{vector<asset>} prices` - currency prices
- `{asset} tvl` - reported TVL averaged value in EOS
- `{asset} usd` - reported TVL averaged value in USD

### example

```json
{
    "period": "2022-05-13T00:00:00",
    "protocol": "myprotocol",
    "contracts": ["myprotocol", "mytreasury"],
    "evm": ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"],
    "balances": ["1000.0000 EOS", "1500.0000 USDT"],
    "prices": ["1.5000 USD", "1.0000 USD"],
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD"
}
```

## TABLE `oracles`

### params

- `{name} oracle` - oracle account
- `{name} status="pending"` - status (`pending/active/denied`)
- `{extended_asset} balance` - balance available to be claimed
- `{map<string, string} metadata` - metadata
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} claimed_at` - claimed at time

### example

```json
{
    "oracle": "myoracle",
    "status": "active",
    "balance": {"quantity": "2.5000 EOS", "contract": "eosio.token"},
    "metadata": [{"key": "url", "value": "https://myoracle.com"}],
    "created_at": "2022-05-13T00:00:00",
    "updated_at": "2022-05-13T00:00:00",
    "claimed_at": "1970-01-01T00:00:00"
}
```

## ACTION `init`

> Initialize Yield+ oracle contract

- **authority**: `get_self()`

### params

- `{extended_symbol} rewards` - Yield+ oracle rewards token
- `{name} yield_contract` - Yield+ core contract
- `{name} admin_contract` - Yield+ admin contract

### Example

```bash
$ cleos push action oracle.yield init '[["4,EOS", "eosio.token"], rewards.yield, admin.yield]' -p oracle.yield
```


## ACTION `addtoken`

- **authority**: `get_self()`

Add token as supported asset

### params

- `{symbol_code} symcode` - token symbol code
- `{name} contract` - token contract
- `{uint64_t} [defibox_oracle_id=""]` - (optional) Defibox oracle ID
- `{name} [delphi_oracle_id=""]` - (optional) Delphi oracle ID

### example

```bash
$ cleos push action oracle.yield addtoken '["EOS", "eosio.token", 1, "eosusd"]' -p oracle.yield
```

## ACTION `deltoken`

- **authority**: `get_self()`

Delete token as supported asset

### params

- `{symbol_code} symcode` - token symbol code

### example

```bash
$ cleos push action oracle.yield deltoken '["EOS"]' -p oracle.yield
```

## ACTION `setreward`

> Set oracle rewards

- **authority**: `get_self()`

### params

- `{asset} reward_per_update` - reward per update

### Example

```bash
$ cleos push action oracle.yield setreward '["0.0200 EOS"]' -p oracle.yield
```

## ACTION `regoracle`

> Register oracle

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account
- `{map<string, string} metadata` - metadata

### Example

```bash
$ cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value": "https://myoracle.com"}]]' -p myoracle
```

## ACTION `unregister`

> Un-register oracle

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account

### Example

```bash
$ cleos push action oracle.yield unregister '[myoracle]' -p myoracle
```

## ACTION `approve`

> Approve oracle

- **authority**: `admin.yield`

### params

- `{name} oracle` - oracle account to approve

### Example

```bash
$ cleos push action oracle.yield approve '[myoracle]' -p admin.yield
```

## ACTION `deny`

> Deny oracle

- **authority**: `admin.yield`

### params

- `{name} oracle` - oracle account to deny

### Example

```bash
$ cleos push action oracle.yield deny '[myoracle]' -p admin.yield
```

## ACTION `update`

> Update TVL for single protocol

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account (must be approved)
- `{name} protocol` - protocol account to update

### Example

```bash
$ cleos push action oracle.yield update '[myoracle, myprotocol]' -p myoracle
```

## ACTION `updateall`

> Update TVL for all protocol(s)

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account (must be approved)
- `{uint16_t} [max_rows=20]` - (optional) maximum rows to process

### Example

```bash
$ cleos push action oracle.yield updateall '[myoracle, 20]' -p myoracle
```

## ACTION `updatelog`

> Update logging

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle initiated update
- `{name} protocol` - protocol updated
- `{name} category` - protocol category
- `{set<name>} contracts` - EOS contracts
- `{set<string>} evm` - EVM contracts
- `{time_point_sec} period` - time period
- `{vector<asset>} balances` - balances in all contracts
- `{vector<asset>} prices` - prices of assets
- `{asset} tvl` - overall TVL
- `{asset} usd` - overall TVL in USD

### Example

```json
{
    "oracle": "myoracle",
    "protocol": "myprotocol",
    "category": "dexes",
    "contracts": ["myprotocol"],
    "evm": [],
    "period": "2022-06-16T01:40:00",
    "balances": ["200000.0000 EOS"],
    "prices": ["1.5000 USD"],
    "tvl": "200000.0000 EOS",
    "usd": "300000.0000 USD"
}
```

## ACTION `claim`

> Claim Oracle rewards

- **authority**: `oracle`

### params

- `{name} oracle` - oracle claiming rewards

### Example

```bash
$ cleos push action oracle.yield claim '[myoracle]' -p myoracle
```

## ACTION `claimlog`

> Claim logging

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle account which received rewards
- `{extended_asset} claimed` - claimed funds

### Example

```json
{
    "protocol": "myprotocol",
    "claimed": {"contract": "eosio.token", "quantity": "0.5500 EOS"}
}
```