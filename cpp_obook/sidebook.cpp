#include "sidebook.hpp"
#include <iostream>

void SideBook::setup_segment(std::string path, shm_mode mode){
	if (mode == read_write_shm)
    segment = new managed_shared_memory(open_or_create, path.c_str(), SIDEBOOK_SIZE);
  else if (mode == read_shm)
    segment = new managed_shared_memory(open_only, path.c_str());
}

SideBook::SideBook(std::string path, shm_mode mode){
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
