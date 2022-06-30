import { Name } from "@greymass/eosio";
import { expectToThrow } from "./helpers";
import { YieldConfig, Protocol } from "./interfaces"
import { contracts } from "./init"
import { category, category1, eos_contracts, evm_contracts, metadata_yield } from "./constants"

// get tables
const getConfig = (): YieldConfig => {
  const scope = Name.from('eosio.yield').value.value;
  return contracts.yield.eosio.tables.config(scope).getTableRows()[0];
}

const getProtocol = ( protocol: string ): Protocol => {
  const scope = Name.from('eosio.yield').value.value;
  const primary_key = Name.from(protocol).value.value;
  return contracts.yield.eosio.tables.protocols(scope).getTableRow(primary_key)
}

const getStatus = ( protocol: string ): string | null => {
  try {
    return getProtocol( protocol ).status;
  } catch (e) {
    return null;
  }
}

describe('eosio.yield', () => {
  it("config::init", async () => {
    await contracts.yield.eosio.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "oracle.yield", "admin.yield"]).send();
    await contracts.yield.eosio.actions.setrate([500, "200000.0000 EOS", "6000000.0000 EOS"]).send();
    const config = getConfig();
    expect(config.annual_rate).toBe(500);
    expect(config.min_tvl_report).toBe("200000.0000 EOS");
    expect(config.max_tvl_report).toBe("6000000.0000 EOS");
  });

  it("regprotocol", async () => {
    await contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata_yield]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata_yield);
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
    expect(getStatus("protocol1")).toEqual(null);
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

  it("setcontracts", async () => {
    const auth = eos_contracts.map(contract => { return { actor: contract, permission: "active"} });
    await contracts.yield.eosio.actions.setcontracts([ "myprotocol", eos_contracts ]).send(auth);
    const protocol = getProtocol("myprotocol");
    expect(protocol.contracts).toEqual(eos_contracts);
  });

  it("setevm", async () => {
    const action = contracts.yield.eosio.actions.setevm([ "myprotocol", evm_contracts ]).send("myprotocol@active");
    await expectToThrow(action, "NOT IMPLEMENTED");
  });

  it("admin::approve", async () => {
    const before = getProtocol("myprotocol");
    expect(before.status).toEqual("pending");

    await contracts.yield.eosio.actions.approve([ "myprotocol" ]).send("admin.yield@active");
    const after = getProtocol("myprotocol");
    expect(after.status).toEqual("active");
  });
});
