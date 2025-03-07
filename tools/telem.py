from socket import socket, AF_INET, SOCK_STREAM
from enum import IntEnum
from struct import unpack, calcsize
import synnax as sy
import time

client = sy.Synnax(
   host="128.46.118.59", port=9090, username="Bill", password="Bill", secure=False
)

TELEM_FORMAT = '<QQQ'
TELEM_SIZE   = calcsize(TELEM_FORMAT)

timestamp_channel = client.channels.create(
    name="adc_time",
    data_type=sy.DataType.TIMESTAMP,
    retrieve_if_name_exists=True,
    is_index=True,
)

other_channel = client.channels.create(
    name="adc_val",
    data_type=sy.DataType.FLOAT64,
    retrieve_if_name_exists=True,
    index=timestamp_channel.key,
)

if __name__ == '__main__':
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('192.168.1.147', 25565))
        
        start_time = sy.TimeStamp.now()
        with client.open_writer(
            start=start_time,
            channels=["adc_time", "adc_val"],
            enable_auto_commit=True,
        ) as writer:
            try:
                # get single packet
                packet = s.recv(TELEM_SIZE)
                while (len(packet) != TELEM_SIZE):
                    packet = s.recv(TELEM_SIZE)

                offset = start_time + packet[0] * 1000

                buff = b''
                while True:
                    packet = s.recv(1000)
                    if packet: 
                        buff += packet

                    while len(buff) >= TELEM_SIZE:
                        p = buff[:TELEM_SIZE]
                        buff = buff[TELEM_SIZE:]

                        if len(p) != TELEM_SIZE:
                            print(len(p))
                            break

                        deser_packet = unpack(TELEM_FORMAT, p)
                        
                        if deser_packet[2] == 2:
                            print((((deser_packet[2]/float(2**31)) * 5) * 0.958241 + 0.0146882) * 187)
                            writer.write({
                                "adc_time": offset + deser_packet[0] * 1000,
                                "adc_val": ((((deser_packet[1]/float(2**31)) * 5) * 0.958241 + 0.0146882) * 1),
                            })
                            print(deser_packet)
            except Exception as e:
                # writer.close()
                print(f'Server closed: {e}')
