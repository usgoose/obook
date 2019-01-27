#include "orderbook.hpp"
#include <unistd.h>
#include <iostream>
#include <iomanip>

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace boost::interprocess;


void OrderbookReader::init_shm(std::string path){
  std::cout << "Initiating reader shm" << '\n';
  bids = new SideBook(path + BID_PATH_SUFFIX, read_shm, static_cast<number>(0.0));
  asks = new SideBook(path + ASK_PATH_SUFFIX, read_shm, static_cast<number>(999999999999.9));
}

std::pair<number**, int> OrderbookReader::_side_up_to_volume_(SideBook *sb, number target_volume) {
 number** result = new number*[100];
 int i = 0;
 for (sidebook_ascender it=sb->begin(); it!=sb->end(); ++it){
    target_volume -= quantity(it);
    result[i] = new number[2];
    if (target_volume <= static_cast<number>(0.0)) {
      result[i][0] = price(it);
      result[i][1] = (quantity(it) + target_volume);
      break;
    }
    if (price(it) == number(0.0))
      break;
    result[i][0] = price(it);
    result[i][1] = quantity(it);
    i++;
  }
  return std::pair<number**, int>(result, i);
}

std::pair<number**, int> OrderbookReader::asks_up_to_volume(number target_volume) {
  return _side_up_to_volume_(asks, target_volume);
}

std::pair<number**, int> OrderbookReader::bids_up_to_volume(number target_volume) {
  return _side_up_to_volume_(bids, target_volume);
}

p::list OrderbookReader::_py_side_up_to_volume_(SideBook *sb, number target_volume) {
  p::list result;
  for (sidebook_ascender it=sb->begin(); it!=sb->end(); ++it){
     target_volume -= quantity(it);
     if (target_volume <= static_cast<number>(0.0)) {
       result.append(p::make_tuple(price(it),  (quantity(it) + target_volume)));
       break;
     }
     if (price(it) == number(0.0))
       break;
    result.append(p::make_tuple(price(it), quantity(it)));
   }
   return result;
}

p::list OrderbookReader::py_bids_up_to_volume(number target_volume) {
  return _py_side_up_to_volume_(bids, target_volume);
}

p::list OrderbookReader::py_asks_up_to_volume(number target_volume) {
  return _py_side_up_to_volume_(asks, target_volume);
}

void OrderbookReader::display_side (order_side side) {
  if (side == ASK) {
    for (sidebook_ascender it=asks->begin(); it!=asks->end() && price(it)!= 0; ++it)
      std::cout << "ASK: " << std::setprecision(15) << price(it) << " => " << std::setprecision(20) << quantity(it) << '\n';
  } else if (side == BID) {
    for (sidebook_ascender it=bids->begin(); it!=bids->end() && price(it)!= 0; ++it)
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
