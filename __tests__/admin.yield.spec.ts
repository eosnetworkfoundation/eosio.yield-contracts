import { Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { expectToThrow } from "./helpers";

export interface Metakey {
  key: string; //  "category"
  required: boolean; // true
  description: string; // "Protocol category"
}

export interface Category {
  category: string; //  "dexes"
  description: string; // "Protocols where you can swap/trade cryptocurrency"
}

/**
 * Initialize
 */
const blockchain = new Blockchain()
const adminYield = blockchain.createContract('admin.yield', 'contracts/admin.yield/admin.yield');

/**
 * Helpers
 */
const getMetakeys = (key: string): Metakey => {
  const scope =Name.from('admin.yield').value.value;
  return adminYield.tables.metakeys(scope).getTableRow( Name.from(key).value.value );
}

const getCategory = (category: string): Category => {
  const scope = Name.from('admin.yield').value.value;
  return adminYield.tables.categories(scope).getTableRow( Name.from(category).value.value );
}

describe('admin.yield', () => {

  it("config::setmetakey", async () => {
    const metakeys = [
      {required: true,  type: "url", key: "website", description: "Protocol website"},
      {required: true,  type: "string", key: "name", description: "Protocol name"},
      {required: false, type: "string", key: "dappradar", description: "DappRadar identifier"},
    ]
    for ( const metakey of metakeys ) {
      await adminYield.actions.setmetakey(metakey).send();
      expect(getMetakeys(metakey.key)).toEqual(metakey);
    }
  });

  it("config::setcategory", async () => {
    const categories = [
      {category: "dexes", description: "Protocols where you can swap/trade cryptocurrency"},
    ]
    for ( const metakey of categories ) {
      await adminYield.actions.setcategory(metakey).send();
      expect(getCategory(metakey.category)).toEqual(metakey);
    }
  });
});
