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
  const primaryKey = Name.from(category).value.value;
  return contracts.yield.admin.tables.categories(scope).getTableRow(primaryKey);
}

describe('admin.yield', () => {
  it("config::setmetakey", async () => {
    for ( const row of metakeys ) {
      await contracts.yield.admin.actions.setmetakey(row).send();
      expect(getMetakeys(row.key)).toEqual(row);
    }
  });

  it("config::delmetakey", async () => {
    const { key } = metakeys[0];
    await contracts.yield.admin.actions.delmetakey([key]).send();
    expect(getMetakeys(key)).toEqual(undefined);
    await contracts.yield.admin.actions.setmetakey(metakeys[0]).send();
    expect(getMetakeys(key).key).toEqual(key);
  });

  it("config::setcategory", async () => {
    for ( const row of categories ) {
      await contracts.yield.admin.actions.setcategory(row).send();
      expect(getCategory(row.category)).toEqual(row);
    }
  });

  it("config::delcategory", async () => {
    const { category } = categories[0];
    await contracts.yield.admin.actions.delcategory([category]).send();
    expect(getCategory(category)).toEqual(undefined);
    await contracts.yield.admin.actions.setcategory(categories[0]).send();
    expect(getCategory(category).category).toEqual(category);
  });
});
