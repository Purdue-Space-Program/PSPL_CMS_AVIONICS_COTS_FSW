from socket import socket, AF_INET, SOCK_STREAM
from enum import IntEnum
from struct import unpack, calcsize
import pandas as pd
import synnax as sy

class Channel(IntEnum):
    PT_HE = 0
    PT_FU = 2
    PT_OX = 4
    TC_01 = 5
    TC_02 = 6
    BB_FU_STATE = 7
    BB_OX_STATE = 8
    BB_FU_POS   = 9
    BB_OX_POS   = 10
    BB_FU_UPPER_SETP = 11
    BB_OX_UPPER_SETP = 12
    BB_FU_LOWER_SETP = 13
    BB_OX_LOWER_SETP = 14

TELEM_FORMAT = '<Qi4xQ'
TELEM_SIZE   = calcsize(TELEM_FORMAT)

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx')
channels = [item for name in df['Name'] for item in [name, f'{name}_time']]

client = sy.Synnax(
   host="192.168.2.69", port=9090, username="Bill", password="Bill", secure=False
)

if __name__ == '__main__':
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('192.168.2.192', 25565))

        p = s.recv(TELEM_SIZE)
        start_time_avi = p[0]
        start_time = sy.TimeSpan.now().trunc(sy.TimeSpan.MICROSECOND)

        offset = start_time + start_time_avi * 1000
        with client.open_writer(
            start=start_time,
            channels=channels,
            enable_auto_commit=True,
        ) as writer:
            values = []
            try:
                buff = b''
                count = 0
                while True:
                    packet = s.recv(TELEM_SIZE * 10)
                    if packet: 
                        buff += packet

                    while len(buff) >= TELEM_SIZE:
                        p = buff[:TELEM_SIZE]
                        buff = buff[TELEM_SIZE:]

                        if len(p) != TELEM_SIZE:
                            print(len(p))
                            break

                        deser_packet = unpack(TELEM_FORMAT, p)
                        
                        name = df[df['ID'].astype(str) == str(deser_packet[2])]['Name'].iloc[0]

                        writer.write({
                            name: int(deser_packet[1]),
                            f'{name}_time': offset + deser_packet[0] * 1000,
                        })
            except KeyboardInterrupt:
                s.close()
                writer.close()
