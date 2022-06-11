import { Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { expectToThrow } from "./helpers";
// import { Config, Protocol } from "./interfaces"

/**
 * Initialize
 */
const blockchain = new Blockchain()
// const adminYield = blockchain.createContract('admin.yield', 'contracts/admin.yield/admin.yield');

// /**
//  * Helpers
//  */
//  const getConfig = (): Config => {
//   const scope = Name.from('admin.yield').value.value;
//   return adminYield.tables.config(scope).getTableRows()[0];
// }

describe('admin.yield', () => {

  it("config::setmetakeys", async () => {
    // const metakeys = ["description", "name", "url"];
    // await adminYield.actions.setmetakeys([metakeys]).send();
    // const config = getConfig();
    // expect(config.metadata_keys).toEqual(metakeys);
    expect(true).toBeTruthy
  });
});
