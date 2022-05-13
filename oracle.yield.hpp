#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <math.h>

using namespace eosio;

CONTRACT oracleyield : public contract {
   public:
      using contract::contract;

      //delphi datapoint structure
      struct delphi_datapoint {

         uint64_t id;

         name owner;

         uint64_t value;
         uint64_t median;
         time_point timestamp;

         uint64_t primary_key() const {return id;}
         uint64_t by_timestamp() const {return timestamp.elapsed.to_seconds();}
         uint64_t by_value() const {return value;}

      };
      typedef eosio::multi_index<"datapoints"_n, delphi_datapoint,
         indexed_by<"value"_n, const_mem_fun<delphi_datapoint, uint64_t, &delphi_datapoint::by_value>>,
         indexed_by<"timestamp"_n, const_mem_fun<delphi_datapoint, uint64_t, &delphi_datapoint::by_timestamp>>> delphipoints;


/*      //eosio.yield tier structure
      struct tier {

        uint8_t number;

        asset min_tvl;
        asset max_tvl;

        uint64_t primary_key()const { return number; }

      };*/

      //eosio.yield protocol structure
      struct protocol {

         name contract;
         name beneficiary;

         //tier current_tier;

         time_point last_claim;

         bool approved;

         uint64_t primary_key()const { return contract.value; }

      };
      typedef eosio::multi_index< "protocols"_n, protocol> protocols;

      //INTERNAL STRUCTURES 

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


      //CONSTANTS

      const name EOSIO_CONTRACT = "eosio"_n;

      const name EOSIO_YIELD_CONTRACT = "testeosyield"_n;

      //TODO : move symbols to a multi_index table
      const name SYSTEM_TOKEN_CONTRACT = "eosio.token"_n;
      const symbol SYSTEM_TOKEN_SYMBOL = symbol("EOS", 4);

      const name TETHER_TOKEN_CONTRACT = "tethertether"_n;
      const symbol TETHER_TOKEN_SYMBOL = symbol("USDT", 4);

      const symbol USD_SYMBOL = symbol("USD", 4);

      //Phase 1 : only EOS and USDT are supported
      const std::vector<std::pair<name, symbol>> TOKENS = { std::make_pair(SYSTEM_TOKEN_CONTRACT, SYSTEM_TOKEN_SYMBOL), 
                                                            std::make_pair(TETHER_TOKEN_CONTRACT, TETHER_TOKEN_SYMBOL)};

      //delphi oracle constants         
      const name ORACLE_CONTRACT = "delphioracle"_n;
      const name EOS_USD = "eosusd"_n;

#ifdef DEBUG

      //shorten time units in debug mode for faster testing
      const uint64_t TEN_MINUTES = seconds(30).to_seconds();
      const uint64_t ONE_DAY = minutes(6).to_seconds();

#else 

      const uint64_t TEN_MINUTES = minutes(10).to_seconds();
      const uint64_t ONE_DAY = days(1).to_seconds();

#endif

      const uint64_t PERIOD_INTERVAL = TEN_MINUTES;


      //ACTIONS 

      ACTION stamp();

      ACTION report(snapshot report);

#ifdef DEBUG
      ACTION clear();
#endif

      //GLOBALS AND CONSTRUCTOR
      
      snapshots _snapshots;
      protocols _protocols;

      rexbaltable _rexbaltable;
      rexpooltable _rexpooltable;

      oracleyield( name receiver, name code, datastream<const char*> ds ) :
        contract(receiver, code, ds),
        _snapshots(receiver, receiver.value), 
        _rexbaltable(EOSIO_CONTRACT, EOSIO_CONTRACT.value), 
        _rexpooltable(EOSIO_CONTRACT, EOSIO_CONTRACT.value), 
        _protocols(EOSIO_YIELD_CONTRACT, EOSIO_YIELD_CONTRACT.value) {}

private: 

      //INTERNAL FUNCTIONS DEFINITION

      asset get_contract_balance(name account, std::pair<name, symbol> token);

      asset get_oracle_rate();

      asset get_rex_in_eos( const asset& rex_quantity );

      void update_global_snapshots(snapshot report);

};