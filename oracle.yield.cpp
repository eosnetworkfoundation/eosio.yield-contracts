#include <oracleyield.hpp>

// @system
[[eosio::action]]
void oracle::addtoken( const symbol sym, const name contract, const uint64_t defibox_oracle_id, const name delphi_oracle_id )
{
    require_auth( get_self() );

    oracle::tokens_table _tokens( get_self(), get_self().value );

    check( _tokens.find( sym.code().raw() ) == _tokens.end(), "oracle::addtoken: [sym] already exists");

    _tokens.emplace( get_self(), [&]( auto& row ) {
        row.sym = sym;
        row.token = token;
        row.defibox_oracle_id = defibox_oracle_id;
        row.delphi_oracle_id = delphi_oracle_id;
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

// @system
[[eosio::action]]
void oracle::setcontracts( const name protocol, const set<name> contracts )
{
    require_auth( get_self() );

    yield::tvl_table _tvl( get_self(), get_self().value );
    const auto configs = get_configs();

    // TO-DO check `eosio.yield` if matches

    auto insert = [&]( auto& row ) {
        row.protocol = protocol;
        row.contracts = contracts;
    });

    // modify or create
    auto itr = _tvl.find( protocol.value );
    if ( itr == _tvl.end() ) _tvl.emplace( get_self(), insert );
    else _tvl.modify( itr, get_self(), insert );
}

// @system
[[eosio::action]]
void oracle::delprotocol( const name protocol )
{
    require_auth( get_self() );

    oracle::tvl_table _tvl( get_self(), get_self().value );
    auto & itr = _tvl.get( protocol.value, "oracle::delprotocol: [protocol] does not exists" );
    _tvl.erase( itr );
}


// @oracle
[[eosio::action]]
void update( const name protocol )
{
    require_auth( get_self() );

    oracle::tvl_table _tvl( get_self(), get_self().value );
    oracle::tokens_table _tokens( get_self(), get_self().value );

    const time_point_sec period = get_current_period();
    auto & itr = _tvl.get( protocol.value, "oracle::update: [protocol] does not exists" );

    // prevent duplicate updates
    check( itr.period != period, "oracle::update: tvl already updated" );

    // get contract balances
    vector<asset> balances;
    for ( const name contract : itr.contracts ) {
        for ( const auto token : _tokens ) {
            const asset balance = get_balance_amount( contract, token.contract, token.sym );
            if ( balance.amount <= 0 ) continue;
            balances.push_back( balance );
        }
    }
    // calculate USD valuation
    int64_t usd = 0;
    for ( const asset balance : balances ) {
        usd += calculate_usd_value( balance );
    }

    // calculate EOS valuation
    int64_t eos = convert_usd_to_eos( usd );

    // update token TVL
    _tvl.modify( itr, get_self(), [&]( auto& row ) {
        row.period = period;
        row.tvl[period] = TVL{ balances, usd, eos };
    });
}

// @oracle
[[eosio::action]]
void updateall()
{
    require_auth( get_self() );

    const time_point_sec period = get_current_period();

    limit = 3;
    for ( const auto row : _tvl ) {
        if ( row.period == period ) continue;
        update( row.protocol );
        limit -= 1;
        if ( limit <= 0 ) break;
    }
}

time_point_sec oracle::get_current_period()
{
    const uint32_t = current_time_point().sec_since_epoch();
    return time_point_sec((now / PERIOD_INTERVAL) * PERIOD_INTERVAL);
}

asset oracle::get_balance_amount( const name& token_contract_account, const name& owner, const symbol& sym )
{
    token::accounts _accounts( token_contract_account, owner.value );
    const auto itr = _accounts.find( sym.code().raw() );
    if ( itr == _accounts.end() ) return { 0, sym };
    check( itr->balance.symbol == sym, "oracle::get_balance_amount: [sym] does not match");
    return { itr->balance.amount, sym };
}

int64_t oracle::calculate_usd_value( const asset quantity )
{

}

int64_t oracle::convert_usd_to_eos( const int64_t usd )
{
    oracle::tokens_table _tokens( get_self(), get_self().value );

    auto token = _tokens.get( EOS.code().raw(), "oracle::convert_usd_to_eos: [EOS] does not exists");

}

int64_t oracle::get_delphi_price( const name delphi_oracle_id )
{
    delphioracle::pairs _pairs( DELPHI_ORACLE_CONTRACT, DELPHI_ORACLE_CONTRACT.value);
    delphioracle::datapoints _datapoints( DELPHI_ORACLE_CONTRACT, delphi_oracle_id.value);
    const auto pairs = _pairs.get(delphi_oracle_id.value, "oracle::get_delphi_price: [delphi_oracle_id] does not exists");
    const auto datapoints = _datapoints.begin();
    check(datapoints != datapoints.end(), "oracle::get_delphi_price: [delphi_oracle_id] is empty");
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

[[eosio::action]]
void report( const name protocol, const int64_t eos, const int64_t usd, const time_point_sec period )
{
    require_auth( get_self() );

    require_recipient("eosio.yield"_n);
}