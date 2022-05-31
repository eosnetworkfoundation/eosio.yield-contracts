/*

  delphioracle

  Authors:
    Guillaume "Gnome" Babin-Tremblay - EOS Titan
    Andrew "netuoso" Chaney - EOS Titan

  Website: https://eostitan.com
  Email: guillaume@eostitan.com

  Github: https://github.com/eostitan/delphioracle/

  Published under MIT License
*/

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <math.h>

using namespace eosio;

// Setup per-chain system constants
#ifdef WAX
    const std::string SYSTEM_SYMBOL = "WAX";
    const uint64_t SYSTEM_PRECISION = 8;
    static const asset one_larimer = asset(1000000, symbol(SYSTEM_SYMBOL, SYSTEM_PRECISION));
#else
    const std::string SYSTEM_SYMBOL = "EOS";
    const uint64_t SYSTEM_PRECISION = 4;
    static const asset one_larimer = asset(1, symbol(SYSTEM_SYMBOL, SYSTEM_PRECISION));
#endif

class [[eosio::contract("delphioracle")]] delphioracle : public eosio::contract {
public:
    using contract::contract;

  //Types

  enum e_asset_type: uint16_t {
      fiat=1,
      cryptocurrency=2,
      erc20_token=3,
      eosio_token=4,
      equity=5,
      derivative=6,
      other=7
  };

  typedef uint16_t asset_type;

  //Holds the last datapoints_count datapoints from qualified oracles
  struct [[eosio::table]] datapoints {
    uint64_t id;
    name owner;
    uint64_t value;
    uint64_t median;
    time_point timestamp;

    uint64_t primary_key() const {return id;}
    uint64_t by_timestamp() const {return timestamp.elapsed.to_seconds();}
    uint64_t by_value() const {return value;}
  };

  //Holds the list of pairs
  struct [[eosio::table]] pairs {
    bool active = false;
    bool bounty_awarded = false;
    bool bounty_edited_by_custodians = false;

    name proposer;
    name name;

    asset bounty_amount = asset(0, symbol(SYSTEM_SYMBOL, SYSTEM_PRECISION));

    std::vector<eosio::name> approving_custodians;
    std::vector<eosio::name> approving_oracles;

    symbol base_symbol;
    asset_type base_type;
    eosio::name base_contract;

    symbol quote_symbol;
    asset_type quote_type;
    eosio::name quote_contract;

    uint64_t quoted_precision;

    uint64_t primary_key() const {return name.value;}
  };

  typedef eosio::multi_index<"pairs"_n, pairs> pairstable;
  typedef eosio::multi_index<"datapoints"_n, datapoints,
      indexed_by<"value"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
      indexed_by<"timestamp"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>> datapointstable;

  [[eosio::action]]
  void setprice( const name pair, const uint64_t value );

  [[eosio::action]]
  void setpair( const name pair, const symbol base_symbol, const name base_contract, const uint64_t quoted_precision );
};