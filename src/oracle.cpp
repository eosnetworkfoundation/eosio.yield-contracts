

//dividing a by b, expressed in output_symbol units
asset divide_assets(asset a, asset b, symbol output_symbol) {

    check(a.symbol.precision() == b.symbol.precision(), "can only multiply assets of same precision");

    double d_a_quantity = (double)(a.amount);
    double d_b_quantity = (double)(b.amount);

    double d_multiplier = pow(10, (double)a.symbol.precision());

    double n_a_quantity = d_a_quantity / d_multiplier;
    double n_b_quantity = d_b_quantity / d_multiplier;

    uint64_t i_result = uint64_t(((d_a_quantity / d_b_quantity) * d_multiplier)+0.5);

    return asset(i_result, output_symbol);

}

// return amount of EOS that would be returned for a REX sale
// derived from https://github.com/EOSIO/eosio.contracts/blob/master/contracts/eosio.system/src/rex.cpp#L772
asset oracleyield::get_rex_in_eos( const asset& rex_quantity ) {

    auto rexitr = _rexpooltable.begin();
    int64_t S0 = rexitr->total_lendable.amount;
    int64_t R0 = rexitr->total_rex.amount;

    // print("rexitr->total_lendable.amount: ", rexitr->total_lendable.amount, "\n");
    // print("rexitr->total_rex.amount: ", rexitr->total_rex.amount, "\n");

    int64_t p  = (uint128_t(rex_quantity.amount) * S0) / R0;

    asset proceeds( p, SYSTEM_TOKEN_SYMBOL );

    return proceeds;
}

//Get EOS/USD rate
asset oracleyield::get_oracle_rate() {

    delphipoints dtp_table(ORACLE_CONTRACT, EOS_USD.value);

    auto dtp_idx = dtp_table.get_index<"timestamp"_n>();

    check(dtp_idx.rbegin() != dtp_idx.rend(), "no oracle datapoint available");

    //print("dtp_idx.begin()->median ", dtp_idx.begin()->median, "\n");
    //print("dtp_idx.begin()->owner ", dtp_idx.begin()->owner, "\n");
    //print("dtp_idx.begin()->timestamp ", dtp_idx.begin()->timestamp.sec_since_epoch(), "\n");

    auto itr = dtp_idx.rbegin();

    return asset(itr->median, USD_SYMBOL);

}

//fetch an account balance on a standard eosio.token contract. Returns 0 if no balance found.
asset oracleyield::get_contract_balance(name account, std::pair<name, symbol> token){

    //print("get_contract_balance\n");

    auto n = token.first;
    auto s = token.second;

    //print("n ", n, "\n");
    //print("s ", s, "\n");

    accounts a_table(n, account.value);

    auto itr = a_table.find(s.code().raw());

    if (itr == a_table.end()) {

        //print("not found \n");

        return asset(0, token.second);

    }
    else {

        //print("itr->balance ", itr->balance, "\n");

        return itr->balance;

    }

}

//updates the list of global snapshots, and deletes any snapshots older than ONE_DAY
void oracleyield::update_global_snapshots(snapshot report){

    //print("update_global_snapshots\n");

    uint64_t c_ts = current_time_point().sec_since_epoch();
    uint64_t older_than_one_day = c_ts - ONE_DAY;

    auto itr = _snapshots.begin();
    auto end_itr = _snapshots.upper_bound(older_than_one_day);

    //erase old snapshots
    while (itr != end_itr){

        //print("erasing old snapshots : ", itr->timestamp.sec_since_epoch(), "\n");

        _snapshots.erase(itr++);

    }

    _snapshots.emplace( get_self(), [&]( auto& p ) {p = report;});

}

//attempt to record all values for a given snapshot
//TODO : benchmark and calculate limits. Possibly need refactor to be batchable
ACTION oracleyield::stamp(){

    require_auth(_self);

    snapshot report;

    report.timestamp = current_time_point();

    report.eos_usd_rate = get_oracle_rate(); //get EOS/USD rate from oracle

    //Iterate through all approved protocols
    auto p_itr = _protocols.begin();

    while (p_itr!=_protocols.end()){

        //print("protocol : ", p_itr->contract, "\n");

        //if the protocol has been approved for rewards
        if (p_itr->approved) {

            oracleyield::tvl_item tvli;

            auto rex_itr = _rexbaltable.find(p_itr->contract.value);

            asset rex_qty = {0, SYSTEM_TOKEN_SYMBOL}; //fetch REX balance

            if (rex_itr!= _rexbaltable.end()){
                rex_qty.amount = get_rex_in_eos(rex_itr->rex_balance).amount; //convert REX to EOS
            }

            asset eos_qty = get_contract_balance(p_itr->contract, std::make_pair(SYSTEM_TOKEN_CONTRACT, SYSTEM_TOKEN_SYMBOL));  //get EOS balance
            asset usdt_qty = get_contract_balance(p_itr->contract, std::make_pair(TETHER_TOKEN_CONTRACT, TETHER_TOKEN_SYMBOL)); //get USDT balance

            //print("rex_qty : ", rex_qty, "\n");
            //print("eos_qty : ", eos_qty, "\n");
            //print("usdt_qty : ", usdt_qty, "\n");

            eos_qty+=rex_qty;

            //add all balances
            asset total = eos_qty;

            total+= divide_assets(usdt_qty, report.eos_usd_rate, SYSTEM_TOKEN_SYMBOL) ; //convert USDT to EOS

            //print("total value in EOS : ", total, "\n");

            tvli.assets.push_back(eos_qty);
            tvli.assets.push_back(usdt_qty);

            //tvli.eos_in_rex = rex_qty;
            tvli.total_in_eos = total;

            report.tvl_items[p_itr->contract] = tvli;
        }

        p_itr++;

    }

    //update snapshot data
    update_global_snapshots(report);

    //print report to block log
    action act_report(
        permission_level{_self, "active"_n},
        _self, "report"_n,
        std::make_tuple(report)
    );
    act_report.send();

}


//called inline when a stamp action successfully executes
ACTION oracleyield::report(snapshot report){

    require_auth(_self);

}

//zero out the contract RAM
ACTION oracleyield::clear() {

    require_auth(_self);

        while (_snapshots.begin() != _snapshots.end()) _snapshots.erase(_snapshots.begin());

}