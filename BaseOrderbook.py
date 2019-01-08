from itertools import takewhile
import blist
import syslog
import time


class BaseOrderBook(object):
    def __init__(self, name=None, cross_prevention=True, book_size_limit=25):
        syslog.openlog(ident='Orderbook', logoption=syslog.LOG_PID)
        self.init_asks()
        self.init_bids()
        self.book_size_limit = book_size_limit
        self.last_crossing = 0
        self.name = name
        self.changedSinceRender = True
        self.lastInsert = -1
        self.cross_prevention = cross_prevention

    def check_for_cross(self, side, quantity, price):
        if self.cross_prevention and not self.__is_insert_coherent__(side, quantity, price):
            opposing_first_limit = ('bid', self.get_first_limit('bid')) if side == 'ask' else ('ask', self.get_first_limit('ask'))
            self.log_crossing(side, quantity, price, opposing_first_limit)
            return True
        return False

    def log_crossing(self, side, quantity, price, opposing_first_limit):
        now = time.time()
        if now - self.last_crossing > 240:
            print("Crossing alert {}! {}:{}@{} with 1st limit {}".format(self.name, side, quantity, price, opposing_first_limit))#, severity=syslog.LOG_ERR)
            self.last_crossing = now

    def wipe_and_reset(self):
        self.clean_bids()
        self.clean_asks()

    def insert(self, side, quantity, price):
        self.changedSinceRender = True
        self.lastInsert = time.time()
        if quantity != 0 and self.cross_prevention:
            if self.check_for_cross(side, quantity, price):
                if('binance' in self.name):
                    return
                else:
                    print("wiping ", self.name, "for crossover")
                    self.wipe_and_reset()
                    return
        sidebook = self.bids if side == 'bid' else self.asks
        indices = self.bids_indices if side == 'bid' else self.asks_indices
        if quantity <= 0:
            if self.__is_first_limit__(side, price): self.__set_first_limit__(side, price, quantity)
            self.remove_from_sidebook(price, sidebook, indices)
        elif self.can_insert(side, indices, price):
            self.add_to_sidebook(sidebook, indices, price, quantity)
            if self.__is_first_limit__(side, price): self.__set_first_limit__(side, price, quantity)

    def can_insert(self, side, indices, price):
        if len(indices) < self.book_size_limit:
            return True
        if side == 'ask' and price <= indices[-1]:
            return True
        if side == 'bid' and price >= indices[0]:
            return True
        return False

    def add_to_sidebook(self, sidebook, indices, price, quantity):
        sidebook[price] = quantity
        try:
            indices.add(price)
        except:
            print('Failed to insert', price)

    def remove_from_sidebook(self, price, sidebook, indices):
        try:
            del sidebook[price]
            indices.remove(price)
        except KeyError:
            pass
        return

    def get_first_limit(self, side):
        price = self.max_bid if side == 'bid' else self.min_ask
        quantity = self.quantity_at(price, side)
        return (price, quantity)

    def get_sidebook(self, side, maxlen=25):
        sidebook = self.bids if side == 'bid' else self.asks
        indices = reversed(self.bids_indices) if side == 'bid' else self.asks_indices
        result = []
        length = 0
        for price in indices:
            result.append((sidebook[price], price))
            length += 1
            if length>=maxlen:
                break
        return result

    def get_whole_book(self):
        return {'bid': self.get_sidebook('bid'), 'ask': self.get_sidebook('ask')}

    def __is_insert_coherent__(self, side, quantity, price):
        if quantity == 0:
            return True
        if (side == 'bid') and self.min_ask and (price > self.min_ask):
            return False
        if (side == 'ask') and self.max_bid and (price < self.max_bid):
            return False
        return True

    def init_bids(self):
        self.max_bid = None
        self.bids_indices = blist.sortedset()
        self.bids = {}

    def init_asks(self):
        self.min_ask = None
        self.asks_indices = blist.sortedset()
        self.asks = {}

    def clean_bids(self):
        self.max_bid = None
        self.bids_indices = blist.sortedset()
        self.bids = {}
        self.changedSinceRender = True

    def loggit(self, msg, severity=syslog.LOG_INFO):
        syslog.syslog(severity, '{}: {}'.format(self.name, msg))

    def clean_asks(self):
        self.min_ask = None
        self.asks_indices = blist.sortedset()
        self.asks = {}
        self.changedSinceRender = True

    def insert_whole_sidebook(self, side, entries):
        self.clean_bids() if side == 'bid' else self.clean_asks()
        for price, quantity in entries:
            self.insert(side, quantity, price)
        self.changedSinceRender = True

    # Allows to multiply all the quantities by a factor
    def scale_bids_volume_by(self, factor):
        for entry in self.bids:
            entry['quantity'] *= factor

    def scale_asks_volume_by(self, factor):
        for entry in self.asks:
            entry['quantity'] *= factor

    def price_comprised(self, side, price):
        return (price <= self.max_bid) if side == 'bid' else (price >= self.min_ask)

    index_in_book = lambda self, index: (0 <= index < self.buffer_size-1)

    def get_volume_for(self, side, price_limit):
        if not self.price_comprised(side, price_limit):
            return None
        if side == "bid":
            sub_book = takewhile(lambda entry: (entry['price'] >= price_limit) and entry['price'], self.bids)
        elif side == 'ask':
            sub_book = takewhile(lambda entry: (entry['price'] <= price_limit) and entry['price'], self.asks)
        return sum([item[1] for item in sub_book])


    def get_weighted_price(self, side, target_volume):
        prices = self.get_prices(side)
        return self.__compute_weighted_price__(prices, side, target_volume)

    def __compute_weighted_price__(self, prices, side, volume):
        total, divider = 0, 0
        for price in prices:
            size = self.quantity_at(price, side)
            if divider + size >= volume:
                size = volume - divider
                divider, total = divider + size, total + (price * size)
                break
            divider, total = divider + size, total + (price * size)
        return (total / divider) if divider else 0

    def __is_first_limit__(self, side, price):
        if side == 'bid':
            return (self.max_bid is None) or (price >= self.max_bid)
        return (self.min_ask is None) or (price <= self.min_ask)

    def __set_first_limit__(self, side, price, quantity):
        if side == 'bid':
            if not quantity and (price == self.max_bid):
                top_entry = self.find_next_bid(price)
                self.max_bid = top_entry if top_entry else None
            elif quantity:
                self.max_bid = price
        elif side == 'ask':
            if not quantity and (price == self.min_ask):
                top_entry = self.find_next_ask(price)
                self.min_ask = top_entry if top_entry else None
            elif quantity:
                self.min_ask =  price

    def get_prices(self, side):
        if side == 'bid':
            return list(reversed(self.bids_indices))
        elif side == 'ask':
            return list(self.asks_indices)

    def quantity_at(self, price, side):
        if side == 'bid':
            return self.bids[price] if price in self.bids else 0.0
        elif side == 'ask':
            return self.asks[price] if price in self.asks else 0.0

    def getBidsUpToVolume(self, target_volume):
        result = []
        for price in reversed(self.bids_indices):
            size = self.bids[price]
            target_volume -= size
            if target_volume <= 0:
                result.append((price, size + target_volume))
                break
            result.append((price, size))
        return result

    def getAsksUpToVolume(self, target_volume):
        result = []
        for price in self.asks_indices:
            size = self.asks[price]
            target_volume -= size
            if target_volume <= 0:
                result.append((price, size + target_volume))
                break
            result.append((price, size))
        return result

    def find_next_bid(self, price):
        next_index = self.bids_indices.index(price) - 1
        if next_index >= 0:
            return self.bids_indices[next_index]
        else:
            return None

    def find_next_ask(self, price):
        next_index = self.asks_indices.index(price) + 1
        try:
            return self.asks_indices[next_index]
        except IndexError:
            return None

    def display(self):
        print(self.__dict__)
