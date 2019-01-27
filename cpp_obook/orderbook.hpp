#include <iostream>
#include <string>
#include "sidebook.hpp"
#include <boost/python/numpy.hpp>
#include <tuple>

#define BID_PATH_SUFFIX "_bids"
#define ASK_PATH_SUFFIX "_asks"

#define BID true
#define ASK false

typedef bool order_side;


class OrderbookReader {
  protected:
    SideBook *bids, *asks;
    std::pair<number**, int> _side_up_to_volume_(SideBook*, number);
    boost::python::list _py_side_up_to_volume_(SideBook*, number);


  public:
    virtual void init_shm (std::string);

    std::pair<number**, int> bids_up_to_volume (long double);
    std::pair<number**, int> asks_up_to_volume (long double);

    boost::python::list py_asks_up_to_volume(number target_volume);
    boost::python::list py_bids_up_to_volume(number target_volume);

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

