#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <array>
#include <utility>
#include <vector>

#define SIDEBOOK_SIZE       100

using namespace boost::interprocess;

enum shm_mode { read_shm, read_write_shm };

typedef managed_shared_memory::segment_manager 									 	segment_manager_t;
typedef allocator<void, segment_manager_t>                           				void_allocator;

typedef long double                                                                 number;
typedef std::array<number, 2>                                                       orderbook_entry_type;
typedef std::pair<number, number>                                                   orderbook_entry_rep;
typedef std::vector<orderbook_entry_rep >                                           orderbook_extract;

typedef std::array< orderbook_entry_type, SIDEBOOK_SIZE>                            sidebook_content;
typedef sidebook_content::iterator                                                  sidebook_ascender;
typedef sidebook_content::reverse_iterator                                          sidebook_descender;

number quantity(sidebook_content::iterator loc);

number price(sidebook_content::iterator loc);

number quantity(sidebook_content::reverse_iterator loc);

number price(sidebook_content::reverse_iterator loc);


class SideBook {
	mapped_region *region;
	managed_shared_memory *segment;
    sidebook_content *data;
	void_allocator *allocator;

    void fill_with(number);

	void setup_segment (std::string, shm_mode);
    void insert_at_place(sidebook_content*, orderbook_entry_type, sidebook_content::iterator);

	public:
        SideBook(std::string, shm_mode, number);

        void insert_ask(number, number);
        void insert_bid(number, number);


        //long double quantity_at(long double);

        sidebook_ascender begin();
        sidebook_ascender end();
};
