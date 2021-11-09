#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT xfersplit : public contract {
  public:
    using contract::contract;
    [[eosio::action]]
    void addsplit(name sender, name account, uint8_t allocation);
    [[eosio::action]]
    void clear();    
    [[eosio::on_notify("eosio.token::transfer")]]
    void transfer(name from, name to, asset quantity, std::string memo);

  private:
    TABLE splits {
      name    sender;
      name    account;      
      uint8_t  allocation;
      auto primary_key() const { return account.value; }
    };

    TABLE senders {
      name    account;
      auto primary_key() const { return account.value; }
    };
    typedef multi_index<name("splits"), splits> splits_table;
    typedef multi_index<name("senders"), senders> senders_table;
};
