import { Asset, Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { wait } from "./helpers";

/**
 * Initialize
 */
const blockchain = new Blockchain()
const eosioYield = blockchain.createContract('eosio.yield', 'contracts/eosio.yield/eosio.yield');
const [ myprotocol, myvault, protocol1, protocol2 ] = blockchain.createAccounts('myprotocol', 'myvault', "protocol1", "protocol2");

/**
 * Helpers
 */
const getConfig = () => {
  const scope = Name.from('eosio.yield').value.value;
  return eosioYield.tables.config(scope).getTableRows()[0];
}

const getProtocol = ( protocol: string ) => {
  const scope = Name.from('eosio.yield').value.value;
  const primary_key = Name.from(protocol).value.value;
  return eosioYield.tables.protocols(scope).getTableRow(primary_key)
}


// one-time setup
beforeAll(async () => {
  const eosioSystem = blockchain.createContract('eosio', 'external/eosio.system/eosio.system');
  await wait(0);

  // set ABI hashes for protocols
  await eosioSystem.actions.abihash(["myprotocol", "fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3"]).send('eosio@active');
  await eosioSystem.actions.abihash(["protocol1", "fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3"]).send('eosio@active');
  await eosioSystem.actions.abihash(["protocol2", "fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3"]).send('eosio@active');
});

test("config::setrate", async () => {
  await eosioYield.actions.setrate([500, "200000.0000 EOS", "6000000.0000 EOS"]).send();
  const config = getConfig();
  expect(config.annual_rate).toBe(500);
  expect(config.min_tvl_report).toBe("200000.0000 EOS");
  expect(config.max_tvl_report).toBe("6000000.0000 EOS");
});

test("config::setmetakeys", async () => {
  const metakeys = ["description", "name", "url"];
  await eosioYield.actions.setmetakeys([metakeys]).send();
  const config = getConfig();
  expect(config.metadata_keys).toEqual(metakeys);
});

test("regprotocol", async () => {
  const metadata = [{"key": "url", "value": "https://myprotocol.com"}];
  await eosioYield.actions.regprotocol(["myprotocol", metadata]).send('myprotocol@active');
  const protocol = getProtocol("myprotocol");
  expect(protocol.metadata).toEqual(metadata);
});
