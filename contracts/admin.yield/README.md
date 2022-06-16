# EOSIO Yield+ - Admin

# Overview

Admin account for Yield+ on-notification events

## Actions

- Checks protocol metadata
- Checks category value

## Quickstart

### `ADMIN` (Operators)

```bash
# set metakey
$ cleos push action admin.yield setmetakey '[url, true, "Protocol URL"]' -p admin.yield

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
- `{bool} required` - is required (true/false)
- `{string} description` - metadata description

### example

```json
{
    "key": "name",
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
- `{bool} required` - is required (true/false)
- `{string} description` - metadata description

### Example

```bash
$ cleos push action admin.yield setmetakey '[url, true, "Protocol URL"]' -p admin.yield
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
$ cleos push action admin.yield delmetakey '[url]' -p admin.yield
```