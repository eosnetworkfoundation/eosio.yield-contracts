import { Name } from "@greymass/eosio";
import { expectToThrow, mapToObject } from "@tests/helpers";
import { YieldConfig, Protocol } from "@tests/interfaces"
import { contracts } from "@tests/init"
import { category, category1, eos_contracts, evm_contracts, metadata_yield, RATE, MIN_TVL, MAX_TVL, PERIOD_INTERVAL } from "@tests/constants"

// get tables
const getConfig = (): YieldConfig => {
  const scope = Name.from('eosio.yield').value.value;
  return contracts.yield.eosio.tables.config(scope).getTableRows()[0];
}

const getProtocol = ( protocol: string ): Protocol => {
  const scope = Name.from('eosio.yield').value.value;
  const primaryKey = Name.from(protocol).value.value;
  return contracts.yield.eosio.tables.protocols(scope).getTableRow(primaryKey);
}

const getStatus = ( protocol: string ): string => {
  return getProtocol( protocol )?.status;
}

describe('eosio.yield', () => {

  it("error::regprotocol before init", async () => {
    const action = contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata_yield]).send('myprotocol@active');
    await expectToThrow(action, "is not initialized");
  });

  it("config::init", async () => {
    const EOS = {sym: "4,EOS", contract: "eosio.token"};
    await contracts.yield.eosio.actions.init([EOS, "oracle.yield", "admin.yield"]).send();
    const config = getConfig();
    expect(config.admin_contract).toBe("admin.yield");
    expect(config.oracle_contract).toBe("oracle.yield");
    expect(config.rewards).toEqual(EOS);
  });

  it("config::setrate", async () => {
    await contracts.yield.eosio.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "oracle.yield", "admin.yield"]).send();
    await contracts.yield.eosio.actions.setrate([RATE, MIN_TVL, MAX_TVL]).send();
    const config = getConfig();
    expect(config.annual_rate).toBe(RATE);
    expect(config.min_tvl_report).toBe(MIN_TVL);
    expect(config.max_tvl_report).toBe(MAX_TVL);
  });

  it("regprotocol", async () => {
    await contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata_yield]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata_yield);
    expect(protocol.status).toEqual("pending");
  });

  it("regprotocol::error::missing required authority", async () => {
    const action = contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata_yield]).send('myaccount@active')
    await expectToThrow(action, "missing required authority");
  });

  it("regprotocol::error::invalid metadata_key", async () => {
    const action = contracts.yield.eosio.actions.regprotocol(["protocol1", category, [{"key": "foo", "value": "bar"}]]).send('protocol1@active')
    await expectToThrow(action, "[key=foo] is not valid");
  });

  it("setmetadata", async () => {
    await contracts.yield.eosio.actions.setmetadata(["myprotocol", metadata_yield]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata_yield);
  });

  it("regprotocol::approve/deny/register/unregister", async () => {
    // register
    await contracts.yield.eosio.actions.regprotocol(["protocol1", category, metadata_yield]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual("pending");

    // approve
    await contracts.yield.eosio.actions.approve(["protocol1"]).send('admin.yield@active');
    expect(getStatus("protocol1")).toEqual("active");

    // deny
    await contracts.yield.eosio.actions.deny(["protocol1"]).send('admin.yield@active');
    expect(getStatus("protocol1")).toEqual("denied");

    // approve after denied
    await contracts.yield.eosio.actions.regprotocol(["protocol1", category, metadata_yield]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual("pending");

    // unregister
    await contracts.yield.eosio.actions.unregister(["protocol1"]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual(undefined);
  });

  it("regprotocol::deny/setmetadata/setmetakey/setcategory", async () => {
    // register
    await contracts.yield.eosio.actions.regprotocol(["protocol1", category, metadata_yield]).send('protocol1@active');
    await contracts.yield.eosio.actions.deny(["protocol1"]).send('admin.yield@active');
    expect(getStatus("protocol1")).toEqual("denied");

    // update metadata
    await contracts.yield.eosio.actions.setmetadata(["protocol1", metadata_yield]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual("pending");
    await contracts.yield.eosio.actions.deny(["protocol1"]).send('admin.yield@active');

    // update metakey
    await contracts.yield.eosio.actions.setmetakey(["protocol1", metadata_yield[0].key, metadata_yield[0].value]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual("pending");
    await contracts.yield.eosio.actions.deny(["protocol1"]).send('admin.yield@active');

    // update category
    await contracts.yield.eosio.actions.setcategory(["protocol1", category1]).send('protocol1@active');
    expect(getStatus("protocol1")).toEqual("pending");
  });

  it("setmetakey", async () => {
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", metadata_yield[0].key, metadata_yield[0].value]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata_yield);
  });

  it("setmetakey:: add token", async () => {
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", "token.code", "eosio.token"]).send('myprotocol@active');
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", "token.symcode", "EOS"]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(mapToObject(protocol.metadata)["token.code"]).toEqual("eosio.token");
    expect(mapToObject(protocol.metadata)["token.symcode"]).toEqual("EOS");

    // TO-DO: Vert requires fix to support inline action error throwing
    // const action = contracts.yield.eosio.actions.setmetakey(["myprotocol", "token.code", "FOO"]).send('myprotocol@active');
    // await expectToThrow(action, "invalid supply symbol code");
  });

  it("setmetakey:: Recover+ as integer", async () => {
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", "recover", 123]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(mapToObject(protocol.metadata).recover).toEqual("123");

    // TO-DO: Vert requires fix to support inline action error throwing
    // const action = contracts.yield.eosio.actions.setmetakey(["myprotocol", "recover", "not a number"]).send("myprotocol", );
    // await expectToThrow(action, "invalid integer value");
  });

  it("setcontracts", async () => {
    const auth = eos_contracts.map(contract => { return { actor: contract, permission: "active"} });
    await contracts.yield.eosio.actions.setcontracts([ "myprotocol", eos_contracts, [] ]).send(auth);
    const protocol = getProtocol("myprotocol");
    expect(protocol.contracts).toEqual(eos_contracts);

    const action = contracts.yield.eosio.actions.setcontracts([ "not.exists", eos_contracts, [] ]).send();
    await expectToThrow(action, "does not exists");
  });

  it("admin::approve", async () => {
    const before = getProtocol("myprotocol");
    expect(before.status).toEqual("pending");

    await contracts.yield.eosio.actions.approve([ "myprotocol" ]).send("admin.yield@active");
    const after = getProtocol("myprotocol");
    expect(after.status).toEqual("active");
  });

  it("setcontracts - protocol not included by default", async () => {
    await contracts.yield.eosio.actions.setcontracts([ "myprotocol", ["vault"], [] ]).send();
    const protocol = getProtocol("myprotocol");
    expect(protocol.contracts).toEqual(["vault"]);
  });

  it("setmetakey - valid IPFS", async () => {
    const ipfs = "QmSPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk"
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", "logo", ipfs]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    for ( const row of protocol.metadata ) {
      if ( row.key == "logo") expect(row.value).toEqual(ipfs);
    }
  });

  it("setmetakey - invalid IPFS", async () => {
    const action1 = contracts.yield.eosio.actions.setmetakey(["myprotocol", "logo", "invalid"]).send('myprotocol@active');
    await expectToThrow(action1, "invalid IPFS value");

    const action2 = contracts.yield.eosio.actions.setmetakey(["myprotocol", "logo", "QmSPLWbp...fns9LXoUjdk"]).send('myprotocol@active');
    await expectToThrow(action2, "invalid IPFS value");

    const action3 = contracts.yield.eosio.actions.setmetakey(["myprotocol", "logo", "SPLWbpUttHQqd4gPnPKBGE6XWy6PricPgfns9LXoUjdk"]).send('myprotocol@active');
    await expectToThrow(action3, "invalid IPFS value");
  });
});
