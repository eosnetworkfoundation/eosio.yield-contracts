[[eosio::on_notify("*::metadatalog")]]
void admin::on_metadatalog( const name protocol, const name status, const name category, const map<name, string> metadata )
{
    check_metadata_keys( category, metadata );
}

[[eosio::on_notify("*::claimlog")]]
void admin::on_claimlog( const name protocol, const name category, const name receiver, const asset claimed, const asset balance )
{
    // no checks
}

[[eosio::on_notify("*::createlog")]]
void admin::on_createlog( const name protocol, const name status, const name category, const map<name, string> metadata )
{
    check_category( category );
    check_metadata_keys( category, metadata );
}