import zmq
import sys
import os
import time
import numpy as np

sys.path.append(os.getcwd() + '/build')
import msg_pb2
from msg_pb2 import *

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:5557")
socket.setsockopt_string(zmq.SUBSCRIBE, "")
time.sleep(1)

msg = Figure()


def unpack_plotly_msg(inputs):
    def unpack(inputs):
        if type(inputs) is msg_pb2.Dictionary:
            return {k: unpack(v) for (k, v) in inputs.data.items()}
        if type(inputs) is msg_pb2.DictItemVal:
            return unpack(getattr(inputs, inputs.WhichOneof('value')))
        elif type(inputs) in (msg_pb2.SeriesI, msg_pb2.SeriesD):
            return np.array(inputs.data)
        elif type(inputs) in (bool, str, float):
            return inputs
        else:
            raise RuntimeError("Unrecognised type {}".format(type(inputs)))
    return msg.uuid, unpack(msg.kwargs)

while True:

    print("waiting")
    encoded_msg = socket.recv()
    print("rec")

    msg.ParseFromString(encoded_msg)
    # print(str(msg))

    print(unpack_plotly_msg(msg))

    #print(msg.series)
    #print(msg.kwargs)


    break

