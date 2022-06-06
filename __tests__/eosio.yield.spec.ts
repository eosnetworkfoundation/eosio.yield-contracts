import { Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { expectToThrow } from "./helpers";
import { Config, Protocol } from "./interfaces"

/**
 * Initialize
 */
const blockchain = new Blockchain()
const eosioYield = blockchain.createContract('eosio.yield', 'contracts/eosio.yield/eosio.yield');
const eosioSystem = blockchain.createContract('eosio', 'external/eosio.system/eosio.system');
const [ myprotocol, myvault, protocol1, protocol2, myaccount ] = blockchain.createAccounts('myprotocol', 'myvault', "protocol1", "protocol2", "myaccount");

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
});

describe('eosio.yield', () => {
  // Config
  it("config::setrate", async () => {
    await eosioYield.actions.setrate([500, "200000.0000 EOS", "6000000.0000 EOS"]).send();
    const config = getConfig();
    expect(config.annual_rate).toBe(500);
    expect(config.min_tvl_report).toBe("200000.0000 EOS");
    expect(config.max_tvl_report).toBe("6000000.0000 EOS");
  });

  it("config::setmetakeys", async () => {
    const metakeys = ["description", "name", "url"];
    await eosioYield.actions.setmetakeys([metakeys]).send();
    const config = getConfig();
    expect(config.metadata_keys).toEqual(metakeys);
  });

  // Register Protocol
  const metadata = [{"key": "url", "value": "https://myprotocol.com"}];
  it("regprotocol::success", async () => {
    await eosioYield.actions.regprotocol(["myprotocol", metadata]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata);
  });

  it("regprotocol::error::missing required authority", async () => {
    const action = eosioYield.actions.regprotocol(["myprotocol", metadata]).send('myaccount@active')
    await expectToThrow(action, "missing required authority");
  });

  it("regprotocol::error::invalid metadata_key", async () => {
    const action = eosioYield.actions.regprotocol(["protocol1", [{"key": "foo", "value": "bar"}]]).send('protocol1@active')
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

    await eosioYield.actions.approve([ "myprotocol" ]).send();
    const after = getProtocol("myprotocol");
    expect(after.status).toEqual("active");
  });
});
