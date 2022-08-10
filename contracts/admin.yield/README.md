# Yield+ Admin

## Overview

Admin account for Yield+ on-notification events

## Audits

- <a href="https://s3.eu-central-1.wasabisys.com/audit-certificates/Smart%20Contract%20Audit%20Certificate%20-%20%20EOS%20Yield+.pdf"><img height=30px src="https://user-images.githubusercontent.com/550895/132641907-6425e632-1b1b-4015-9b84-b7f26a25ec58.png" /> Sentnl Audit</a> (2022-08)

## Actions

- Checks protocol metadata
- Checks category value

## Quickstart

### `ADMIN` (Operators)

```bash
# set metakey
$ cleos push action admin.yield setmetakey '[website, true, "Protocol website"]' -p admin.yield

# set category
$ cleos push action admin.yield setcategory '[dexes, "Protocols where you can swap/trade cryptocurrency"]' -p admin.yield
```

## Table of Content

- [TABLE `metakeys`](#table-metakeys)
- [TABLE `categories`](#table-categories)
- [ACTION `setmetakey`](#action-setmetakey)
- [ACTION `setcategory`](#action-setcategory)
- [ACTION `delmetakey`](#action-delmetakey)
- [ACTION `delcategory`](#action-delcategory)


## TABLE `metakeys`

- `{name} key` - metadata key
- `{name} type` - value type (ex: string/boolean/ipfs/url)
- `{bool} required` - is required (true/false)
- `{string} description` - metadata description

### example

```json
{
    "key": "name",
    "type": "string",
    "required": true,
    "description": "Name of protocol"
}
```

## TABLE `categories`

- `{name} category` - category [metadata.type] value
- `{string} description` - category description

### example

```json
{
    "category": "dexes",
    "description": "Protocols where you can swap/trade cryptocurrency"
}
```

## ACTION `setmetakeys`

> Set metakey

- **authority**: `get_self()`

### params

- `{name} key` - metadata key
- `{name} type` - value type (ex: string/boolean/ipfs/url)
- `{bool} required` - is required (true/false)
- `{string} description` - metadata description

### Example

```bash
$ cleos push action admin.yield setmetakey '[website, url, true, "Protocol website"]' -p admin.yield
```

## ACTION `setcategory`

> Set category

- **authority**: `get_self()`

### params

- `{name} category` - category
- `{string} description` - category description

### Example

```bash
$ cleos push action admin.yield setcategory '[dexes, "Protocols where you can swap/trade cryptocurrency"]' -p admin.yield
```

## ACTION `delcategory`

> Delete category

- **authority**: `get_self()`

### params

- `{name} category` - category

### Example

```bash
$ cleos push action admin.yield delcategory '[dexes]' -p admin.yield
```

## ACTION `delmetakeys`

> Delete metakey

- **authority**: `get_self()`

### params

- `{name} key` - metadata key

### Example

```bash
$ cleos push action admin.yield delmetakey '[website]' -p admin.yield
```