void yield::notify_admin()
{
    const name admin_contract = get_config().admin_contract;
    if ( admin_contract ) require_recipient( admin_contract );
}

[[eosio::on_notify("*::transfer")]]
void yield::on_transfer( const name from, const name to, const asset quantity, const std::string memo )
{
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::rewardslog( const name protocol, const name category, const time_point_sec period, const uint32_t period_interval, const asset tvl, const asset usd, const asset rewards, const asset balance )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::claimlog( const name protocol, const name category, const name receiver, const asset claimed, const asset balance )
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
void yield::contractslog( const name protocol, const name status, const set<name> contracts, const set<string> evm )
{
    require_auth( get_self() );
    notify_admin();
}

// @eosio.code
[[eosio::action]]
void yield::createlog( const name protocol, const name status, const name category, const map<name, string> metadata )
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
void yield::metadatalog( const name protocol, const name status, const name category, const map<name, string> metadata )
{
    require_auth( get_self() );
    notify_admin();
}
