#include <iostream>
#include <string>
#include "sidebook.hpp"

#define BID_PATH_SUFFIX "_bids"
#define ASK_PATH_SUFFIX "_asks"

#define BID true
#define ASK false

typedef bool order_side;


class OrderbookReader {
  protected:
    SideBook *bids, *asks;
    std::vector< std::vector<long double> > _side_up_to_volume_(SideBook*, number);

  public:
    virtual void init_shm (std::string);

    //orderbook_extract* bids_up_to_volume (long double);
    //orderbook_extract* asks_up_to_volume (long double);

    std::vector< std::vector<long double> > bids_up_to_volume (long double);
    std::vector< std::vector<long double> > asks_up_to_volume (long double);

    long double first_price () {
        return price(bids->begin());
    }
    void display_side (order_side);
};


class OrderbookWriter: public OrderbookReader {
  public:
  	
    void init_shm (std::string);
    void set_quantity_at (order_side, long double, long double);
 };

