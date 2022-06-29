import { Name } from "@greymass/eosio";
import { OracleConfig, Oracle } from './interfaces'
import { contracts } from "./init"

export const getOracleConfig = (): OracleConfig => {
    const scope = Name.from('oracle.yield').value.value;
    return contracts.yield.oracle.tables.config(scope).getTableRows()[0];
}

export const getOracle = ( oracle: string ): Oracle => {
    const scope = Name.from('oracle.yield').value.value;
    const primary_key = Name.from(oracle).value.value;
    return contracts.yield.oracle.tables.oracles(scope).getTableRow(primary_key)
}