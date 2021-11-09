#include <xfersplit.hpp>
#include <eosio/asset.hpp>

ACTION xfersplit::addsplit(name sender, name account, uint8_t allocation) {
  require_auth(get_self());

  // Init the _message table using sender scope
  splits_table _splits(get_self(), sender.value);

  // Get total allocation minus account
  auto split_itr = _splits.begin();
  uint8_t total_allocated = 0;
  while(split_itr != _splits.end()) {
    if(split_itr->account!=account) {
      total_allocated = total_allocated + split_itr->allocation;
    }
    split_itr++;
  }

  total_allocated = total_allocated + allocation;
  // check that total allocation will be less than 100%
  check(total_allocated < 100, "Cannot allocated more than 100%");
  // Find the record from _messages table
  split_itr = _splits.find(account.value);

  if (split_itr == _splits.end()) {
    // Create a message record if it does not exist
    _splits.emplace(get_self(), [&](auto& rec) {
      rec.account = account;
      rec.allocation = allocation;
    });
  } else {
    // Modify a message record if it exists
    _splits.modify(split_itr, get_self(), [&](auto& rec) {
      rec.allocation = allocation;
    });
  }

  // Init the _message table using sender scope
  senders_table _senders(get_self(), get_self().value);

  // Add sender to accounts table if it does not already exist
  auto sender_itr = _senders.find(sender.value);
  if(sender_itr == _senders.end()) {
    _senders.emplace(get_self(), [&](auto& rec) {
      rec.account = sender;
    });
  }
}

ACTION xfersplit::clear() {
  require_auth(get_self());

  splits_table _splits(get_self(), get_self().value);

  // Delete all records in _messages table
  auto split_itr = _splits.begin();
  while (split_itr != _splits.end()) {
    split_itr = _splits.erase(split_itr);
  }
}

void xfersplit::transfer(name from, name to, asset quantity, std::string memo) {
  if (to != _self || from == _self) return;

  splits_table _splits(get_self(), from.value);
  auto split_itr = _splits.begin();
  while(split_itr != _splits.end()) {
    auto quantity_split = quantity * split_itr->allocation / 100;
    std::string memo_split = std::to_string(split_itr->allocation) + "% split from " + from.to_string();
    action(
        permission_level{get_self(), "active"_n},
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple(get_self(), split_itr->account, quantity_split, memo_split)
    ).send();
    split_itr++;
  }
}

