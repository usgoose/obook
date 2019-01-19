#include "orderbook.hpp"
#include <unistd.h>
#include <iomanip>

using namespace boost::interprocess;


void OrderbookReader::init_shm(std::string path){
  std::cout << "Doing reader shm" << '\n';
  bids = new SideBook(path + BID_PATH_SUFFIX, read_shm, 0.0);
  asks = new SideBook(path + ASK_PATH_SUFFIX, read_shm, 999999999999.9);
}

orderbook_extract* OrderbookReader::asks_up_to_volume(number target_volume) {
  orderbook_extract *result = new orderbook_extract();

  for (sidebook_ascender it=asks->begin(); it!=asks->end(); ++it){
    target_volume -= quantity(it);
    if (target_volume <= 0.000000000) {

      result->push_back(orderbook_entry_rep(price(it), (quantity(it) + target_volume)));
      break;
    }
    result->push_back(orderbook_entry_rep(price(it), quantity(it)));
  }
  return result;
}

orderbook_extract* OrderbookReader::bids_up_to_volume(number target_volume) {
  orderbook_extract *result = new orderbook_extract();

  for (sidebook_ascender it=bids->begin(); it!=bids->end(); ++it){
    std::cout << "Target is " << target_volume << " and next quantity " << quantity(it) << std::endl;
    target_volume -= quantity(it);
    if (target_volume <= 0.00000000000) {
      std::cout << "Target is finally " << target_volume << std::endl;
      std::cout << "Pushing finally " << price(it) << " " << (quantity(it) + target_volume) << std::endl;
      result->push_back(orderbook_entry_rep(price(it), (quantity(it) + target_volume)));
      break;
    }
    result->push_back(orderbook_entry_rep(price(it), quantity(it)));
  }
  return result;
}

void OrderbookReader::display_side (order_side side) {
  if (side == ask) {
    for (sidebook_ascender it=asks->begin(); it!=asks->end(); ++it)
      std::cout << "ASK: " << std::setprecision(15) << price(it) << " => " << std::setprecision(20) << quantity(it) << '\n';
  } else if (side == bid) {
    for (sidebook_ascender it=bids->begin(); it!=bids->end(); ++it)
      std::cout << "BID: " << std::setprecision(15) << price(it) << " => " << std::setprecision(20) << quantity(it) << '\n';
  }
}

void OrderbookWriter::set_quantity_at (order_side side, number new_price, number new_quantity) {
  if (side == ask)
    asks->insert_ask(new_price, new_quantity);
  else if (side == bid)
    bids->insert_bid(new_price, new_quantity);
}

void OrderbookWriter::init_shm(std::string path) {
  bids = new SideBook(path + BID_PATH_SUFFIX, read_write_shm, 0.0);
  asks = new SideBook(path + ASK_PATH_SUFFIX, read_write_shm, 999999999999.9);
}
