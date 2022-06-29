[[eosio::on_notify("*::metadatalog")]]
void admin::on_metadatalog( const name protocol, const map<name, string> metadata )
{
    check_metadata_keys( metadata );
}

[[eosio::on_notify("*::categorylog")]]
void admin::on_categorylog( const name protocol, const name category )
{
    check_category( category );
}

[[eosio::on_notify("*::claimlog")]]
void admin::on_claimlog( const name protocol, const name category, const name receiver, const asset claimed )
{
    // no checks
}

[[eosio::action]]
void admin::on_createlog( const name protocol, const name category, const map<name, string> metadata )
{
    check_category( category );
    check_metadata_keys( metadata );
}