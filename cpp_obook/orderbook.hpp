#include <iostream>
#include <string>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <vector>

#define ORDERBOOK_SIZE 65536
#define BID_PATH_SUFFIX "_bids"
#define ASK_PATH_SUFFIX "_asks"

using namespace boost::interprocess;

enum order_side { bid, ask };

typedef managed_shared_memory::segment_manager 									 	segment_manager_t;
typedef allocator<void, segment_manager_t>                           				void_allocator;

typedef std::pair<const double, double> 											orderbook_entry_type;
typedef allocator<orderbook_entry_type, segment_manager_t> 							orderbook_entry_type_allocator;
typedef std::vector<orderbook_entry_type> 											orderbook_extract;

typedef map<double, double, std::less<double>, orderbook_entry_type_allocator>   	side_book_content;
typedef side_book_content::iterator 												side_book_ascender;
typedef side_book_content::reverse_iterator 										side_book_descender;


class SideBook {
	mapped_region *region;
	managed_shared_memory *segment;
	side_book_content *data;
	void_allocator *allocator;
	void setup_segment (std::string, int);

	public:
		SideBook(std::string, int);

		void clear();
		void insert(double, double);

		double quantity_at(double);

		side_book_ascender from_smallest();
		side_book_ascender end_from_smallest();
		side_book_descender from_biggest();
		side_book_descender end_from_biggest();
};

class OrderbookContainer {
	SideBook *bids, *asks;

  public:
  	OrderbookContainer (std::string, int);

  	orderbook_extract* bids_up_to_volume (double);
  	orderbook_extract* asks_up_to_volume (double);

    double quantity_at(order_side, double);
    void set_quantity_at (order_side, double, double);

    void display_side (order_side);
};

