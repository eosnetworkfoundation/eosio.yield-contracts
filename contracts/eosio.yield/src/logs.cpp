// @eosio.code
[[eosio::action]]
void yield::rewardslog( const name protocol, const name category, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::claimlog( const name protocol, const name category, const name receiver, const asset claimed )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::statuslog( const name protocol, const name status )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::contractslog( const name protocol, const set<name> contracts, const set<string> evm )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::categorylog( const name protocol, const name category )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::createlog( const name protocol, const name category, const map<name, string> metadata )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::eraselog( const name protocol )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::balancelog( const name protocol, const asset balance )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::metadatalog( const name protocol, const map<name, string> metadata )
{
    require_auth( get_self() );
    notify_admin();
}
