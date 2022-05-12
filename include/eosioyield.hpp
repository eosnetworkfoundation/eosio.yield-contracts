#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <math.h>

using namespace eosio;

CONTRACT eosioyield : public contract {
   public:
      using contract::contract;

      //EXTERNAL STRUCTURES

      //TODO : link header files instead of copying structs

      //eosio.token balance object
      struct account {
        asset   balance;
        uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "accounts"_n, account > accounts;

      struct tvl_item {
        
        std::vector<asset> assets;
        asset total_in_eos ;

      };

      TABLE snapshot {

         time_point timestamp; //timestamp of the snapshot

         asset eos_usd_rate; //measured EOS/USD rate at that snapshot time

         std::map<name, tvl_item> tvl_items;

         uint64_t primary_key()const { return timestamp.sec_since_epoch(); }

      };
      typedef eosio::multi_index< "snapshots"_n, snapshot> snapshots;

      //INTERNAL STRUCTURES

      TABLE protocol {

         name contract;
         //name beneficiary;

         time_point last_claim;

         bool approved;

         uint64_t primary_key()const { return contract.value; }

      }; 
      typedef eosio::multi_index< "protocols"_n, protocol> protocols;

      //CONSTANTS

      const name ORACLE_YIELD_CONTRACT = "testorayield"_n;

      const name SYSTEM_TOKEN_CONTRACT = "eosio.token"_n;
      const symbol SYSTEM_TOKEN_SYMBOL = symbol("EOS", 4); 

#ifdef DEBUG
      const double ANNUAL_YIELD = 0.0005;
#else 
      const double ANNUAL_YIELD = 0.05;
#endif

      const double DAYS_IN_YEAR = 365.25;

      const asset MIN_REWARD = asset(2000000000, SYSTEM_TOKEN_SYMBOL); //200,000 EOS
      const asset MAX_REWARD = asset(120000000000, SYSTEM_TOKEN_SYMBOL); //12 million EOS

#ifdef DEBUG
      //shorten time units in debug mode for faster testing
      const uint64_t EIGHT_HOURS = minutes(2).to_seconds();
      const uint64_t ONE_DAY = minutes(6).to_seconds();
#else 
      const uint64_t EIGHT_HOURS = hours(8).to_seconds();
      const uint64_t ONE_DAY = days(1).to_seconds();
#endif

      const uint64_t CLAIM_INTERVAL = ONE_DAY;

      //ACTIONS 

      ACTION regprotocol(name contract);
      ACTION editprotocol(name contract);
      ACTION claim(name contract, name beneficiary);
      
      ACTION approve(name contract);
      ACTION unapprove(name contract);

#ifdef DEBUG
      ACTION clear();
#endif

      //GLOBALS AND CONSTRUCTOR
      
      protocols _protocols;
      snapshots _snapshots;

      eosioyield( name receiver, name code, datastream<const char*> ds ) :
        contract(receiver, code, ds),
        _protocols(receiver, receiver.value),
        _snapshots(ORACLE_YIELD_CONTRACT, ORACLE_YIELD_CONTRACT.value)   {}


private :

      //INTERNAL FUNCTIONS DEFINITION

      asset get_oracle_tvl(name contract);
      //tier get_tier_from_tvl(asset tvl );
      asset get_contract_balance();
      asset calculate_incentive_reward(asset tvl);

     double asset_to_double(asset a);
      


};