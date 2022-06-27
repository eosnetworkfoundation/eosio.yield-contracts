// @eosio.code
[[eosio::action]]
void oracle::updatelog( const name oracle, const name protocol, const name category, const set<name> contracts, const set<string> evm, const time_point_sec period, const vector<asset> balances, const vector<asset> prices, const asset tvl, const asset usd )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::claimlog( const name oracle, const extended_asset claimed )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::statuslog( const name oracle, const name status )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::createlog( const name oracle, const map<name, string> metadata )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::eraselog( const name oracle )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::balancelog( const name oracle, const extended_asset balance )
{
    require_auth( get_self() );
}

// @eosio.code
[[eosio::action]]
void oracle::metadatalog( const name oracle, const map<name, string> metadata )
{
    require_auth( get_self() );
}
