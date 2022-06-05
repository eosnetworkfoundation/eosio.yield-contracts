
//converts an asset to a double
double eosioyield::asset_to_double(asset a)
{
    double d_a = (double)(a.amount);
    double d_multiplier = pow(10, (double)a.symbol.precision());
    double n_val = d_a / d_multiplier;

    return n_val;
}

//get average tvl recorded by oracle
asset eosioyield::get_oracle_tvl( name contract ) {

    //print("get_oracle_tvl\n");

    //print("  contract : ", contract ,"\n");

    //convert to double
    double d_multiplier = pow(10, (double)SYSTEM_TOKEN_SYMBOL.precision());

    uint64_t c_ts = current_time_point().sec_since_epoch();

    //slice the datapoints into 3x eight hours periods
    uint64_t period_1 = c_ts - EIGHT_HOURS * 3;
    uint64_t period_2 = c_ts - EIGHT_HOURS * 2;
    uint64_t period_3 = c_ts - EIGHT_HOURS;

    auto p1_itr = _snapshots.upper_bound(period_1);
    auto p2_itr = _snapshots.upper_bound(period_2);
    auto p3_itr = _snapshots.upper_bound(period_3);

    int d1 = std::distance(p1_itr, p2_itr);
    int d2 = std::distance(p2_itr, p3_itr);
    int d3 = std::distance(p3_itr, _snapshots.end());

    double avg_p1 = 0.0;
    double avg_p2 = 0.0;
    double avg_p3 = 0.0;

    //print("d1 : ", d1 ,"\n");
    //print("d2 : ", d2 ,"\n");
    //print("d3 : ", d3 ,"\n");

    //for each slice, calculate average
    while (p1_itr!=p2_itr){
        //print("period 1 : ", p1_itr->timestamp.sec_since_epoch(), "\n" );

        auto tvl_itr = p1_itr->tvl_items.find(contract);

        double tvl = 0.0;

        if ( tvl_itr!=p1_itr->tvl_items.end()) asset_to_double(tvl_itr->second.total_in_eos);
        //print("  tvl : ", tvl ,"\n");

        avg_p1+=tvl;
        p1_itr++;
    }
    while (p2_itr!=p3_itr){
        //print("period 2 : ", p2_itr->timestamp.sec_since_epoch(), "\n" );

        auto tvl_itr = p2_itr->tvl_items.find(contract);

        double tvl = 0.0;
        if ( tvl_itr!=p2_itr->tvl_items.end()) asset_to_double(tvl_itr->second.total_in_eos);
        //print("  tvl : ", tvl ,"\n");

        avg_p2+=tvl;
        p2_itr++;
    }
    while (p3_itr!=_snapshots.end()){
        //print("period 3 : ", p3_itr->timestamp.sec_since_epoch(), "\n" );

        auto tvl_itr = p3_itr->tvl_items.find(contract);

        double tvl = 0.0;

        if ( tvl_itr!=p3_itr->tvl_items.end()) asset_to_double(tvl_itr->second.total_in_eos);
        //print("  tvl : ", tvl ,"\n");

        avg_p3+=tvl;
        p3_itr++;
    }

    avg_p1 /= d1;
    avg_p2 /= d2;
    avg_p3 /= d3;

    //print("avg_p1 : ", avg_p1 ,"\n");
    //print("avg_p2 : ", avg_p2 ,"\n");
    //print("avg_p3 : ", avg_p3 ,"\n");

    //calculate the average the 3x 8hours periods
    double avg_all = (avg_p1 + avg_p2 + avg_p3) / 3;

    //print("avg_all : ", avg_all ,"\n");

    //round and convert back to asset
    uint64_t i_result = uint64_t((avg_all * d_multiplier)+0.5);

    return asset(i_result, SYSTEM_TOKEN_SYMBOL);
}

asset eosioyield::get_contract_balance()
{
    //print("get_contract_balance\n");

    accounts a_table(SYSTEM_TOKEN_CONTRACT, _self.value);

    auto itr = a_table.find(SYSTEM_TOKEN_SYMBOL.code().raw());

    if (itr == a_table.end()) return asset(0, SYSTEM_TOKEN_SYMBOL);
    else return itr->balance;
}


asset eosioyield::calculate_incentive_reward(asset tvl)
{
    //print("calculate_incentive_reward\n");

    if (tvl<MIN_REWARD) return asset{0, SYSTEM_TOKEN_SYMBOL};
    if (tvl>MAX_REWARD) tvl = MAX_REWARD;

    double d_quantity = (double)(tvl.amount);
    double d_multiplier = pow(10, (double)tvl.symbol.precision());

    double n_quantity = d_quantity / d_multiplier;

    double d_reward = (n_quantity * ANNUAL_YIELD) / DAYS_IN_YEAR;

    asset value = {0, SYSTEM_TOKEN_SYMBOL};

    value.amount = uint64_t((d_reward * d_multiplier)+0.5);

    return value;
}