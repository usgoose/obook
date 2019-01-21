#include "orderbook.hpp"
#include <unistd.h>
#include <iomanip>

using namespace boost::interprocess;


void OrderbookReader::init_shm(std::string path){
  std::cout << "Doing reader shm" << '\n';
  bids = new SideBook(path + BID_PATH_SUFFIX, read_shm, static_cast<long double>(0.0));
  asks = new SideBook(path + ASK_PATH_SUFFIX, read_shm, static_cast<long double>(999999999999.9));
}

std::vector< std::vector<long double> > OrderbookReader::_side_up_to_volume_(SideBook *sb, number target_volume) {
  std::vector< std::vector<long double> > result;
  for (sidebook_ascender it=sb->begin(); it!=sb->end(); ++it){
    target_volume -= quantity(it);
    if (target_volume <= static_cast<long double>(0.0)) {
      std::vector<number> current_entry;
      current_entry.push_back(price(it));
      current_entry.push_back(quantity(it) + target_volume);
      result.push_back(current_entry);
      break;
    }
    if (price(it) == number(0.0))
      break;
    std::vector<number> final_entry;
    final_entry.push_back(price(it));
    final_entry.push_back(quantity(it));
    result.push_back(final_entry);
  }
  return result;
}

std::vector< std::vector<long double> > OrderbookReader::asks_up_to_volume(number target_volume) {
  return _side_up_to_volume_(asks, target_volume);
}

std::vector< std::vector<long double> > OrderbookReader::bids_up_to_volume(number target_volume) {
  return _side_up_to_volume_(bids, target_volume);
}

void OrderbookReader::display_side (order_side side) {
  if (side == ASK) {
    for (sidebook_ascender it=asks->begin(); it!=asks->end(); ++it)
      std::cout << "ASK: " << std::setprecision(15) << price(it) << " => " << std::setprecision(20) << quantity(it) << '\n';
  } else if (side == BID) {
    for (sidebook_ascender it=bids->begin(); it!=bids->end(); ++it)
      std::cout << "BID: " << std::setprecision(15) << price(it) << " => " << std::setprecision(20) << quantity(it) << '\n';
  }
}

void OrderbookWriter::set_quantity_at (order_side side, number new_price, number new_quantity) {
  if (side == ASK)
    asks->insert_ask(new_price, new_quantity);
  else if (side == BID)
    bids->insert_bid(new_price, new_quantity);
}

void OrderbookWriter::init_shm(std::string path) {
  bids = new SideBook(path + BID_PATH_SUFFIX, read_write_shm, static_cast<long double>(0.0));
  asks = new SideBook(path + ASK_PATH_SUFFIX, read_write_shm, static_cast<long double>(999999999999.9));
}
