void oracle::notify_admin()
{
    const name admin_contract = get_config().admin_contract;
    if ( admin_contract ) require_recipient( admin_contract );
}

[[eosio::on_notify("*::transfer")]]
void oracle::on_transfer( const name from, const name to, const asset quantity, const std::string memo )
{
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::updatelog( const name oracle, const name protocol, const name category, const set<name> contracts, const set<string> evm, const time_point_sec period, const vector<asset> balances, const vector<asset> prices, const asset tvl, const asset usd )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::claimlog( const name oracle, const name category, const name receiver, const asset claimed, const asset balance )

{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::statuslog( const name oracle, const name status )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::createlog( const name oracle, const name type, const map<name, string> metadata )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::eraselog( const name oracle )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::metadatalog( const name oracle, const map<name, string> metadata )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void oracle::rewardslog( const name oracle, const asset rewards, const asset balance )
{
    require_auth( get_self() );
    notify_admin();
}
