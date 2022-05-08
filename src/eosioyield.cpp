#include <eosioyield.hpp>

//converts an asset to a double
double eosioyield::asset_to_double(asset a){

   double d_a = (double)(a.amount);
   double d_multiplier = pow(10, (double)a.symbol.precision());
   double n_val = d_a / d_multiplier;

   return n_val;

}

//get the appropriate tier from TVL
eosioyield::tier eosioyield::get_tier_from_tvl(asset tvl ){

   print("get_tier_from_tvl\n");

   check (tvl.amount>=0, "TVL must be a positive value");

   if (tvl < TIERS[0].max_tvl) return TIERS[0]; //tier zero
   if (tvl > TIERS[TIERS.size()-1].max_tvl) return TIERS[TIERS.size()-1]; //max tier

   //find correct tier for TVL
   for (auto &tier : TIERS){  
     if (tvl >= tier.min_tvl && tvl < tier.max_tvl) return tier;
   }

   check(false, "error retrieving TVL"); //should not happen

}

//get average tvl recorded by oracle 
asset eosioyield::get_oracle_tvl(name contract ){

   print("get_oracle_tvl\n");

   print("  contract : ", contract ,"\n");

   //convert to double
   double d_multiplier = pow(10, (double)SYSTEM_TOKEN_SYMBOL.precision());

   uint64_t c_ts = current_time_point().sec_since_epoch();

   //slice the datapoints into 3x eight hours periods 
   uint64_t period_1 = c_ts - EIGHT_HOURS*3;
   uint64_t period_2 = c_ts - EIGHT_HOURS*2;
   uint64_t period_3 = c_ts - EIGHT_HOURS;

   auto p1_itr = _snapshots.upper_bound(period_1);
   auto p2_itr = _snapshots.upper_bound(period_2);
   auto p3_itr = _snapshots.upper_bound(period_3);

   int d1 = std::distance(p1_itr, p2_itr);
   int d2 = std::distance(p2_itr, p3_itr);
   int d3 = std::distance(p3_itr, _snapshots.end());

   int f1 = 0;
   int f2 = 0;
   int f3 = 0;
   
   double avg_p1 = 0.0;
   double avg_p2 = 0.0;
   double avg_p3 = 0.0;

   print("d1 : ", d1 ,"\n");
   print("d2 : ", d2 ,"\n");
   print("d3 : ", d3 ,"\n");
   print("f1 : ", f1 ,"\n");
   print("f2 : ", f2 ,"\n");
   print("f3 : ", f3 ,"\n");

   //for each slice, calculate average
   while (p1_itr!=p2_itr){
      print("period 1 : ", p1_itr->timestamp.sec_since_epoch(), "\n" );

      double tvl = asset_to_double(p1_itr->tvl_items.find(contract)->second);

      print("  tvl : ", tvl ,"\n");

      if (tvl==0.0) f1++;
      avg_p1+=tvl;
      p1_itr++;
   }
   while (p2_itr!=p3_itr){
      print("period 2 : ", p2_itr->timestamp.sec_since_epoch(), "\n" );

      double tvl = asset_to_double(p2_itr->tvl_items.find(contract)->second);

      print("  tvl : ", tvl ,"\n");

      if (tvl==0.0) f2++;
      avg_p2+=tvl;
      p2_itr++;
   }
   while (p3_itr!=_snapshots.end()){
      print("period 3 : ", p3_itr->timestamp.sec_since_epoch(), "\n" );

      double tvl = asset_to_double(p3_itr->tvl_items.find(contract)->second);

      print("  tvl : ", tvl ,"\n");

      if (tvl==0.0) f3++;
      avg_p3+=tvl;
      p3_itr++;
   }

   avg_p1 /= (d1-f1);
   avg_p2 /= (d2-f2);
   avg_p3 /= (d3-f3);

   print("avg_p1 : ", avg_p1 ,"\n");
   print("avg_p2 : ", avg_p2 ,"\n");
   print("avg_p3 : ", avg_p3 ,"\n");
   
   //calculate the average the 3x 8hours periods 
   double avg_all = (avg_p1 + avg_p2 + avg_p3) / 3;

   print("avg_all : ", avg_all ,"\n");

   //round and convert back to asset 
   uint64_t i_result = uint64_t((avg_all * d_multiplier)+0.5);

   return asset(i_result, SYSTEM_TOKEN_SYMBOL);


}

asset eosioyield::get_contract_balance(){

   print("get_contract_balance\n");

   accounts a_table(SYSTEM_TOKEN_CONTRACT, _self.value);

   auto itr = a_table.find(SYSTEM_TOKEN_SYMBOL.code().raw());

   if (itr == a_table.end()) return asset(0, SYSTEM_TOKEN_SYMBOL);
   else return itr->balance;

}


asset eosioyield::calculate_incentize_reward(asset tvl){

   print("calculate_incentize_reward\n");

   double d_quantity = (double)(tvl.amount);
   double d_multiplier = pow(10, (double)tvl.symbol.precision());

   double n_quantity = d_quantity / d_multiplier;

   double d_reward = (n_quantity * ANNUAL_YIELD) / DAYS_IN_YEAR;

   asset value = {0, SYSTEM_TOKEN_SYMBOL};

   value.amount = uint64_t((d_reward * d_multiplier)+0.5);

   if (value > MAX_REWARD) value = MAX_REWARD;

   return value;

}

ACTION eosioyield::regprotocol( name contract, name beneficiary){

   print("regprotocol\n");

   print("has_auth(_self) ", has_auth(_self), "\n");


#ifdef DEBUG

   if (has_auth(_self) == false){
      require_auth(contract);
   }

#else 
   require_auth(contract);
#endif

   auto itr = _protocols.find(contract.value);

   check(itr==_protocols.end(), "protocol already registered");
   
   _protocols.emplace( get_self(), [&]( auto& p ) {
      p.contract = contract;
      p.beneficiary = beneficiary;
      p.current_tier = TIER_ZERO;
      p.last_claim = time_point(eosio::seconds(0));
      p.approved = false;
   });

}

//change beneficary
ACTION eosioyield::editprotocol( name contract, name beneficiary){

   print("editprotocol\n");

#ifdef DEBUG

   if (has_auth(_self) == false){
      require_auth(contract);
   }

#else 
   require_auth(contract);
#endif

   auto itr = _protocols.find(contract.value);

   check(itr!=_protocols.end(), "protocol not found");
   
   _protocols.modify( itr, same_payer, [&]( auto& p ) {
      p.beneficiary = beneficiary;
   });

}

//approve a protocol
ACTION eosioyield::approve( name contract){

   print("approve\n");


   require_auth(_self);

   auto itr = _protocols.find(contract.value);

   check(itr!=_protocols.end(), "protocol not found");
   
   _protocols.modify( itr, same_payer, [&]( auto& p ) {
      p.approved = true;
   });

}

//unapprove a protocol
ACTION eosioyield::unapprove( name contract){

   print("unapprove\n");

   require_auth(_self);

   auto itr = _protocols.find(contract.value);

   check(itr!=_protocols.end(), "protocol not found");

   _protocols.modify( itr, same_payer, [&]( auto& p ) {
      p.approved = false;
   });

}

//claim rewards (if any)
ACTION eosioyield::claim(name contract){

   print("claim\n");

#ifdef DEBUG

   if (has_auth(_self) == false){
      require_auth(contract);
   }

#else 
   require_auth(contract);
#endif

   auto itr = _protocols.find(contract.value);

   check(itr!=_protocols.end(), "contract not found");
   check(itr->approved, "contract not approved for rewards");

   uint64_t c_ts = current_time_point().sec_since_epoch();

   bool claim_allowed = c_ts>itr->last_claim.sec_since_epoch() + CLAIM_INTERVAL;

   check(claim_allowed, "can only claim once every 24h");

   asset tvl = get_oracle_tvl(contract);
   tier new_tier = get_tier_from_tvl(tvl);

   print("tvl ", tvl, "\n");
   print("new_tier ", new_tier.number, "\n");

   _protocols.modify( itr, same_payer, [&]( auto& p ) {
      p.current_tier = new_tier;
      p.last_claim = current_time_point();
   });

   if(new_tier.number == TIER_ZERO.number) return; // if tier is tier zero, no rewards
   
   asset balance = get_contract_balance();
   asset claim = calculate_incentize_reward(tvl);

   print("eosio.yield balance ", balance, "\n");
   print("claim ", claim, "\n");

   check(balance.amount>= claim.amount, "yield program depleted");
   check(claim.amount> 0, "claim must be higher than 0");

   action act(
     permission_level{_self, "active"_n},
     SYSTEM_TOKEN_CONTRACT, "transfer"_n,
     std::make_tuple(_self, itr->beneficiary, claim, std::string("Yield+ TVL reward") )
   );
   act.send();

}

#ifdef DEBUG

   //zero out the contract RAM
   ACTION eosioyield::clear() {

      require_auth(_self);

       while (_protocols.begin() != _protocols.end()) _protocols.erase(_protocols.begin());

   }

#endif