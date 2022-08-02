import { Blockchain } from "@proton/vert"
import { TimePointSec } from "@greymass/eosio";
import { categories, metakeys } from "./constants"

export const blockchain = new Blockchain()

// contracts
export const contracts = {
  yield: {
    oracle: blockchain.createContract('oracle.yield', 'contracts/oracle.yield/oracle.yield', true),
    eosio: blockchain.createContract('eosio.yield', 'contracts/eosio.yield/eosio.yield', true),
    admin: blockchain.createContract('admin.yield', 'contracts/admin.yield/admin.yield', true),
  },
  oracle: {
    delphi: blockchain.createContract('delphioracle', 'external/delphioracle/delphioracle'),
    defi: blockchain.createContract('oracle.defi', 'external/oracle.defi/oracle.defi'),
  },
  token: {
    EOS: blockchain.createContract('eosio.token', 'external/eosio.token/eosio.token'),
    USDT: blockchain.createContract('tethertether', 'external/eosio.token/eosio.token'),
  },
}

// accounts
export const accounts = blockchain.createAccounts('eosio', 'myprotocol', 'myoracle', 'myvault', "protocol1", "protocol2", "protocol3", "myaccount", "foobar");

// one-time setup
beforeAll(async () => {
  blockchain.setTime(TimePointSec.from(new Date()));

  // create EOS token
  await contracts.token.EOS.actions.create(["eosio", "10000000000.0000 EOS"]).send();
  await contracts.token.EOS.actions.issue(["eosio", "10000000000.0000 EOS", "init"]).send("eosio@active");
  await contracts.token.EOS.actions.transfer(["eosio", "oracle.yield", "100000.0000 EOS", "init"]).send("eosio@active");
  await contracts.token.EOS.actions.transfer(["eosio", "protocol1", "100000.0000 EOS", "init"]).send("eosio@active");

  // create USDT token
  await contracts.token.USDT.actions.create(["tethertether", "10000000000.0000 USDT"]).send("tethertether@active");
  await contracts.token.USDT.actions.issue(["tethertether", "10000000000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.USDT.actions.transfer(["tethertether", "oracle.yield", "100000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.USDT.actions.transfer(["tethertether", "myprotocol", "500000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.USDT.actions.transfer(["tethertether", "protocol1", "100000.0000 USDT", "init"]).send("tethertether@active");
  await contracts.token.USDT.actions.transfer(["tethertether", "protocol2", "200000.0000 USDT", "init"]).send("tethertether@active");

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