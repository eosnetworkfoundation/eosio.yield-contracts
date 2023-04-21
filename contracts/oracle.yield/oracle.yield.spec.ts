import { Name, Asset } from "@greymass/eosio";
import { expectToThrow } from "@tests/helpers";
import { blockchain, contracts } from "@tests/init";
import { metadata_oracle, RATE, MIN_TVL, MAX_TVL, PERIOD_INTERVAL } from "@tests/constants";
import { OracleConfig, Oracle, Period, Protocol } from '@tests/interfaces';

const getConfig = (): OracleConfig => {
  const scope = Name.from('oracle.yield').value.value;
  return contracts.yield.oracle.tables.config(scope).getTableRows()[0];
}

const getOracle = ( oracle: string ): Oracle => {
  const scope = Name.from('oracle.yield').value.value;
  const primary_key = Name.from(oracle).value.value;
  return contracts.yield.oracle.tables.oracles(scope).getTableRow(primary_key)
}

const getPeriods = ( protocol: string ): Period[] => {
  const scope = Name.from(protocol).value.value;
  const rows = contracts.yield.oracle.tables.periods(scope).getTableRows();
  return rows;
}

const getProtocol = ( protocol: string ): Protocol => {
  const scope = Name.from('eosio.yield').value.value;
  const primary_key = Name.from(protocol).value.value;
  return contracts.yield.eosio.tables.protocols(scope).getTableRow(primary_key)
}

const getBalance = ( account: string, symcode = "EOS" ): number => {
  const contract = (contracts.token as any)[symcode];
  const scope = Name.from(account).value.value;
  const primaryKey = Asset.SymbolCode.from(symcode).value.value;
  const result = contract.tables.accounts(scope).getTableRow(primaryKey);
  if ( result?.balance ) return Asset.from( result.balance ).value;
  return 0;
}

const calculateRewards = (tvl: string) => {
  return Number(BigInt(Asset.from(tvl).units.toNumber()) * BigInt(RATE) / 365n / 24n / 6n / 10000n);
}

beforeAll(async () => {
  // set eosio.yield
  await contracts.yield.eosio.actions.init([{sym: "4,EOS", contract: "eosio.token"}, "oracle.yield", "admin.yield"]).send();
  await contracts.yield.eosio.actions.setrate([RATE, MIN_TVL, MAX_TVL]).send();
  await contracts.yield.eosio.actions.regprotocol(["myprotocol", "dexes", metadata_oracle]).send('myprotocol@active');
  await contracts.yield.eosio.actions.approve([ "myprotocol" ]).send("admin.yield@active");
  await contracts.token.EOS.actions.transfer(["eosio", "eosio.yield", "100000.0000 EOS", "init"]).send("eosio@active");

  // set oracles
  await contracts.oracle.defi.actions.setprice([1, "eosio.token", "4,EOS", 13869]).send();
  await contracts.oracle.delphi.actions.setpair(["eosusd", "4,EOS", "eosio.token", 4]).send();
  await contracts.oracle.delphi.actions.setprice(["eosusd", 13869]).send();
});

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
    await contracts.yield.oracle.actions.regoracle(["myoracle", metadata_oracle]).send('myoracle@active');
    const oracle = getOracle("myoracle");
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
    const protocol = getOracle("myoracle");
    expect(protocol.metadata).toEqual(metadata_oracle);
  });

  it("setmetakey", async () => {
    await contracts.yield.oracle.actions.setmetakey(["myoracle", metadata_oracle[0].key, metadata_oracle[0].value]).send('myoracle@active');
    const oracle = getOracle("myoracle");
    expect(oracle.metadata).toEqual(metadata_oracle);
  });

  it("admin::approve", async () => {
    const before = getOracle("myoracle");
    expect(before.status).toEqual("pending");

    await contracts.yield.oracle.actions.approve([ "myoracle" ]).send("admin.yield@active");
    const after = getOracle("myoracle");
    expect(after.status).toEqual("active");
  });

  it("update", async () => {
    const before = getOracle("myoracle");
    expect(Asset.from(before.balance.quantity).value).toEqual(0.00);

    await contracts.yield.oracle.actions.update(["myoracle", "myprotocol"]).send("myoracle@active");
    const after = getOracle("myoracle");
    expect(Asset.from(after.balance.quantity).value).toEqual(0.02);
  });

  it("oracle.yield::claim", async () => {
    const balance = Asset.from(getOracle("myoracle").balance.quantity).value;
    expect(getBalance("myoracle", "EOS")).toBe(0);
    expect(balance).toBeGreaterThan(0);
    await contracts.yield.oracle.actions.claim(["myoracle", null]).send('myoracle@active');
    expect(Asset.from(getOracle("myoracle").balance.quantity).value).toBe(0);
    expect(getBalance("myoracle", "EOS")).toBe(balance);
  });

  it("updateall", async () => {
    blockchain.addTime(PERIOD_INTERVAL); // push time by 10 minutes
    await contracts.yield.oracle.actions.updateall(["myoracle", 20]).send("myoracle@active");
    const oracle = getOracle("myoracle");
    expect(Asset.from(oracle.balance.quantity).value).toEqual(0.02);
  });

  it("updateall::145 times", async () => {
    let count = 145;
    while (count > 0 ) {
      blockchain.addTime(PERIOD_INTERVAL); // push time by 10 minutes
      await contracts.yield.oracle.actions.updateall(["myoracle", 20]).send("myoracle@active");
      count -= 1;
    }
    expect(getPeriods("myprotocol").length).toEqual(144);
  });

  it("updateall::check protocol balance", async () => {
    const before = getProtocol("myprotocol");
    const balance = Asset.from(before.balance.quantity);
    blockchain.addTime(PERIOD_INTERVAL); // push time by 10 minutes
    await contracts.yield.oracle.actions.updateall(["myoracle", 20]).send("myoracle@active");
    const after = getProtocol("myprotocol");
    const rewards = calculateRewards(before.tvl);
    expect(Asset.from(after.balance.quantity).value * 10000).toEqual(balance.value * 10000 + rewards);
  });

  it("update::overflow checks", async () => {
    // 1B tokens EOS & USDT
    await contracts.token.EOS.actions.transfer(["eosio", "protocol3", "1000000000.0000 EOS", "init"]).send("eosio@active");
    await contracts.token.USDT.actions.transfer(["tethertether", "protocol3", "1000000000.0000 USDT", "init"]).send("tethertether@active");

    // register protocol
    await contracts.yield.eosio.actions.regprotocol(["protocol3", "dexes", metadata_oracle]).send('protocol3@active');
    await contracts.yield.eosio.actions.approve([ "protocol3" ]).send("admin.yield@active");

    // update
    await contracts.yield.oracle.actions.update(["myoracle", "protocol3"]).send("myoracle@active");
    expect(getPeriods("protocol3").length).toEqual(1);
    const protocol = getProtocol("protocol3");
    expect(Asset.from(protocol.balance.quantity).value).toEqual(0);
  });

  it("eosio.yield::claim", async () => {
    const balance = Asset.from(getProtocol("myprotocol").balance.quantity).value;
    expect(getBalance("myprotocol", "EOS")).toBe(0);
    expect(balance).toBeGreaterThan(0);
    await contracts.yield.eosio.actions.claim(["myprotocol", null]).send('myprotocol@active');
    expect(Asset.from(getProtocol("myprotocol").balance.quantity).value).toBe(0);
    expect(getBalance("myprotocol", "EOS")).toBe(balance);
  });

  it("allow claim when not active", async () => {
    blockchain.addTime(PERIOD_INTERVAL); // push time by 10 minutes
    await contracts.yield.oracle.actions.updateall(["myoracle", 20]).send("myoracle@active");
    await contracts.yield.eosio.actions.deny([ "myprotocol" ]).send("admin.yield@active");
    await contracts.yield.eosio.actions.claim(["myprotocol", null]).send('myprotocol@active');
  });

});
