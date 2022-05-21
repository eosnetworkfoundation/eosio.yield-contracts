// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// oracles
#include <oracle.defi/oracle.defi.hpp>
#include <delphioracle/delphioracle.hpp>

// sibling
#include <eosio.yield/eosio.yield.hpp>

// local
#include "./oracle.yield.hpp"
#include "./src/notifiers.cpp"

// @oracle
[[eosio::action]]
void oracle::regoracle( const name oracle, const map<name, string> metadata )
{
    require_auth( oracle );

    oracle::oracles_table _oracles( get_self(), get_self().value );
    const auto config = get_config();
    const set<name> metadata_keys = config.metadata_keys;

    // validate input
    check(config.status == "active"_n, "yield::regoracle: [status] must be `active`");
    for ( const auto item : metadata ) {
        check( metadata_keys.find(item.first) != metadata_keys.end(), "yield::regoracle: invalid [metadata_keys]");
    }

    auto insert = [&]( auto& row ) {
        // status => "pending" by default
        row.oracle = oracle;
        row.metadata = metadata;
        row.balance.contract = TOKEN_CONTRACT;
        row.balance.quantity.symbol = TOKEN_SYMBOL;
        row.claimed.symbol = TOKEN_SYMBOL;
        if ( !row.created_at.sec_since_epoch() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    // modify or create
    auto itr = _oracles.find( oracle.value );
    if ( itr == _oracles.end() ) _oracles.emplace( get_self(), insert );
    else _oracles.modify( itr, get_self(), insert );
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

void oracle::set_status( const name oracle, const name status )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );

    auto & itr = _oracles.get(oracle.value, "oracle::set_status: [protocol] does not exists");
    check( ORACLE_STATUS_TYPES.find( status ) != ORACLE_STATUS_TYPES.end(), "oracle::set_status: [status] is invalid");

    _oracles.modify( itr, same_payer, [&]( auto& row ) {
        check( row.status != status, "oracle::set_status: [status] not modified");
        row.status = status;
    });
}

void oracle::check_oracle_active( const name oracle )
{
    oracle::oracles_table _oracles( get_self(), get_self().value );
    auto & itr = _oracles.get(oracle.value, "oracle::set_status: [protocol] does not exists");
    check( itr == "active"_n, "oracle::check_oracle_active: [status] must be active");
}

// @system
[[eosio::action]]
void oracle::addtoken( const symbol_code symcode, const name contract, const optional<uint64_t> defibox_oracle_id, const optional<name> delphi_oracle_id )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );

    check( _tokens.find( symcode.raw() ) == _tokens.end(), "oracle::addtoken: [symcode] already exists");

    // validate
    const asset supply = eosio::token::get_supply( contract, symcode );
    check( supply.amount > 0,  "oracle::addtoken: [supply] is none");
    // TO-DO Delphi Oracle checks
    // TO-DO Defibox Oracle checks

    // add supported token
    _tokens.emplace( get_self(), [&]( auto& row ) {
        row.sym = supply.symbol;
        row.contract = contract;
        row.defibox_oracle_id = *defibox_oracle_id;
        row.delphi_oracle_id = *delphi_oracle_id;
    });
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
void oracle::update( const name oracle, const name protocol )
{
    require_auth( oracle );
    check_oracle_active( oracle );

    // tables
    oracle::tvl_table _tvl( get_self(), get_self().value );
    yield::protocols_table _protocols( get_self(), get_self().value );
    oracle::tokens_table _tokens( get_self(), get_self().value );

    // get protocol details
    const time_point_sec period = get_current_period();
    const auto contracts = _protocols.get( protocol.value, "oracle::update: [protocol] does not exists" ).contracts;
    auto & itr = _tvl.get( protocol.value, "oracle::update: [tvl.protocol] does not exists" );

    // prevent duplicate updates
    check( itr.period_at != period, "oracle::update: [tvl.period] tvl already updated" );

    // get all balances from protocol EOS contracts
    vector<asset> balances;
    for ( const name contract : contracts.eos ) {
        for ( const auto token : _tokens ) {
            // liquid balance
            const asset balance = get_balance_quantity( contract, token.contract, token.sym );
            if ( balance.amount <= 0 ) continue;
            balances.push_back( balance );

            // staked EOS (REX & delegated CPU/NET)
            const asset staked = get_eos_staked( contract );
            if ( staked.amount <= 0 ) continue;
            balances.push_back( staked );
        }
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

    // add TVL to history
    _tvl.modify( itr, get_self(), [&]( auto& row ) {
        row.period_at = period;
        row.history[period] = TVL{ balances, usd, eos };
    });

    // report
    generate_report( oracle, period, itr.history );

    // log event
    oracle::updatelog_action updatelog( get_self(), { get_self(), "active"_n });
    updatelog.send( oracle, protocol, period, usd, eos, balances );
}

// generate report TVL to Yield+ Rewards
void oracle::generate_report( const name protocol, const time_point_sec period, const map<time_point_sec, TVL> history )
{
    // TO-DO make sure report is valid (3x48 TVL buckets)
    if ( false ) return;

    oracle::report_action report( get_self(), { get_self(), "active"_n });
    report.send( protocol, period, usd, eos );
}

int64_t oracle::compute_average_tvl( )
{
    // TO-DO
}

// @oracle
[[eosio::action]]
void oracle::updateall( const name oracle, const optional<uint16_t> max_rows )
{
    require_auth( oracle );

    oracle::tvl_table _tvl( get_self(), get_self().value );
    const time_point_sec period = get_current_period();

    int limit = max_rows ? *max_rows : 20;
    for ( const auto row : _tvl ) {
        if ( row.period_at == period ) continue;
        update( oracle, row.protocol );
        limit -= 1;
        if ( limit <= 0 ) break;
    }
}

// @eosio.code
[[eosio::action]]
void yield::updatelog( const name protocol, const time_point_sec period, const vector<asset> balances, const int64_t usd, const int64_t eos )
{
    require_auth( get_self() );
}

// @system
[[eosio::action]]
void oracle::setreward( const asset reward_per_update )
{
    require_auth( get_self() );

    oracle::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();
    check( reward_per_update.symbol == TOKEN_SYMBOL, "oracle::setreward: [symbol] is invalid");
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

// @eosio.code
[[eosio::action]]
void oracle::report( const name protocol, const time_point_sec period, const int64_t eos, const int64_t usd )
{
    require_auth( get_self() );

    require_recipient(YIELD_CONTRACT);
}

time_point_sec oracle::get_current_period()
{
    const uint32_t now = current_time_point().sec_since_epoch();
    return time_point_sec((now / PERIOD_INTERVAL) * PERIOD_INTERVAL);
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
    return ( price * usd ) / pow( 10, PRECISION );
}

int64_t oracle::get_oracle_price( const symbol_code symcode )
{
    oracle::tokens_table _tokens( get_self(), get_self().value );
    auto token = _tokens.get( symcode.raw(), "oracle::calculate_usd_value: [symbol] does not exists");

    // Defibox Oracle
    const int64_t price1 = get_defibox_price( token.defibox_oracle_id );

    // Delphi Oracle
    const int64_t price2 = get_delphi_price( token.delphi_oracle_id );

    // in case oracles do not exists
    if ( !price2 ) return price1;
    if ( !price1 ) return price2;

    // TO-DO add price variations checks
    check( true, "oracle::get_oracle_price: invalid price deviation");

    // average price
    return ( price1 + price2 ) / 2;
}

int64_t oracle::get_delphi_price( const name delphi_oracle_id )
{
    delphioracle::pairstable _pairs( DELPHI_ORACLE_CONTRACT, DELPHI_ORACLE_CONTRACT.value );
    delphioracle::datapointstable _datapoints( DELPHI_ORACLE_CONTRACT, delphi_oracle_id.value );
    const auto pairs = _pairs.get(delphi_oracle_id.value, "oracle::get_delphi_price: [delphi_oracle_id] does not exists");
    const auto datapoints = _datapoints.begin();
    check(datapoints != _datapoints.end(), "oracle::get_delphi_price: [delphi_oracle_id] is empty");
    return normalize_price(datapoints->median, pairs.quoted_precision);
}

int64_t oracle::get_defibox_price( const uint64_t defibox_oracle_id )
{
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