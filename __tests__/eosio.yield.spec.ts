import { Name } from "@greymass/eosio";
import { expectToThrow } from "./helpers";
import { YieldConfig, Protocol } from "./interfaces"
import { contracts } from "./init"

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

// defaults
const category = "dexes";
const eos_contracts = ["myprotocol", "myvault"];
const evms = ["0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0"];
const metadata = [
  {key: "name", value: "My Protocol"},
  {key: "website", value: "https://myprotocol.com"}
];

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
    await contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata);
  });

  it("regprotocol::error::missing required authority", async () => {
    const action = contracts.yield.eosio.actions.regprotocol(["myprotocol", category, metadata]).send('myaccount@active')
    await expectToThrow(action, "missing required authority");
  });

  it("regprotocol::error::invalid metadata_key", async () => {
    const action = contracts.yield.eosio.actions.regprotocol(["protocol1", category, [{"key": "foo", "value": "bar"}]]).send('protocol1@active')
    await expectToThrow(action, "[key=foo] is not valid");
  });

  it("setmetadata", async () => {
    await contracts.yield.eosio.actions.setmetadata(["myprotocol", metadata]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata);
  });

  it("setmetakey", async () => {
    await contracts.yield.eosio.actions.setmetakey(["myprotocol", metadata[0].key, metadata[0].value]).send('myprotocol@active');
    const protocol = getProtocol("myprotocol");
    expect(protocol.metadata).toEqual(metadata);
  });

  it("setcontracts", async () => {
    const auth = eos_contracts.map(contract => { return { actor: contract, permission: "active"} });
    await contracts.yield.eosio.actions.setcontracts([ "myprotocol", eos_contracts ]).send(auth);
    const protocol = getProtocol("myprotocol");
    expect(protocol.contracts).toEqual(eos_contracts);
  });

  it("setevm", async () => {
    const action = contracts.yield.eosio.actions.setevm([ "myprotocol", evms ]).send("myprotocol@active");
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
