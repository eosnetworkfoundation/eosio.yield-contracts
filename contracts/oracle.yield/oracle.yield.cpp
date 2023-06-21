// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// oracles
#include <oracle.defi/oracle.defi.hpp>
#include <delphioracle/delphioracle.hpp>

// core
#include <oracle.yield/oracle.yield.hpp>

// logging (used for backend syncing)
#include "src/logs.cpp"

// EOS EVM support
#include "src/evm.cpp"

// DEBUG (used to help testing)
#ifdef DEBUG
#include "src/debug.cpp"
#endif

// @oracle
[[eosio::action]]
void oracle::regoracle( const name oracle, const map<name, string> metadata )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    const auto config = get_config();

    auto insert = [&]( auto& row ) {
        row.oracle = oracle;
        row.metadata = metadata;
        row.balance.contract = config.reward_per_update.contract;
        row.balance.quantity.symbol = config.reward_per_update.quantity.symbol;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };


    // modify or create
    auto itr = _oracles.find( oracle.value );
    const bool is_exists = itr != _oracles.end();
    if ( is_exists ) _oracles.modify( itr, oracle, insert );
    else _oracles.emplace( oracle, insert );

    // if denied revert back to pending
    if ( itr->status == "denied"_n ) set_status(oracle, "pending"_n);

    // logging
    oracle::createlog_action createlog( get_self(), { get_self(), "active"_n });
    oracle::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });

    if ( !is_exists ) createlog.send( oracle, "pending"_n, "oracle"_n, metadata );
    else metadatalog.send( oracle, itr->status, "oracle"_n, metadata );
}

// @protocol
[[eosio::action]]
void oracle::unregister( const name oracle )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get(oracle.value, "oracle::unregister: [oracle] does not exists");
    _oracles.erase( itr );

    // logging
    oracle::eraselog_action eraselog( get_self(), { get_self(), "active"_n });
    eraselog.send( oracle );
}

// @admin
[[eosio::action]]
void oracle::approve( const name oracle )
{
    require_auth_admin();
    set_status( oracle, "active"_n );
}

// @admin
[[eosio::action]]
void oracle::deny( const name oracle )
{
    require_auth_admin();
    set_status( oracle, "denied"_n );
}

// @protocol OR @admin
[[eosio::action]]
void oracle::setmetadata( const name oracle, const map<name, string> metadata )
{
    const auto config = get_config();
    const bool is_admin = has_auth( config.admin_contract );
    if ( !is_admin ) require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get( oracle.value, "oracle::setmetadata: [oracle] does not exists");

    const name ram_payer = is_admin ? config.admin_contract : oracle;
    _oracles.modify( itr, ram_payer, [&]( auto& row ) {
        row.metadata = metadata;
        row.updated_at = current_time_point();
    });

    // if denied revert back to pending
    if ( itr.status == "denied"_n ) set_status(oracle, "pending"_n);

    // logging
    oracle::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });
    metadatalog.send( oracle, itr.status, "oracle"_n, metadata );
}

// @oracle OR @admin
[[eosio::action]]
void oracle::setmetakey( const name oracle, const name key, const optional<string> value )
{
    const auto config = get_config();
    const bool is_admin = has_auth( config.admin_contract );
    if ( !is_admin ) require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get( oracle.value, "oracle::setmetakey: [oracle] does not exists");

    const name ram_payer = is_admin ? config.admin_contract : oracle;
    _oracles.modify( itr, ram_payer, [&]( auto& row ) {
        if ( value ) row.metadata[key] = *value;
        else row.metadata.erase(key);
        row.updated_at = current_time_point();
    });

    // if denied revert back to pending
    if ( itr.status == "denied"_n ) set_status(oracle, "pending"_n);

    // logging
    oracle::metadatalog_action metadatalog( get_self(), { get_self(), "active"_n });
    metadatalog.send( oracle, itr.status, "oracle"_n, itr.metadata );
}

// @oracle
[[eosio::action]]
void oracle::claim( const name oracle, const optional<name> receiver )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    const auto config = get_config();

    if ( receiver ) check( is_account( *receiver ), "oracle::claim: [receiver] does not exists");

    // validate
    auto & itr = _oracles.get(oracle.value, "oracle::claim: [oracle] does not exists");
    const extended_asset claimable = itr.balance;
    check( itr.status == "active"_n, "oracle::claim: [status] must be `active`");
    check( claimable.quantity.amount > 0, "oracle::claim: nothing to claim");

    // check oracle.yield balance
    const asset balance = eosio::token::get_balance( claimable.contract, get_self(), claimable.quantity.symbol.code() );
    check( balance >= claimable.quantity, "oracle::claim: contract has insuficient balance, please contact administrator");

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
    claimlog.send( oracle, "oracle"_n, to, claimable.quantity, itr.balance.quantity );
}

void oracle::set_status( const name oracle, const name status )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );

    auto & itr = _oracles.get(oracle.value, "oracle::set_status: [oracle] does not exists");
    check( ORACLE_STATUS_TYPES.find( status ) != ORACLE_STATUS_TYPES.end(), "oracle::set_status: [status] is invalid");

    if ( itr.status == status ) return; // no status change
    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        row.status = status;
    });

    // logging
    oracle::statuslog_action statuslog( get_self(), { get_self(), "active"_n });
    statuslog.send( oracle, status );
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
    check( supply.amount > 0,  "oracle::addtoken: [supply] has no supply");
    const bool is_usdt = symcode == USDT.code() && contract == USDT_CONTRACT;
    if ( is_usdt ) check( !*defibox_oracle_id && !delphi_oracle_id->value, "oracle::addtoken: USDT Tether does not require any oracle ID");
    else check( *defibox_oracle_id || delphi_oracle_id->value, "oracle::addtoken: must provide at least one oracle ID");


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
        if ( defibox_oracle_id ) row.defibox_oracle_id = *defibox_oracle_id;
        if ( delphi_oracle_id ) row.delphi_oracle_id = *delphi_oracle_id;
    };

    // modify or create
    auto itr = _tokens.find( symcode.raw() );
    if ( itr == _tokens.end() ) _tokens.emplace( get_self(), insert );
    else _tokens.modify( itr, get_self(), insert );
}

// @system
[[eosio::action]]
void oracle::init( const extended_symbol rewards, const name yield_contract, const name admin_contract )
{
    require_auth( get_self() );

    oracle::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();

    // check if accounts exists
    check( is_account( rewards.get_contract() ), "oracle::init: [rewards.contract] account does not exists");
    check( is_account( yield_contract ), "oracle::init: [yield_contract] account does not exists");
    check( is_account( admin_contract ), "oracle::init: [admin_contract] account does not exists");

    // validate rewards token
    const asset supply = eosio::token::get_supply( rewards.get_contract(), rewards.get_symbol().code() );
    check( supply.symbol == rewards.get_symbol(),  "oracle::init: [supply.symbol] does not match [rewards]");

    // cannot modify existing values
    if ( config.reward_per_update.contract ) check( config.reward_per_update.get_extended_symbol() == rewards, "oracle::init: [rewards] cannot be modified once initialized");

    // set values
    config.reward_per_update.contract = rewards.get_contract();
    config.reward_per_update.quantity.symbol = rewards.get_symbol();
    config.yield_contract = yield_contract;
    config.admin_contract = admin_contract;
    _config.set(config, get_self());
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

    auto config = get_config();
    yield::protocols_table _protocols( config.yield_contract, config.yield_contract.value );
    yield::state_table _state( config.yield_contract, config.yield_contract.value );
    const time_point_sec period = get_current_period( PERIOD_INTERVAL );
    check( _state.exists(), "oracle::updateall: [yield_contract.state] does not exists");
    auto state = _state.get();

    int limit = max_rows ? *max_rows : 20;
    int count = 0;
    check( limit, "oracle::updateall: [max_rows] must be above 0");

    for ( const name protocol : state.active_protocols ) {
        // skip based on oracle
        auto itr = _protocols.get( protocol.value, "oracle::updateall: [yield_contract.protocols] does not exists");
        oracle::periods_table _periods( get_self(), protocol.value );
        auto period_itr = _periods.find( period.sec_since_epoch()  ); // inverse multi-index
        if ( period_itr != _periods.end() ) continue; // period already updated

        // skip based on protocol
        if ( itr.period_at == period ) continue; // protocol period already updated
        if ( itr.status != "active"_n ) continue; // protocol not active
        update( oracle, protocol );
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
    auto config = get_config();
    oracle::tokens_table _tokens( get_self(), get_self().value );
    oracle::oracles_table _oracles( get_self(), get_self().value );
    oracle::periods_table _periods( get_self(), protocol.value );
    yield::protocols_table _protocols( config.yield_contract, config.yield_contract.value );

    // get oracle
    const auto oracle_itr = _oracles.get( oracle.value, "oracle::update: [oracle] does not exists" );
    check( oracle_itr.status == "active"_n, "oracle::update: [oracle] must be active" );

    // get protocol details
    const auto protocol_itr = _protocols.get( protocol.value, "oracle::update: [protocol] does not exists" );
    check( protocol_itr.status == "active"_n, "oracle::update: [protocol] must be active" );

    // get current period
    const time_point_sec period = get_current_period( PERIOD_INTERVAL );
    auto itr = _periods.find( period.sec_since_epoch() ); // inverse multi-index
    check( itr == _periods.end(), "oracle::update: [period] for [protocol] is already updated" );

    // contracts
    const set<name> contracts = protocol_itr.contracts;
    const set<string> evm = protocol_itr.evm;
    const name category = protocol_itr.category;

    // get all balances from protocol EOS contracts
    vector<asset> balances;
    vector<asset> prices;
    for ( const name contract : contracts ) {
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

    for ( const string contract : evm ) {
        check(false, "NOT IMPLEMENTED");
    }

    // calculate USD valuation
    int64_t usd_amount = 0;
    for ( const asset balance : balances ) {
        usd_amount += calculate_usd_value( balance );
    }

    // calculate EOS valuation
    const int64_t eos = convert_usd_to_eos( usd_amount );
    const asset tvl = { eos, EOS };
    const asset usd = { usd_amount, USD };

    // add TVL to history
    _periods.emplace( get_self(), [&]( auto& row ) {
        row.period = period;
        row.protocol = protocol;
        row.category = category;
        row.contracts = contracts;
        row.evm = evm;
        row.balances = balances;
        row.prices = prices;
        row.tvl = tvl;
        row.usd = usd;
    });

    // log update
    oracle::updatelog_action updatelog( get_self(), { get_self(), "active"_n });
    updatelog.send( oracle, protocol, category, contracts, evm, period, balances, prices, tvl, usd );

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

    // logging
    oracle::rewardslog_action rewardslog( get_self(), { get_self(), "active"_n });
    rewardslog.send( oracle, config.reward_per_update.quantity, itr.balance.quantity );
}

void oracle::prune_protocol_periods( const name protocol )
{
    oracle::periods_table _periods( get_self(), protocol.value );

    // TO-DO validate if pruning is still functional

    // erase any periods that exceeds 24 hours
    const time_point_sec last_period = get_last_period( PERIOD_INTERVAL * MAX_PERIODS_REPORT );
    auto itr = _periods.begin();
    while ( itr != _periods.end() ) {
        if ( itr->period <= last_period ) itr = _periods.erase( itr ); // erase
        if ( itr != _periods.end()) itr++; // continue
    }
}

// generate report TVL to Yield+ Rewards
void oracle::generate_report( const name protocol, const time_point_sec period )
{
    // yield config
    auto config = get_config();
    asset tvl = { 0, EOS };
    asset usd = { 0, USD };

    // slice values into 3 buckets of 8 hours each
    uint64_t current_time_sec = current_time_point().sec_since_epoch();
    uint64_t period_1 = current_time_sec - EIGHT_HOURS * 3;
    uint64_t period_2 = period_1 + EIGHT_HOURS;
    uint64_t period_3 = period_2 + EIGHT_HOURS;

    // retrieve datapoint from timepoint
    // skip generating report if any median contains no TVL
    auto median_1 = get_median( protocol, period_1, period_2 );
    if ( !median_1.tvl.amount ) return;

    auto median_2 = get_median( protocol, period_2, period_3 );
    if ( !median_2.tvl.amount ) return;

    auto median_3 = get_median( protocol, period_3, current_time_sec );
    if ( !median_3.tvl.amount ) return;

    // compute the average of the 3 windows median
    tvl += (median_1.tvl + median_2.tvl + median_3.tvl ) / 3;
    usd += (median_1.usd + median_2.usd + median_3.usd ) / 3;

    // send oracle report to Yield+ Rewards
    yield::report_action report( config.yield_contract, { get_self(), "active"_n });
    report.send( protocol, period, PERIOD_INTERVAL, tvl, usd );
}

oracle::periods_row oracle::get_median( const name protocol, const uint64_t period_start, const uint64_t period_end )
{
    oracle::periods_table _periods( get_self(), protocol.value );

    // find limit pointers
    auto start = _periods.upper_bound(period_start);
    auto end = _periods.upper_bound(period_end);

    // calculate how many datapoints we have in each window of 8 hours
    int count = std::distance(start, end);

    // verify if the number of datapoints for each 8 hours window is within acceptable range, return if any is outside of the range
    if (count < MIN_BUCKET_PERIODS || count > BUCKET_PERIODS ) return {};

    // add datapoints to vectors for sorting (first value is amount as uint64_t, second is datapoint time_point)
    std::vector<std::pair<uint64_t, time_point>> datapoints;
    for (int i = 0; i < count; i++) {
        datapoints.push_back({ start->tvl.amount, start->period });
        start++;
    }

    // run a partial sort to find the median datapoint for each 8 hours window
    int half = count / 2;
    nth_element(datapoints.begin(), datapoints.begin() + half, datapoints.end());
    auto median = datapoints[half];

    // retrieve datapoint from timepoint
    return _periods.get( median.second.sec_since_epoch() );
}

// @system
[[eosio::action]]
void oracle::setreward( const asset reward_per_update )
{
    require_auth( get_self() );

    oracle::config_table _config( get_self(), get_self().value );
    auto config = get_config();
    check( config.reward_per_update.quantity.symbol == reward_per_update.symbol, "oracle::setreward: [reward_per_update] symbol does not match");
    config.reward_per_update.quantity = reward_per_update;
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
    const int64_t price1 = get_defibox_price( *token.defibox_oracle_id );

    // Delphi Oracle
    const int64_t price2 = get_delphi_price( *token.delphi_oracle_id );

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
    return _config.get_or_default();
}

void oracle::transfer( const name from, const name to, const extended_asset value, const string& memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}

void oracle::require_auth_admin()
{
    require_auth( get_config().admin_contract );
}

void oracle::require_auth_admin( const name account )
{
    if ( has_auth( get_config().admin_contract ) ) return;
    require_auth( account );
}