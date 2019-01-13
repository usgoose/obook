#include "orderbook.hpp"
#include <unistd.h>

using namespace boost::interprocess;


OrderbookContainer::OrderbookContainer(std::string path, int mode){
  bids = new SideBook(path + BID_PATH_SUFFIX, mode);
  asks = new SideBook(path + ASK_PATH_SUFFIX, mode);
}

orderbook_extract* OrderbookContainer::asks_up_to_volume(double target_volume) {
  orderbook_extract *result = new orderbook_extract();

  for (side_book_ascender it=asks->from_smallest(); it!=asks->end_from_smallest(); ++it){
    target_volume -= it->second;
    if (target_volume <= 0) {
      result->push_back(orderbook_entry_type(it->first, (it->second + target_volume)));
      break;
    }
    result->push_back(orderbook_entry_type(it->first, it->second));
  }
  return result;
}

orderbook_extract* OrderbookContainer::bids_up_to_volume(double target_volume) {
  orderbook_extract *result = new orderbook_extract();

  for (side_book_descender it=bids->from_biggest(); it!=bids->end_from_biggest(); ++it){
    target_volume -= it->second;
    if (target_volume <= 0) {
      result->push_back(orderbook_entry_type(it->first, (it->second + target_volume)));
      break;
    }
    result->push_back(orderbook_entry_type(it->first, it->second));
  }
  return result;
}

void OrderbookContainer::set_quantity_at (order_side side, double price, double quantity) {
  if (side == ask)
    asks->insert(price, quantity);
  else if (side == bid)
    bids->insert(price, quantity);
}

double OrderbookContainer::quantity_at (order_side side, double price) {
  if (side == ask)
    return asks->quantity_at(price);
  else
    return bids->quantity_at(price);
}

void OrderbookContainer::display_side (order_side side) {
  if (side == ask) {
    for (side_book_ascender it=asks->from_smallest(); it!=asks->end_from_smallest(); ++it)
      std::cout << "ASK: " << it->first << " => " << it->second << '\n';
  } else if (side == bid) {
  	for (side_book_descender it=bids->from_biggest(); it!=bids->end_from_biggest(); ++it)
      std::cout << "BID: " << it->first << " => " << it->second << '\n';
  }
}