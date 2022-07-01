import { Name } from "@greymass/eosio";
import { OracleConfig, Oracle, Period } from './interfaces'
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

export const getPeriods = ( protocol: string ): Period[] => {
    const scope = Name.from(protocol).value.value;
    const rows = contracts.yield.oracle.tables.periods(scope).getTableRows();
    return rows;
}