import { Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { expectToThrow } from "./helpers";
import { KV, ExtendedAsset, ExtendedSymbol } from "./interfaces"

/**
 * Initialize
 */
const blockchain = new Blockchain()
const eosioYield = blockchain.createContract('eosio.yield', 'contracts/eosio.yield/eosio.yield', true);
const eosioSystem = blockchain.createContract('eosio', 'external/eosio.system/eosio.system');
const eosioToken = blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token');
blockchain.createAccounts('myprotocol', 'myvault', "protocol1", "protocol2", "myaccount", "oracle.yield", "admin.yield");

export interface Protocol {
  protocol: string; //  "myprotocol"
  category: string; //  "dexes"
  status: string; //  "active"
  contracts: string[]; //  ["myprotocol", "mytreasury"]
  evm: string[]; //  ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"]
  tvl: string; //  "200000.0000 EOS"
  usd: string; //  "300000.0000 USD"
  balance: ExtendedAsset; //  {"quantity": "2.5000 EOS", "contract": "eosio.token"}
  metadata: KV[]; //  [{"key": "website", "value": "https://myprotocol.com"}]
  created_at: string; //  "2022-05-13T00:00:00"
  updated_at: string; //  "2022-05-13T00:00:00"
  claimed_at: string; //  "1970-01-01T00:00:00"
  period_at: string; //  "1970-01-01T00:00:00"
}

export interface Config {
  annual_rate: number; // 500
  min_tvl_report: string; // "200000.0000 EOS"
  max_tvl_report: string; // "6000000.0000 EOS"
  rewards: ExtendedSymbol; // { "sym": "4,EOS", "contract": "eosio.token" }
  oracle_contract: string; // "oracle.yield"
  admin_contract: string; // "admin.yield"
}

/**
 * Helpers
 */
const getConfig = (): Config => {
  const scope = Name.from('eosio.yield').value.value;
  return eosioYield.tables.config(scope).getTableRows()[0];
}

const getProtocol = ( protocol: string ): Protocol => {
  const scope = Name.from('eosio.yield').value.value;
  const primary_key = Name.from(protocol).value.value;
  return eosioYield.tables.protocols(scope).getTableRow(primary_key)
}

// one-time setup
beforeAll(async () => {
  // set ABI hashes for protocols
  const hash = "fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3";
  await eosioSystem.actions.abihash(["myprotocol", hash]).send();
  await eosioSystem.actions.abihash(["protocol1", hash]).send();
  await eosioSystem.actions.abihash(["protocol2", hash]).send();

  // setcode
  // await eosioYield.actions.

  // create token
  await eosioToken.actions.create(["eosio", "1000000000.0000 EOS"]).send();
  await eosioToken.actions.issue(["eosio", "1000000000.0000 EOS", "init"]).send("eosio@active");
});

describe('eosio.yield', () => {
  it("config::init", async () => {
    await eosioYield.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "oracle.yield", "admin.yield"]).send();
    await eosioYield.actions.setrate([500, "200000.0000 EOS", "6000000.0000 EOS"]).send();
    const config = getConfig();
    expect(config.annual_rate).toBe(500);
    expect(config.min_tvl_report).toBe("200000.0000 EOS");
    expect(config.max_tvl_report).toBe("6000000.0000 EOS");
  });

  // Register Protocol
  const category = "dexes";
  const metadata = [
    {"key": "name", "value": "My Protocol"},
    {"key": "website", "value": "https://myprotocol.com"}
  ];
  it("regprotocol::success", async () => {
    await eosioYield.actions.regprotocol(["myprotocol", category, metadata]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata);
  });

  it("regprotocol::error::missing required authority", async () => {
    const action = eosioYield.actions.regprotocol(["myprotocol", category, metadata]).send('myaccount@active')
    await expectToThrow(action, "missing required authority");
  });

  it("regprotocol::error::invalid metadata_key", async () => {
    const action = eosioYield.actions.regprotocol(["protocol1", category, [{"key": "foo", "value": "bar"}]]).send('protocol1@active')
    await expectToThrow(action, "invalid [metadata_key=foo]");
  });

  // Set Contracts
  const contracts = ["myprotocol", "myvault"];
  it("setcontracts::multi-authority", async () => {
    const auth = contracts.map(contract => { return { actor: contract, permission: "active"} });
    await eosioYield.actions.setcontracts([ "myprotocol", contracts ]).send(auth);
    const protocol = getProtocol("myprotocol");
    expect(protocol.contracts).toEqual(contracts);
  });

  it("admin::approve", async () => {
    const before = getProtocol("myprotocol");
    expect(before.status).toEqual("pending");

    await eosioYield.actions.approve([ "myprotocol" ]).send("admin.yield@active");
    const after = getProtocol("myprotocol");
    expect(after.status).toEqual("active");
  });
});
