// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// oracles
#include <oracle.defi/oracle.defi.hpp>
#include <delphioracle/delphioracle.hpp>

// local
#include "./oracle.yield.hpp"

// @oracle
[[eosio::action]]
void oracle::regoracle( const name oracle, const map<name, string> metadata )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    const auto config = get_config();
    const set<name> metadata_keys = config.metadata_keys;

    // validate input
    for ( const auto item : metadata ) {
        check( metadata_keys.find(item.first) != metadata_keys.end(), "yield::regoracle: invalid [metadata_keys]");
    }

    auto insert = [&]( auto& row ) {
        // status => "pending" by default
        row.oracle = oracle;
        row.metadata = metadata;
        row.balance.contract = TOKEN_CONTRACT;
        row.balance.quantity.symbol = TOKEN_SYMBOL;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    auto itr = _oracles.find( oracle.value );
    if ( itr == _oracles.end() ) _oracles.emplace( oracle, insert );
    else _oracles.modify( itr, oracle, insert );
}

// @protocol
[[eosio::action]]
void oracle::unregister( const name oracle )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get(oracle.value, "oracle::unregister: [oracle] does not exists");
    _oracles.erase( itr );
}

// @admin
[[eosio::action]]
void oracle::approve( const name oracle )
{
    require_auth( get_self() );
    set_status( oracle, "active"_n );
}

// @admin
[[eosio::action]]
void oracle::deny( const name oracle )
{
    require_auth( get_self() );
    set_status( oracle, "denied"_n );
}

// @oracle
[[eosio::action]]
void oracle::claim( const name oracle, const optional<name> receiver )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );

    if ( receiver ) check( is_account( *receiver ), "oracle::claim: [receiver] does not exists");

    // validate
    auto & itr = _oracles.get(oracle.value, "oracle::claim: [oracle] does not exists");
    const extended_asset claimable = itr.balance;
    check( itr.status == "active"_n, "oracle::claim: [status] must be `active`");
    check( claimable.quantity.amount > 0, "oracle::claim: nothing to claim");

    // transfer funds to receiver
    const name to = receiver ? *receiver : oracle;
    transfer( get_self(), to, claimable, "Yield+ Oracle reward");

    // modify balances
    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        row.balance.quantity.amount = 0;
        row.claimed_at = current_time_point();
    });

    // logging
    oracle::claimlog_action claimlog( get_self(), { get_self(), "active"_n });
    claimlog.send( oracle, to, claimable );
}

// @eosio.code
[[eosio::action]]
void oracle::claimlog( const name protocol, const name receiver, const extended_asset claimed )
{
    require_auth( get_self() );
}

void oracle::set_status( const name oracle, const name status )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );

    auto & itr = _oracles.get(oracle.value, "oracle::set_status: [oracle] does not exists");
    check( ORACLE_STATUS_TYPES.find( status ) != ORACLE_STATUS_TYPES.end(), "oracle::set_status: [status] is invalid");

    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        check( row.status != status, "oracle::set_status: [status] not modified");
        row.status = status;
    });
}

void oracle::check_oracle_active( const name oracle )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get(oracle.value, "oracle::check_oracle_active: [oracle] does not exists, must first call [regoracle] action");
    check( itr.status == "active"_n, "oracle::check_oracle_active: [status] must be active");
}

// @system
[[eosio::action]]
void oracle::addtoken( const symbol_code symcode, const name contract, const optional<uint64_t> defibox_oracle_id, const optional<name> delphi_oracle_id )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );
    delphioracle::pairstable _pairs( DELPHI_ORACLE_CONTRACT, DELPHI_ORACLE_CONTRACT.value );
    delphioracle::datapointstable _datapoints( DELPHI_ORACLE_CONTRACT, delphi_oracle_id->value );
    defi::oracle::prices _prices( DEFIBOX_ORACLE_CONTRACT, DEFIBOX_ORACLE_CONTRACT.value);

    // validate
    const asset supply = eosio::token::get_supply( contract, symcode );
    check( supply.amount > 0,  "oracle::addtoken: [supply] is none");
    if ( symcode != USDT.code() ) check( *defibox_oracle_id || delphi_oracle_id->value, "oracle::addtoken: must provide at least one oracle ID");

    // validate oracles
    if ( delphi_oracle_id ) {
        const auto pairs = _pairs.get(delphi_oracle_id->value, "oracle::addtoken: [delphi_oracle_id] does not exists");
        const auto datapoints = _datapoints.rbegin();
        check(!pairs.quote_contract.value, "oracle::addtoken: [delphi_oracle_id.quote_contract] must be empty");
        check(pairs.quote_symbol == symbol{"USD", 2}, "oracle::addtoken: [delphi_oracle_id.quote_symbol] must be 2,USD");
        check(pairs.base_symbol == supply.symbol, "oracle::addtoken: [delphi_oracle_id.base_symbol] does not match");
        check(pairs.base_contract == contract, "oracle::addtoken: [delphi_oracle_id.base_contract] does not match");
        check(datapoints->median, "oracle::addtoken: [delphi_oracle_id.median] is empty");
    }
    if ( defibox_oracle_id ) {
        const auto prices = _prices.get(*defibox_oracle_id, "oracle::addtoken: [defibox_oracle_id] does not exists");
        check(prices.contract == contract, "oracle::addtoken: [defibox_oracle_id.contract] does not match");
        check(prices.coin == symcode, "oracle::addtoken: [defibox_oracle_id.coin] does not match");
        check(prices.precision == supply.symbol.precision(), "oracle::addtoken: [defibox_oracle_id.precision] does not match");
        check(prices.avg_price, "oracle::addtoken: [defibox_oracle_id.avg_price] is empty");
    }

    // add supported token
    auto insert = [&]( auto& row ) {
        if ( row.contract ) check(row.contract == contract, "oracle::addtoken: [contract] cannot be modified once token is created");
        row.sym = supply.symbol;
        row.contract = contract;
        row.defibox_oracle_id = *defibox_oracle_id;
        row.delphi_oracle_id = *delphi_oracle_id;
    };

    // modify or create
    auto itr = _tokens.find( symcode.raw() );
    if ( itr == _tokens.end() ) _tokens.emplace( get_self(), insert );
    else _tokens.modify( itr, get_self(), insert );
}

// @system
[[eosio::action]]
void oracle::deltoken( const symbol_code symcode )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );
    auto & itr = _tokens.get( symcode.raw(), "oracle::deltoken: [symcode] does not exists" );
    _tokens.erase( itr );
}

// @oracle
[[eosio::action]]
void oracle::updateall( const name oracle, const optional<uint16_t> max_rows )
{
    require_auth( oracle );

    yield::protocols_table _protocols( YIELD_CONTRACT, YIELD_CONTRACT.value );
    const time_point_sec period = get_current_period( PERIOD_INTERVAL );

    int limit = max_rows ? *max_rows : 20;
    int count = 0;
    check( limit, "oracle::updateall: [max_rows] must be above 0");

    for ( const auto row : _protocols ) {
        // skip based on oracle
        oracle::periods_table _periods( get_self(), row.protocol.value );
        auto period_itr = _periods.find( period.sec_since_epoch() * -1 ); // inverse multi-index
        if ( period_itr != _periods.end() ) continue; // period already updated

        // skip based on protocol
        if ( row.period_at == period ) continue; // protocol period already updated
        if ( row.status != "active"_n ) continue; // protocol not active
        update( oracle, row.protocol );
        count += 1;
        if ( count >= limit ) break;
    }
    check( count, "oracle::updateall: nothing to update");
}

// @oracle
[[eosio::action]]
void oracle::update( const name oracle, const name protocol )
{
    require_auth( oracle );
    check_oracle_active( oracle );

    // tables
    oracle::tokens_table _tokens( get_self(), get_self().value );
    oracle::periods_table _periods( get_self(), protocol.value );
    yield::protocols_table _protocols( YIELD_CONTRACT, YIELD_CONTRACT.value );

    // get protocol details
    const auto protocol_itr = _protocols.get( protocol.value, "oracle::update: [protocol] does not exists" );
    check( protocol_itr.status == "active"_n, "oracle::update: [protocol] must be active" );
    const yield::Contracts contracts = protocol_itr.contracts;

    // get current period
    const time_point_sec period = get_current_period( PERIOD_INTERVAL );
    auto itr = _periods.find( period.sec_since_epoch() * -1 ); // inverse multi-index
    check( itr == _periods.end(), "oracle::update: [period] for [protocol] is already updated" );

    // get all balances from protocol EOS contracts
    vector<asset> balances;
    vector<asset> prices;
    for ( const name contract : contracts.eos ) {
        // liquid balance
        for ( const auto token : _tokens ) {
            const asset balance = get_balance_quantity( token.contract, contract, token.sym );
            if ( balance.amount <= 0 ) continue;
            balances.push_back( balance );

            // price only used for logging purposes
            prices.push_back( asset{ get_oracle_price( balance.symbol.code() ), USD } );
        }
        // staked EOS (REX & delegated CPU/NET)
        const asset staked = get_eos_staked( contract );
        if ( staked.amount <= 0 ) continue;
        balances.push_back( staked );

        // EOS price only used for logging purposes
        prices.push_back( asset{ get_oracle_price( EOS.code() ), USD } );
    }

    for ( const string contract : contracts.evm ) {
        check(false, "NOT IMPLEMENTED");
    }

    // calculate USD valuation
    int64_t usd = 0;
    for ( const asset balance : balances ) {
        usd += calculate_usd_value( balance );
    }

    // calculate EOS valuation
    const int64_t eos = convert_usd_to_eos( usd );
    const yield::TVL tvl = {{ usd, USD }, { eos, EOS }};

    // add TVL to history
    _periods.emplace( get_self(), [&]( auto& row ) {
        row.period = period;
        row.protocol = protocol;
        row.contracts = contracts;
        row.balances = balances;
        row.prices = prices;
        row.tvl = tvl;
    });

    // log update
    oracle::updatelog_action updatelog( get_self(), { get_self(), "active"_n });
    updatelog.send( oracle, protocol, contracts, period, balances, prices, tvl );

    // prune last 24 hours
    prune_protocol_periods( protocol );

    // report
    generate_report( protocol, period );

    // update rewards
    allocate_oracle_rewards( oracle );
}

void oracle::allocate_oracle_rewards( const name oracle )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto config = get_config();

    auto & itr = _oracles.get(oracle.value, "oracle::add_oracle_rewards: [oracle] does not exists");
    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        row.balance += config.reward_per_update;
    });
}

void oracle::prune_protocol_periods( const name protocol )
{
    oracle::periods_table _periods( get_self(), protocol.value );

    // erase any periods that exceeds 32 hours
    const time_point_sec last_period = get_last_period( PERIOD_INTERVAL * MAX_PERIODS_REPORT );
    auto itr = _periods.begin();
    while ( itr != _periods.end() ) {
        if ( itr->period < last_period ) itr = _periods.erase( itr ); // erase
        if ( itr != _periods.end()) itr++; // continue
    }
}

// generate report TVL to Yield+ Rewards
void oracle::generate_report( const name protocol, const time_point_sec period )
{
    oracle::periods_table _periods( get_self(), protocol.value );

    // yield config
    yield::config_table _config( YIELD_CONTRACT, YIELD_CONTRACT.value );
    check( _config.exists(), "yield::get_config: contract is not initialized");
    const asset min_tvl_report = _config.get().min_tvl_report;

    // *****
    // TO-DO make sure report is valid (3x48 TVL buckets)
    // *****

    // calculates average TVL of period bucket 42 periods (~7 hours)
    int count = 0;
    yield::TVL tvl = {{ 0, USD }, { 0, EOS }};
    for ( const auto row : _periods ) {
        if ( count >= MIN_BUCKET_PERIODS ) break; // stop sum when minimum periods has been reached
        tvl.usd += row.tvl.usd;
        tvl.eos += row.tvl.eos;
        count += 1;
    }
    if ( count < MIN_BUCKET_PERIODS ) return; // skip if does not exceeed minimum periods
    tvl.usd /= count;
    tvl.eos /= count;

    // send oracle report to Yield+ Rewards
    yield::report_action report( YIELD_CONTRACT, { get_self(), "active"_n });
    report.send( protocol, period, PERIOD_INTERVAL, tvl );
}

// int64_t oracle::compute_average_tvl( )
// {
//     // TO-DO
//     return 0;
// }

// @eosio.code
[[eosio::action]]
void oracle::updatelog( const name oracle, const name protocol, const yield::Contracts contracts, const time_point_sec period, const vector<asset> balances, const vector<asset> prices, const yield::TVL tvl )
{
    require_auth( get_self() );
}

// @system
[[eosio::action]]
void oracle::setreward( const extended_asset reward_per_update )
{
    require_auth( get_self() );

    oracle::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( reward_per_update.quantity.symbol == TOKEN_SYMBOL, "oracle::setreward: [quantity.symbol] is invalid");
    check( reward_per_update.contract == TOKEN_CONTRACT, "oracle::setreward: [contract] is invalid");
    config.reward_per_update = reward_per_update;
    _config.set(config, get_self());
}

// @system
[[eosio::action]]
void oracle::setmetakeys( const set<name> metadata_keys )
{
    require_auth( get_self() );

    oracle::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    config.metadata_keys = metadata_keys;
    _config.set(config, get_self());
}

time_point_sec oracle::get_current_period( const uint32_t period_interval )
{
    const uint32_t now = current_time_point().sec_since_epoch();
    return time_point_sec((now / period_interval) * period_interval);
}

time_point_sec oracle::get_last_period( const uint32_t last )
{
    const uint32_t current = get_current_period( PERIOD_INTERVAL ).sec_since_epoch();
    return time_point_sec( current - last );
}

asset oracle::get_balance_quantity( const name token_contract_account, const name owner, const symbol sym )
{
    eosio::token::accounts _accounts( token_contract_account, owner.value );
    const auto itr = _accounts.find( sym.code().raw() );
    if ( itr == _accounts.end() ) return { 0, sym };
    check( itr->balance.symbol == sym, "oracle::get_balance_amount: [sym] does not match");
    return { itr->balance.amount, sym };
}

asset oracle::get_eos_staked( const name owner )
{
    eosiosystem::voters_table _voter( "eosio"_n, "eosio"_n.value );
    const auto itr = _voter.find( owner.value );
    if ( itr == _voter.end() ) return { 0, EOS };
    return { itr->staked, EOS };
}

int64_t oracle::calculate_usd_value( const asset quantity )
{
    const int64_t price = get_oracle_price( quantity.symbol.code() );
    return quantity.amount * price / pow(10, quantity.symbol.precision());
}

int64_t oracle::convert_usd_to_eos( const int64_t usd )
{
    const int64_t price = get_oracle_price( EOS.code() );
    return usd * pow( 10, PRECISION ) / price;
}

int64_t oracle::get_oracle_price( const symbol_code symcode )
{
    oracle::tokens_table _tokens( get_self(), get_self().value );
    auto token = _tokens.get( symcode.raw(), "oracle::calculate_usd_value: [symbol] does not exists");

    // USDT price = 1.0000 USD
    if ( symcode == USDT.code() ) return 10000;

    // Defibox Oracle
    const int64_t price1 = get_defibox_price( token.defibox_oracle_id );

    // Delphi Oracle
    const int64_t price2 = get_delphi_price( token.delphi_oracle_id );

    // in case oracles do not exists
    if ( !price2 && price1 ) return price1;
    if ( !price1 && price2 ) return price2;

    // TO-DO add price variations checks
    check( price1 && price2, "oracle::get_oracle_price: invalid prices");
    const int64_t average = ( price1 + price2 ) / 2;

    // assert if price deviates from average price
    check( average * (10000 + MAX_PRICE_DEVIATION) / 10000 > price1, "oracle::get_oracle_price: invalid oracle prices, [price1] exceeds deviation");
    check( average * (10000 + MAX_PRICE_DEVIATION) / 10000 > price2, "oracle::get_oracle_price: invalid oracle prices, [price2] exceeds deviation");
    check( average * (10000 - MAX_PRICE_DEVIATION) / 10000 < price1, "oracle::get_oracle_price: invalid oracle prices, [price1] below deviation");
    check( average * (10000 - MAX_PRICE_DEVIATION) / 10000 < price2, "oracle::get_oracle_price: invalid oracle prices, [price2] below deviation");

    return ( price1 + price2 ) / 2;
}

int64_t oracle::get_delphi_price( const name delphi_oracle_id )
{
    if ( !delphi_oracle_id ) return 0;
    delphioracle::pairstable _pairs( DELPHI_ORACLE_CONTRACT, DELPHI_ORACLE_CONTRACT.value );
    delphioracle::datapointstable _datapoints( DELPHI_ORACLE_CONTRACT, delphi_oracle_id.value );
    const auto pairs = _pairs.get(delphi_oracle_id.value, "oracle::get_delphi_price: [delphi_oracle_id] does not exists");
    const auto datapoints = _datapoints.rbegin();
    check(datapoints->id, "oracle::get_delphi_price: [delphi_oracle_id] is empty");
    return normalize_price(datapoints->median, pairs.quoted_precision);
}

int64_t oracle::get_defibox_price( const uint64_t defibox_oracle_id )
{
    if ( !defibox_oracle_id ) return 0;
    defi::oracle::prices _prices( DEFIBOX_ORACLE_CONTRACT, DEFIBOX_ORACLE_CONTRACT.value);
    const auto prices = _prices.get(defibox_oracle_id, "oracle::get_defibox_price: [defibox_oracle_id] does not exists");
    return normalize_price(prices.avg_price, prices.precision);
}

int64_t oracle::normalize_price( const int64_t price, const uint8_t precision )
{
    return price * pow(10, PRECISION) / pow(10, precision);
}

oracle::config_row oracle::get_config()
{
    oracle::config_table _config( get_self(), get_self().value );
    check( _config.exists(), "oracle::get_config: contract is not initialized");
    return _config.get();
}

void oracle::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}

// @debug
template <typename T>
void oracle::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

// @debug
[[eosio::action]]
void oracle::cleartable( const name table_name, const optional<name> scope, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = (!max_rows || *max_rows == 0) ? -1 : *max_rows;
    const uint64_t value = scope ? scope->value : get_self().value;

    // tables
    oracle::config_table _config( get_self(), value );
    oracle::tokens_table _tokens( get_self(), value );
    oracle::periods_table _periods( get_self(), value );
    oracle::oracles_table _oracles( get_self(), value );

    if (table_name == "tokens"_n) clear_table( _tokens, rows_to_clear );
    else if (table_name == "periods"_n) clear_table( _periods, rows_to_clear );
    else if (table_name == "oracles"_n) clear_table( _oracles, rows_to_clear );
    else if (table_name == "config"_n) _config.remove();
    else check(false, "oracle::cleartable: [table_name] unknown table to clear" );
}