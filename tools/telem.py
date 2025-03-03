from socket import socket, AF_INET, SOCK_STREAM
from enum import IntEnum
from time import sleep
from struct import unpack, calcsize
# import synnax as sy
#
# client = sy.Synnax(
#    host="128.46.118.59", port=9090, username="Bill", password="Bill", secure=False
# )

TELEM_FORMAT = '<QQB'
TELEM_SIZE   = calcsize(TELEM_FORMAT)

if __name__ == '__main__':
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('192.168.1.142', 25565))

        # with client.open_writer(
        #     start=sy.TimeStamp.now(), 
        #     channels=[],
        #     enable_auto_commit=True,
        # ) as writer:
            
        while True:
            packet = s.recv(TELEM_SIZE)
            deser_packet = unpack(TELEM_FORMAT, packet)
            
            if deser_packet[2] == 2:
                print((((deser_packet[2]/float(2**31)) * 5) * 0.958241 + 0.0146882) * 500)
                # print(deser_packet)
