export interface KV {
    key: string;
    value: string;
}

export interface ExtendedAsset {
    quantity: string;
    contract: string;
}

export interface ExtendedSymbol {
    sym: string;
    contract: string;
}

export interface Protocol {
    protocol: string; //  "myprotocol"
    status: string; //  "active"
    contracts: string[]; //  ["myprotocol", "mytreasury"]
    evm: string[]; //  ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
    tvl: string; //  "200000.0000 EOS"
    usd: string; //  "300000.0000 USD"
    balance: ExtendedAsset; //  {"quantity": "2.5000 EOS", "contract": "eosio.token"}
    created_at: string; //  "2022-05-13T00:00:00"
    updated_at: string; //  "2022-05-13T00:00:00"
    claimed_at: string; //  "1970-01-01T00:00:00"
    period_at: string; //  "1970-01-01T00:00:00"
    metadata: KV[]; //  [{"key": "url", "value": "https://myprotocol.com"}]
}

export interface Config {
    annual_rate: number; // 500
    min_tvl_report: string; // "200000.0000 EOS"
    max_tvl_report: string; // "6000000.0000 EOS"
    rewards: ExtendedSymbol; // { "sym": "4,EOS", "contract": "eosio.token" }
    oracle_contract: string; // "oracle.yield"
    admin_contract: string; // "admin.yield"
}