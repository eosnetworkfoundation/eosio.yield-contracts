import { Asset, TimePointSec } from "@greymass/eosio";
import { expectToThrow } from "./helpers";
import { blockchain, contracts } from "./init"
import { metadata_oracle } from "./constants"
import * as getters from "./getters"

beforeAll(async () => {
  // set eosio.yield
  await contracts.yield.eosio.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "oracle.yield", "admin.yield"]).send();
  await contracts.yield.eosio.actions.setrate([500, "200000.0000 EOS", "6000000.0000 EOS"]).send();
  await contracts.yield.eosio.actions.regprotocol(["myprotocol", "dexes", metadata_oracle]).send('myprotocol@active');
  await contracts.yield.eosio.actions.approve([ "myprotocol" ]).send("admin.yield@active");

  // set oracles
  await contracts.oracle.defi.actions.setprice([1, "eosio.token", "4,EOS", 13869]).send();
  await contracts.oracle.delphi.actions.setpair(["eosusd", "4,EOS", "eosio.token", 4]).send();
  await contracts.oracle.delphi.actions.setprice(["eosusd", 13869]).send();
});

describe('oracle.yield', () => {
  it("config::init", async () => {
    await contracts.yield.oracle.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "eosio.yield", "admin.yield"]).send();
    await contracts.yield.oracle.actions.setreward(["0.0200 EOS"]).send();
    const config = getters.getOracleConfig();
    expect(config.reward_per_update.quantity).toBe("0.0200 EOS");
  });

  it("config::addtoken", async () => {
    await contracts.yield.oracle.actions.addtoken(["USDT", "tethertether", null, null]).send();
    await contracts.yield.oracle.actions.addtoken(["EOS", "eosio.token", 1, "eosusd"]).send();
  });

  it("regoracle", async () => {
    await contracts.yield.oracle.actions.regoracle(["myoracle", metadata_oracle]).send('myoracle@active');
    const oracle = getters.getOracle("myoracle");
    expect(oracle.metadata).toEqual(metadata_oracle);
    expect(oracle.status).toEqual("pending");
  });

  it("regoracle::error::missing required authority", async () => {
    const action = contracts.yield.oracle.actions.regoracle(["myoracle", metadata_oracle]).send('foobar@active');
    await expectToThrow(action, "missing required authority");
  });

  it("regoracle::error::invalid metadata_key", async () => {
    const action = contracts.yield.oracle.actions.regoracle(["myoracle", [{"key": "foo", "value": "bar"}]]).send('myoracle@active')
    await expectToThrow(action, "[key=foo] is not valid");
  });

  it("setmetadata", async () => {
    await contracts.yield.oracle.actions.setmetadata(["myoracle", metadata_oracle]).send('myoracle@active');
    const protocol = getters.getOracle("myoracle");
    expect(protocol.metadata).toEqual(metadata_oracle);
  });

  it("setmetakey", async () => {
    await contracts.yield.oracle.actions.setmetakey(["myoracle", metadata_oracle[0].key, metadata_oracle[0].value]).send('myoracle@active');
    const oracle = getters.getOracle("myoracle");
    expect(oracle.metadata).toEqual(metadata_oracle);
  });

  it("admin::approve", async () => {
    const before = getters.getOracle("myoracle");
    expect(before.status).toEqual("pending");

    await contracts.yield.oracle.actions.approve([ "myoracle" ]).send("admin.yield@active");
    const after = getters.getOracle("myoracle");
    expect(after.status).toEqual("active");
  });

  it("update", async () => {
    const before = getters.getOracle("myoracle");
    expect(Asset.from(before.balance.quantity).value).toEqual(0.00);

    await contracts.yield.oracle.actions.update(["myoracle", "myprotocol"]).send("myoracle@active");
    const after = getters.getOracle("myoracle");
    expect(Asset.from(after.balance.quantity).value).toEqual(0.02);
  });

  it("claim", async () => {
    blockchain.addTime(TimePointSec.from(86400));
    await contracts.yield.oracle.actions.claim(["myoracle"]).send("myoracle@active");
    const oracle = getters.getOracle("myoracle");
    console.log(oracle)
    expect(Asset.from(oracle.balance.quantity).value).toEqual(0.00);
  });

  // it("updateall", async () => {
  //   await contracts.yield.oracle.actions.updateall(["myoracle", 20]).send("myoracle@active");
  //   const oracle = getters.getOracle("myoracle");
  //   expect(Asset.from(oracle.balance.quantity).value).toEqual(0.02);
  // });
});
