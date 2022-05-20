// eosio
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/eosio.system.hpp>

// oracles
#include <oracle.defi/oracle.defi.hpp>
#include <delphioracle/delphioracle.hpp>

// internal
#include <eosio.yield/eosio.yield.hpp>
#include "./oracle.yield.hpp"

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

[[eosio::on_notify("eosio.yield::approve")]]
void oracle::on_approve( const name protocol )
{
    register_protocol( protocol );
}

[[eosio::on_notify("eosio.yield::deny")]]
void oracle::on_deny( const name protocol )
{
    erase_protocol( protocol );
}

[[eosio::on_notify("eosio.yield::unregister")]]
void oracle::on_unregister( const name protocol )
{
    erase_protocol( protocol );
}

void oracle::erase_protocol( const name protocol )
{
    oracle::tvl_table _tvl( get_self(), get_self().value );
    auto itr = _tvl.find( protocol.value );
    if ( itr != _tvl.end() ) _tvl.erase( itr );
}

void oracle::register_protocol( const name protocol )
{
    oracle::tvl_table _tvl( get_self(), get_self().value );
    auto itr = _tvl.find( protocol.value );
    if ( itr != _tvl.end() ) return; // skip already exists

    // create initial protocol
    _tvl.emplace( get_self(), [&]( auto& row ) {
        row.protocol = protocol;
    });
}

// @oracle
[[eosio::action]]
void oracle::update( const name oracle, const name protocol )
{
    require_auth( oracle );

    oracle::tvl_table _tvl( get_self(), get_self().value );
    yield::protocols_table _protocols( get_self(), get_self().value );
    oracle::tokens_table _tokens( get_self(), get_self().value );

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

    // update token TVL
    _tvl.modify( itr, get_self(), [&]( auto& row ) {
        row.period_at = period;
        row.tvl[period] = TVL{ balances, usd, eos };
    });

    // report to Yield+
    oracle::report_action report( get_self(), { get_self(), "active"_n });
    report.send( protocol, period, usd, eos );
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
void oracle::report( const name protocol, const time_point_sec period, const int64_t eos, const int64_t usd )
{
    require_auth( get_self() );

    require_recipient("eosio.yield"_n);
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
