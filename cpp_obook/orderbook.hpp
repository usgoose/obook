#include <iostream>
#include <string>
#include "sidebook.hpp"

#define BID_PATH_SUFFIX "_bids"
#define ASK_PATH_SUFFIX "_asks"


enum order_side { bid, ask };


class OrderbookReader {
  protected:
	SideBook *bids, *asks;

  public:
  	OrderbookReader (std::string);

    virtual void init_shm (std::string);

  	orderbook_extract* bids_up_to_volume (double);
  	orderbook_extract* asks_up_to_volume (double);

    double quantity_at (order_side, double);
    void display_side (order_side);
};


class OrderbookWriter: public OrderbookReader {
  public:
  	OrderbookWriter (std::string path) : OrderbookReader(path) {};
  	
  	void init_shm (std::string);
    void set_quantity_at (order_side, double, double);
 };

