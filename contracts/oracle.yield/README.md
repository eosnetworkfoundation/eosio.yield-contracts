# Yield+ Oracle

# Overview

Yield+ Oracle system will be supporting Yield+ Rewards to provide reliable TVL data metrics of DeFi protocols and leveraging decentralized data price feeds from existing oracle providers on EOS mainnet.

# Audits

- <a href="https://s3.eu-central-1.wasabisys.com/audit-certificates/Smart%20Contract%20Audit%20Certificate%20-%20%20EOS%20Yield+.pdf"><img height=30px src="https://user-images.githubusercontent.com/550895/132641907-6425e632-1b1b-4015-9b84-b7f26a25ec58.png" /> Sentnl Audit</a> (2022-08)

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

- [TABLE `evm.tokens`](#table-evm.tokens)
- [TABLE `config`](#table-config)
- [TABLE `tokens`](#table-tokens)
- [TABLE `periods`](#table-periods)
- [TABLE `oracles`](#table-oracles)
- [ACTION `addevmtoken`](#action-addevmtoken)
- [ACTION `init`](#action-init)
- [ACTION `addtoken`](#action-addtoken)
- [ACTION `deltoken`](#action-deltoken)
- [ACTION `setreward`](#action-setreward)
- [ACTION `regoracle`](#action-regoracle)
- [ACTION `unregister`](#action-unregister)
- [ACTION `setmetadata`](#action-setmetadata)
- [ACTION `setmetakey`](#action-setmetakey)
- [ACTION `approve`](#action-approve)
- [ACTION `deny`](#action-deny)
- [ACTION `update`](#action-update)
- [ACTION `updateall`](#action-updateall)
- [ACTION `updatelog`](#action-updatelog)
- [ACTION `claim`](#action-claim)
- [ACTION `claimlog`](#action-claimlog)
- [ACTION `rewardslog`](#action-rewardslog)

## TABLE `evm.tokens`

### params

- `{uint64_t} token_id` - (primary key) EOS EVM token account ID
- `{bytes} address` - EOS EVM token address
- `{uint8_t} decimals` - EOS EVM token decimals
- `{symbol} sym` - token symbol

### example

```json
[
    {
        "token_id": 2,
        "address": "c00592aA41D32D137dC480d9f6d0Df19b860104F",
        "decimals": "18",
        "sym": "4,EOS"
    }
    {
        "token_id": 201,
        "sym": "4,USDT",
        "address": "fa9343c3897324496a05fc75abed6bac29f8a40f",
        "decimals": "6"
    },
]
```

## TABLE `evm.balances`

**Scope**: `<uint64_t> token_id`

### params

- `{uint64_t} address_id` - (primary key) EOS EVM address account ID
- `{bytes} address` - EOS EVM address
- `{asset} balance` - current token balance

### example

```json
{
    "address_id": 663,
    "address": "671A5e209A5496256ee21386EC3EaB9054d658A2",
    "balance": "173151.7110 USDT"
}
```

## TABLE `config`

### params

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

> Initializes the Yield+ oracle contract

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

> Add {{symcode}} token as supported asset.

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

> Delete {{symcode}} token as supported asset

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

> Registers the {{oracle}} oracle with the Yield+ oracle contract

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account
- `{map<string, string} metadata` - metadata

### Example

```bash
$ cleos push action oracle.yield regoracle '[myoracle, [{"key": "url", "value": "https://myoracle.com"}]]' -p myoracle
```

## ACTION `unregister`

> Unregisters the {{oracle}} oracle from the Yield+ oracle contract

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account

### Example

```bash
$ cleos push action oracle.yield unregister '[myoracle]' -p myoracle
```

## ACTION `setmetadata`

> Set metadata for the {{oracle}} oracle

- **authority**: `oracle` OR `admin.yield`

### params

- `{name} oracle` - oracle main contract
- `{map<name, string>} metadata` - (optional) key/value

### Example

```bash
$ cleos push action eosio.oracle setmetadata '[myoracle, [{"key": "website", "value":"https://myoracle.com"}]]' -p myoracle
```

## ACTION `setmetakey`

> Set specific metadata key-value pairs

- **authority**: `oracle` OR `admin.yield`

### params

- `{name} oracle` - oracle main contract
- `{name} key` - metakey (ex: name/website/description)
- `{string} [value=null]` - (optional) metakey value (if empty, will erase metakey)

### Example

```bash
$ cleos push action eosio.oracle setmetakey '[myoracle, website, "https://myoracle.com"]' -p myoracle
```

## ACTION `approve`

> Approve the {{oracle}} oracle for Yield+ rewards

- **authority**: `admin.yield`

### params

- `{name} oracle` - oracle account to approve

### Example

```bash
$ cleos push action oracle.yield approve '[myoracle]' -p admin.yield
```

## ACTION `deny`

> Deny the {{oracle}} oracle for Yield+ rewards

- **authority**: `admin.yield`

### params

- `{name} oracle` - oracle account to deny

### Example

```bash
$ cleos push action oracle.yield deny '[myoracle]' -p admin.yield
```

## ACTION `update`

> Update TVL for a specific protocol

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account (must be approved)
- `{name} protocol` - protocol account to update

### Example

```bash
$ cleos push action oracle.yield update '[myoracle, myprotocol]' -p myoracle
```

## ACTION `updateall`

> Update the TVL for all protocols

- **authority**: `oracle`

### params

- `{name} oracle` - oracle account (must be approved)
- `{uint16_t} [max_rows=20]` - (optional) maximum rows to process

### Example

```bash
$ cleos push action oracle.yield updateall '[myoracle, 20]' -p myoracle
```

## ACTION `updatelog`

> Generates a log when an oracle updates its smart contracts

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

> Claims Yield+ rewards for an oracle

- **authority**: `oracle`

### params

- `{name} oracle` - oracle
- `{name} [receiver=""]` - (optional) receiver of rewards (default=oracle)

### Example

```bash
$ cleos push action oracle.yield claim '[myoracle, null]' -p myoracle
//=> rewards sent to myoracle

$ cleos push action oracle.yield claim '[myoracle, myreceiver]' -p myoracle
//=> rewards sent to myreceiver
```

## ACTION `claimlog`

> Generates a log when Yield+ rewards are claimed.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle
- `{name} [category=oracle]` - oracle category type
- `{name} receiver` - receiver of rewards
- `{asset} claimed` - claimed rewards

### Example

```json
{
    "oracle": "myoracle",
    "category": "oracle",
    "receiver": "myreceiver",
    "claimed": "1.5500 EOS"
}
```

## ACTION `statuslog`

> Generates a log when oracle status is modified.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle account
- `{name} status="pending"` - status (`pending/active/denied`)

### example

```json
{
    "oracle": "myoracle",
    "status": "active",
}
```

## ACTION `createlog`

> Generates a log when an oracle is created in the Yield+ oracle contract.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle account
- `{name} status` - status (`pending/active/denied`)
- `{name} [category=oracle]` - oracle category type
- `{map<string, string>} metadata` - metadata

### example

```json
{
    "oracle": "myoracle",
    "status": "pending",
    "category": "oracle",
    "metadata": [{"key": "name", "value": "My oracle"}, {"key": "website", "value": "https://myoracle.com"}]
}
```

## ACTION `eraselog`

> Generates a log when an oracle is erased from the Yield+ oracle contract.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle account

### example

```json
{
    "oracle": "myoracle"
}
```

## ACTION `metadatalog`

> Generates a log when oracle metadata is modified.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle account
- `{name} status` - status (`pending/active/denied`)
- `{name} [category=oracle]` - oracle category type
- `{map<name, string>} metadata` - metadata

### example

```json
{
    "oracle": "myoracle",
    "status": "active",
    "category": "oracle",
    "metadata": [{"key": "name", "value": "My oracle"}, {"key": "website", "value": "https://myoracle.com"}]
}
```

## ACTION `rewardslog`

> Generates a log when rewards are generated from update.

- **authority**: `get_self()`

### params

- `{name} oracle` - oracle
- `{asset} rewards` - Oracle push reward
- `{asset} balance` - current claimable balance

### Example

```json
{
    "oracle": "myoracle",
    "rewards": "2.5500 EOS",
    "balance": "10.5500 EOS"
}
```