import { Name } from "@greymass/eosio";
import { expectToThrow } from "@tests/helpers";
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
