#include <iostream>
#include <string>
#include "sidebook.hpp"

#define BID_PATH_SUFFIX "_bids"
#define ASK_PATH_SUFFIX "_asks"


enum order_side { bid, ask };


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

