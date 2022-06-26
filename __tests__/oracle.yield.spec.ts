import { Name } from "@greymass/eosio";
import { expectToThrow } from "./helpers";
import { contracts } from "./init"
import { OracleConfig, Oracle } from "./interfaces"

// get tables
const getConfig = (): OracleConfig => {
  const scope = Name.from('oracle.yield').value.value;
  return contracts.yield.oracle.tables.config(scope).getTableRows()[0];
}

const getOracle = ( oracle: string ): Oracle => {
  const scope = Name.from('oracle.yield').value.value;
  const primary_key = Name.from(oracle).value.value;
  return contracts.yield.oracle.tables.oracles(scope).getTableRow(primary_key)
}

// defaults
const metadata = [
  {key: "name", value: "My Oracle"},
  {key: "website", value: "https://myoracle.com"}
];

describe('oracle.yield', () => {
  it("config::init", async () => {
    await contracts.yield.oracle.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "eosio.yield", "admin.yield"]).send();
    await contracts.yield.oracle.actions.setreward(["0.0200 EOS"]).send();
    const config = getConfig();
    expect(config.reward_per_update.quantity).toBe("0.0200 EOS");
  });

  it("config::addtoken", async () => {
    await contracts.yield.oracle.actions.addtoken(["USDT", "tethertether", null, null]).send();
    await contracts.yield.oracle.actions.addtoken(["EOS", "eosio.token", 1, "eosusd"]).send();
  });

  it("regoracle", async () => {
    await contracts.yield.oracle.actions.regoracle(["myoracle", metadata]).send('myoracle@active');
    const oracle = getOracle("myoracle");
    expect(oracle.metadata).toEqual(metadata);
    expect(oracle.status).toEqual("pending");
  });

  it("regoracle::approve", async () => {
    await contracts.yield.oracle.actions.approve(["myoracle"]).send('admin.yield@active');
    const oracle = getOracle("myoracle");
    expect(oracle.status).toEqual("active");
  });

  it("regoracle::error::missing required authority", async () => {
    const action = contracts.yield.oracle.actions.regoracle(["myoracle", metadata]).send('foobar@active');
    await expectToThrow(action, "missing required authority");
  });

  it("regoracle::error::invalid metadata_key", async () => {
    const action = contracts.yield.oracle.actions.regoracle(["myoracle", [{"key": "foo", "value": "bar"}]]).send('myoracle@active')
    await expectToThrow(action, "[key=foo] is not valid");
  });

  // it("setmetadata", async () => {
  //   await contracts.yield.oracle.actions.setmetadata(["myprotocol", metadata]).send('myprotocol@active');
  //   const protocol = getProtocol("myprotocol");
  //   expect(protocol.metadata).toEqual(metadata);
  // });

  // it("setmetakey", async () => {
  //   await contracts.yield.oracle.actions.setmetakey(["myprotocol", metadata[0].key, metadata[0].value]).send('myprotocol@active');
  //   const protocol = getProtocol("myprotocol");
  //   expect(protocol.metadata).toEqual(metadata);
  // });

  // it("setcontracts", async () => {
  //   const auth = contracts.map(contract => { return { actor: contract, permission: "active"} });
  //   await contracts.yield.oracle.actions.setcontracts([ "myprotocol", contracts ]).send(auth);
  //   const protocol = getProtocol("myprotocol");
  //   expect(protocol.contracts).toEqual(contracts);
  // });

  // it("setevm", async () => {
  //   const action = contracts.yield.oracle.actions.setevm([ "myprotocol", evms ]).send("myprotocol@active");
  //   await expectToThrow(action, "NOT IMPLEMENTED");
  // });

  // it("admin::approve", async () => {
  //   const before = getProtocol("myprotocol");
  //   expect(before.status).toEqual("pending");

  //   await contracts.yield.oracle.actions.approve([ "myprotocol" ]).send("admin.yield@active");
  //   const after = getProtocol("myprotocol");
  //   expect(after.status).toEqual("active");
  // });
});
