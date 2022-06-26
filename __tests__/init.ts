import { Blockchain } from "@proton/vert"

export const blockchain = new Blockchain()

// contracts
export const contracts = {
  yield: {
    oracle: blockchain.createContract('oracle.yield', 'contracts/oracle.yield/oracle.yield'),
    eosio: blockchain.createContract('eosio.yield', 'contracts/eosio.yield/eosio.yield'),
    admin: blockchain.createContract('admin.yield', 'contracts/admin.yield/admin.yield'),
  },
  oracle: {
    delphi: blockchain.createContract('delphioracle', 'external/delphioracle/delphioracle'),
    defi: blockchain.createContract('oracle.defi', 'external/oracle.defi/oracle.defi'),
  },
  token: {
    eos: blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token'),
    usdt: blockchain.createContract('tethertether', 'external/eosio.token/eosio.token'),
  },
  eosio: {
    system: blockchain.createContract('eosio', 'external/eosio.system/eosio.system'),
  }
}

// defaults
const metakeys = [
  {required: true,  type: "url", key: "website", description: "Protocol website"},
  {required: true,  type: "string", key: "name", description: "Protocol name"},
]

const categories = [
  {category: "dexes", description: "Protocols where you can swap/trade cryptocurrency"},
]

// accounts
export const accounts = blockchain.createAccounts('myprotocol', 'myoracle', 'myvault', "protocol1", "protocol2", "myaccount", "foobar");

// one-time setup
beforeAll(async () => {
  // set ABI hashes for protocols
  const hash = "fead01c2fc2a294e9c3d1adb97511954315518d7f1b7eff4f53a042c20cd27d3";
  await contracts.eosio.system.actions.abihash(["myprotocol", hash]).send();
  await contracts.eosio.system.actions.abihash(["protocol1", hash]).send();
  await contracts.eosio.system.actions.abihash(["protocol2", hash]).send();

  // create EOS token
  await contracts.token.eos.actions.create(["eosio", "1000000000.0000 EOS"]).send();
  await contracts.token.eos.actions.issue(["eosio", "1000000000.0000 EOS", "init"]).send("eosio@active");
  await contracts.token.eos.actions.transfer(["eosio", "oracle.yield", "100000.0000 EOS", "init"]).send("eosio@active");
  await contracts.token.eos.actions.transfer(["eosio", "protocol1", "100000.0000 EOS", "init"]).send("eosio@active");

  // create USDT token
  await contracts.token.usdt.actions.create(["tethertether", "1000000000.0000 USDT"]).send("tethertether@active");
  await contracts.token.usdt.actions.issue(["tethertether", "1000000000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.usdt.actions.transfer(["tethertether", "oracle.yield", "100000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.usdt.actions.transfer(["tethertether", "protocol1", "100000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.usdt.actions.transfer(["tethertether", "protocol2", "200000.0000 USDT", "init"]).send("tethertether@active");

  // set oracles
  await contracts.oracle.defi.actions.setprice([1, "eosio.token", "4,EOS", 13869]).send();
  await contracts.oracle.delphi.actions.setpair(["eosusd", "4,EOS", "eosio.token", 4]).send();
  await contracts.oracle.delphi.actions.setprice(["eosusd", 13869]).send();

  // basic admin setup
  for ( const metakey of metakeys ) {
    await contracts.yield.admin.actions.setmetakey(metakey).send();
  }
  for ( const metakey of categories ) {
    await contracts.yield.admin.actions.setcategory(metakey).send();
  }
});