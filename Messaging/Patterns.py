import zmq

class ZmqConnector(object):
    def __init__(self):
        self.context = zmq.Context()
        
        
    def Rep(self, port=5555):
        socket = self.context.socket(zmq.REP)
        socket.setsockopt(zmq.LINGER, 1000)
        socket.bind("tcp://*:{}".format(port))
        return socket


    def RepConnect(self, ip="localhost", port=5555):
        socket = self.context.socket(zmq.REP)
        socket.setsockopt(zmq.LINGER, 1000)
        socket.connect("tcp://{}:{}".format(ip, port))
        return socket


    def Req(self, ip='localhost', port=5555):
        socket = self.context.socket(zmq.REQ)
        socket.setsockopt(zmq.LINGER, 5000)
        socket.setsockopt(zmq.RCVTIMEO, 2000)
        socket.connect("tcp://{}:{}".format(ip, port))
        return socket
    
    
    def Sub(self, ip='localhost', port=5555):
        socket = self.context.socket(zmq.SUB)
        socket.connect("tcp://{}:{}".format(ip, port))
        socket.setsockopt_string(zmq.SUBSCRIBE, "")
        return socket


    def Pub(self, port=5555):
        socket = self.context.socket(zmq.PUB)
        socket.bind("tcp://*:{}".format(port))
        return socket


    def PairConnect(self, ip='localhost', port=5555):
        socket = self.context.socket(zmq.PAIR)
        socket.connect("tcp://{}:{}".format(ip, port))
        return socket


    def PairBind(self, port=5555):
        socket = self.context.socket(zmq.Pair)
        socket.bind("tcp://*:{}".format(port))
        return socket
    
    
    def Push(self, port=5555):
        socket = self.context.socket(zmq.PUSH)
        socket.bind("tcp://*:{}".format(port))
        return socket
    
    
    def Pull(self, ip='localhost', port=5558):
        socket = self.context.socket(zmq.PULL)
        socket.setsockopt(zmq.LINGER, 1000)
        socket.connect("tcp://{}:{}".format(ip, port))
        return socket

    def PullBind(self, port=5555):
        socket = self.context.socket(zmq.PULL)
        socket.setsockopt(zmq.LINGER, 1000)
        socket.bind("tcp://*:{}".format(port))
        return socket

    def PushConnect(self, ip='localhost', port=5558):
        socket = self.context.socket(zmq.PUSH)
        socket.connect("tcp://{}:{}".format(ip, port))
        return socket
    
    
def Forwarder(port_in=5555, port_out=5558):
    try:
        context = zmq.Context(1)
        frontend = context.socket(zmq.PUB)
        frontend.bind("tcp://*:{}".format(port_out))
        backend = context.socket(zmq.SUB)
        backend.bind("tcp://*:{}".format(port_in))
        zmq.device(zmq.FORWARDER, frontend, backend)
    except Exception as e:
        print(e)
        print("bringing down zmq device")
    finally:
        pass
        frontend.close()
        backend.close()
        context.term()


def Queue(port_in=5555, port_out=5558):
    try:
        context = zmq.Context(1)
        frontend = context.socket(zmq.XREP)
        frontend.bind("tcp://*:{}".format(port_out))
        backend = context.socket(zmq.XREQ)
        backend.bind("tcp://*:{}".format(port_in))
        zmq.device(zmq.QUEUE, frontend, backend)
    except Exception as e:
        print(e)
        print("bringing down zmq device")
    finally:
        pass
        frontend.close()
        backend.close()
        context.term()


def Streamer(port_in=5555, port_out=5558):
    try:
        context = zmq.Context(1)
        frontend = context.socket(zmq.PULL)
        frontend.bind("tcp://*:{}".format(port_out))
        backend = context.socket(zmq.PUSH)
        backend.bind("tcp://*:{}".format(port_in))
        zmq.device(zmq.STREAMER, frontend, backend)
    except Exception as e:
        print(e)
        print("bringing down zmq device")
    finally:
        pass
        frontend.close()
        backend.close()
        context.term()


