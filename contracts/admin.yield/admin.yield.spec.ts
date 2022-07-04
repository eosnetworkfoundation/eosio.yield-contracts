import { Name } from "@greymass/eosio";
import { Metakey, Category } from "@tests/interfaces";
import { contracts } from "@tests/init";
import { metakeys, categories } from "@tests/constants"

const getMetakeys = (key: string): Metakey => {
  const scope = Name.from('admin.yield').value.value;
  return contracts.yield.admin.tables.metakeys(scope).getTableRow( Name.from(key).value.value );
}

const getCategory = (category: string): Category => {
  const scope = Name.from('admin.yield').value.value;
  return contracts.yield.admin.tables.categories(scope).getTableRow( Name.from(category).value.value );
}

describe('admin.yield', () => {
  it("config::setmetakey", async () => {
    for ( const metakey of metakeys ) {
      await contracts.yield.admin.actions.setmetakey(metakey).send();
      expect(getMetakeys(metakey.key)).toEqual(metakey);
    }
  });

  it("config::setcategory", async () => {
    for ( const metakey of categories ) {
      await contracts.yield.admin.actions.setcategory(metakey).send();
      expect(getCategory(metakey.category)).toEqual(metakey);
    }
  });
});
