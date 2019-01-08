from BaseOrderbook import BaseOrderBook
import threading
import cbor
import zmq
import toolz
import binascii
import time
import mmap
import posix_ipc
import syslog
import numpy as np
from Messaging.Patterns import ZmqConnector


class ZmqOrderBook(BaseOrderBook):
    def __init__(self, ports={'rep': 5054, 'pull': 5055, 'sub': 5056}, suffix='', *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.ports = ports
        self.render_iterations = 0
        self.setup_response_map()
        self.shm_path = '/sh' + self.name.replace('/', '') + suffix
        self.shm_buffers = {}
        self.mmaps = {}
        self.semaphores = {}
        self.suffix = suffix
        self.buffer_size = 64
        self.setup_termination()
        self.setup_render_array()
        self.setup_semaphores()
        self.last_render = 0
        self.setup_sockets()
        if('binance' in self.name):
            self.cross_prevention = False

    def create_semaphore(self, name):
        path = '/S' + self.name.replace('/', '') + self.suffix + '_' + name
        self.semaphores[name] = posix_ipc.Semaphore(path, posix_ipc.O_CREAT, initial_value=1)

    def setup_semaphores(self):
        self.semaphores['normal'] = posix_ipc.Semaphore(None, posix_ipc.O_CREX, initial_value=1)
        self.semaphores['bid'] = posix_ipc.Semaphore('/S' + self.name.replace('/', '') + self.suffix + '_b', posix_ipc.O_CREAT, initial_value=1)
        self.semaphores['ask'] = posix_ipc.Semaphore('/S' +self.name.replace('/', '') + self.suffix + '_a', posix_ipc.O_CREAT, initial_value=1)

    def setup_render_array(self):
        self.bids_shm = self.create_shm_array('/sh' + self.name.replace('/', '') + '_bids', self.buffer_size,
                                          dtype=[('quantity', '>f8'), ('price', '>f8')])
        self.bids_shm.fill(-1)
        self.asks_shm = self.create_shm_array('/sh' + self.name.replace('/', '') + '_asks', self.buffer_size,
                                          dtype=[('quantity', '>f8'), ('price', '>f8')])
        self.asks_shm.fill(-1)

    def render(self):
        if(not self.changedSinceRender):
            return
        if('binance' in self.name):
            now = time.time()
            if(now - self.lastInsert < 0.200):
                return
            if(self.get_first_limit('bid')[1] > self.get_first_limit('ask')[1]):
                print('wiping for crossover', self.name)
                self.wipe_and_reset()
        with self.semaphores['normal']:
            self.render_no_lock()
        self.changedSinceRender = False

    def render_no_lock(self):
        self.__write_to_sidebook_shm__(super().get_sidebook('bid'), 'bid')
        self.__write_to_sidebook_shm__(super().get_sidebook('ask'), 'ask')
        self.last_render += 1

    def __write_to_sidebook_shm__(self, sidebook, side):
        if side == 'bid':
            with self.semaphores['bid']:
                self.bids_shm[:len(sidebook)] = np.array(sidebook, dtype=[('quantity', '>f8'), ('price', '>f8')])
        if side == 'ask':
            with self.semaphores['ask']:
                self.asks_shm[:len(sidebook)] = np.array(sidebook, dtype=[('quantity', '>f8'), ('price', '>f8')])

    def terminate_semaphores(self):
        [self.close_semaphore(sem) for sem in ['A', 'B', 'C']]

    def terminate_shm_buffer(self, path):
        try:
            if path in self.mmaps: self.mmaps[path].close()
            if path in self.shm_buffers: self.shm_buffers[path].unlink()
        except:
            pass

    def __del__(self):
        try:
            self.bids_shm, self.asks_shm = None, None
            self.terminate_shms()
            self.terminate_semaphores()
            self.connector.context.destroy()
        except BaseException as e:
            self.loggit("Failed destroying ZmqOrderbook {}:{}".format(self.name, e), severity=syslog.LOG_ERR)

    def terminate_shms(self):
        to_terminate = ['_bids', '_asks']
        [self.terminate_shm_buffer(self.shm_path + name) for name in to_terminate]

    def setup_termination(self):
        self.terminate = False

    def create_shm_array(self, path, size, dtype=float):
        self.shm_buffers[path] = posix_ipc.SharedMemory(path, posix_ipc.O_CREAT, size=size*8)
        self.mmaps[path] = mmap.mmap(self.shm_buffers[path].fd, self.shm_buffers[path].size)
        self.shm_buffers[path].close_fd()
        return np.frombuffer(self.mmaps[path], dtype=dtype)

    def insert_no_lock(self, side, quantity, price):
        super().insert(side, quantity, price)

    def insert(self, side, quantity, price):
        with self.semaphores['normal']:
            self.check_for_cross(side, quantity, price)
            super().insert(side, quantity, price)

    def get_sidebook(self, side):
        with self.semaphores['normal']:
            return super().get_sidebook(side)

    def get_sidebook_no_lock(self, side):
        return super().get_sidebook(side)

    def get_last_render_time(self):
        return (time.time() - self.last_render)

    def setup_response_map(self):
        self.response_map = {
            'insert': self.insert_no_lock,
            'getAsksUpToVolume': self.getAsksUpToVolume,
            'getBidsUpToVolume': self.getBidsUpToVolume,
            'get_prices': self.get_prices,
            'insert_whole_sidebook': self.insert_whole_sidebook,
            'get_volume_for': self.get_volume_for,
            'bids_indices': self.get_bids_indices,
            'asks_indices': self.get_asks_indices,
            'get_whole_book': self.get_whole_book,
            'last_render': self.get_last_render_time,
            'get_first_limit': self.get_first_limit,
            'find_next_bid': self.find_next_bid,
            'find_next_ask': self.find_next_ask,
            'get_weighted_price': self.get_weighted_price,
            'get_sidebook': self.get_sidebook_no_lock,
            'clean_bids': self.clean_bids,
            'clean_asks': self.clean_asks,
        }

    def generate_response(self, request):
        message = self.response_map[request['type']](**request['parameters'])
        return self.serialize(message)

    def get_bids_indices(self):
        return self.bids_indices

    def get_asks_indices(self):
        return self.asks_indices

    def receive_rep(self):
        try:
            request = self.rep_socket.recv()
            request = self.deserialize(request)
            return request
        except zmq.error.ContextTerminated:
            return None

    def generate_checksum_row_string(self, price, volume):
        strPrice = str(price)
        strVol = str(volume)
        #bitfinex uses node which switches to scientific notation at e-7
        #e-7 and e-8 are possible e-9 should not be (less than 1 sat)
        if 'e' in strPrice and not 'e-07' in strPrice and not 'e-08' in strPrice:
            strPrice = ("%0.15f" % price).rstrip('0')
        elif 'e-07' in strPrice:
            strPrice = strPrice.replace('e-07', 'e-7')
        elif 'e-08' in strPrice:
            strPrice = strPrice.replace('e-08', 'e-8')
        if 'e' in strVol and not 'e-07' in strVol and not 'e-08' in strVol:
            strVol = ("%0.15f" % volume).rstrip('0')
        elif 'e-07' in strVol:
            strVol = strVol.replace('e-07', 'e-7')
        elif 'e-08' in strVol:
            strVol = strVol.replace('e-08', 'e-8')
        return strPrice + ":" + strVol

    def compute_checksum(self):
        csArrBids = []
        csArrAsks = []
        #mirror bitfinex code here: http://blog.bitfinex.com/api/bitfinex-api-order-books-checksums/
        count = 0
        for x in sorted(self.bids, reverse=True):
            csArrBids.append(self.generate_checksum_row_string(x, self.bids[x]))
            count+=1
            if(count > 25):
                break
        count = 0
        for x in sorted(self.asks):
            csArrAsks.append(self.generate_checksum_row_string(x, -self.asks[x]))
            count+=1
            if(count > 25):
                break
        
        #bitfinex alternates the bids and asks
        csArr = list(toolz.itertoolz.interleave([csArrBids, csArrAsks]))
        return binascii.crc32(":".join(csArr).encode('utf-8'))
  
    def respond_rep(self, request):
        response = self.generate_response(request)
        self.rep_socket.send(response)

    def deserialize(self, data):
        return cbor.loads(data)

    def serialize(self, data):
        return cbor.dumps(data)

    def receive_pull(self):
        try:
            request = self.pull_socket.recv()
            request = self.deserialize(request)
            return request
        except zmq.error.ContextTerminated:
            return None

    #@timerfunc
    def execute_pull_request(self, request):
        if request['type'] == 'shutdown':
            self.loggit('Received Terminating request')
            self.terminate = True
            return

        return self.response_map[request['type']](**request['parameters'])

    def setup_sockets(self):
        self.connector = ZmqConnector()
        self.rep_socket = self.connector.Rep(self.ports['rep'])
        self.pull_socket = self.connector.PullBind(self.ports['pull'])
        self.poller = zmq.Poller()
        self.poller.register(self.rep_socket, zmq.POLLIN)
        self.poller.register(self.pull_socket, zmq.POLLIN)

    def treat_requests(self):
        while not self.terminate:
            try:
                socks = dict(self.poller.poll())
            except zmq.error.ZMQError:
                break

            if self.rep_socket in socks:
                request = self.receive_rep()
                if request:
                    with self.semaphores['normal']:
                        self.respond_rep(request)
            if self.pull_socket in socks:
                request = self.receive_pull()
                if request:
                    with self.semaphores['normal']:
                        self.execute_pull_request(request)

    def start(self):
        self.main_thread = threading.Thread(target=self.run)
        self.main_thread.start()

    def stop(self):
        self.terminate = True
        self.connector.context.destroy()
        self.rec_thread.join()

    def close_semaphore(self, name):
        try:
            self.semaphores[name].release()
        except:
            pass
        try:
            self.semaphores[name].close()
        except:
            pass

    def run(self):
        self.loggit("Running ZmqOrderbook up with {}".format(self.ports))
        self.rec_thread = threading.Thread(target=self.treat_requests)
        self.rec_thread.start()
        while not self.terminate:
            time.sleep(2)
        self.loggit("Terminating ZmqOrderbook up with {}".format(self.ports), severity=syslog.LOG_DEBUG)
