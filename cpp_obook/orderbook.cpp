#include "orderbook.hpp"
#include <unistd.h>

using namespace boost::interprocess;

void SideBook::setup_segment(std::string path, int mode){
	if (mode)
    segment = new managed_shared_memory(open_or_create, path.c_str(), ORDERBOOK_SIZE);
  else
    segment = new managed_shared_memory(open_only, path.c_str());
}

SideBook::SideBook(std::string path, int mode){
  setup_segment(path, mode);
  allocator = new void_allocator(segment->get_segment_manager());
  data = segment->find_or_construct<side_book_content> ("unique")(std::less<double>(), *allocator);
  if (mode) clear();
}

void SideBook::clear(){
	data -> clear();
}

void SideBook::insert(double price, double quantity){
	orderbook_entry_type entry(price, quantity);
	data->insert(entry);
}

double SideBook::quantity_at (double price) {
  return data->at(price);
}

side_book_ascender SideBook::from_smallest() {
  return data->begin();
}

side_book_ascender SideBook::end_from_smallest() {
  return data->end();
}

side_book_descender SideBook::from_biggest() {
  return data->rbegin();
}

side_book_descender SideBook::end_from_biggest() {
  return data->rend();
}


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