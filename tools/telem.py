from socket import socket, AF_INET, SOCK_STREAM
from struct import unpack, calcsize
from enum import IntEnum
import pandas as pd
import synnax as sy
import logging
log = logging.getLogger(' Telemetry Client')
logging.basicConfig(level=logging.INFO)

class Channel(IntEnum):
    PT_OX    = 0
    PT_FU    = 1
    PT_HE    = 2
    TC_0     = 7
    TC_1     = 9
    BB_FU_STATE = 10
    BB_OX_STATE = 11
    BB_FU_POS   = 12
    BB_OX_POS   = 13
    BB_FU_UPPER_SETP   = 14
    BB_OX_UPPER_SETP   = 15
    BB_FU_LOWER_SETP   = 16
    BB_OX_LOWER_SETP   = 17
    FREE_SPACE = 18

TELEM_FORMAT = '<QQQ'
TELEM_SIZE   = calcsize(TELEM_FORMAT)
SYNNAX_IP    = '128.46.118.59'
SYNNAX_PORT  = 9090
AVI_IP       = '128.46.118.59'
AVI_PORT     = 25565

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx', sheet_name='channels')
channels = [item for name in df['Name'] for item in [name, f'{name}_time']]

client = sy.Synnax(
    host=SYNNAX_IP,
    port=SYNNAX_PORT,
    username="Bill",
    password="Bill",
    secure=False,
)
log.info(f' Connected to Synnax at {SYNNAX_IP}:{SYNNAX_PORT}')

if __name__ == '__main__':
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect((AVI_IP, AVI_PORT))
        log.info(f' Connected to Avionics system at {AVI_IP}:{AVI_PORT}')

        p = s.recv(TELEM_SIZE)
        dp = unpack(TELEM_FORMAT, p)
        start_time_avi = dp[0]

        start_time = sy.TimeStamp.now()
        offset = start_time - (start_time_avi * 1000)

        with client.open_writer(
            start=start_time,
            channels=channels,
            enable_auto_commit=True,
        ) as writer:
            try:
                buff = b''
                while True:
                    packet = s.recv(TELEM_SIZE)
                    if packet: 
                        buff += packet

                    while len(buff) >= TELEM_SIZE:
                        p = buff[:TELEM_SIZE]
                        buff = buff[TELEM_SIZE:]

                        deser_packet = unpack(TELEM_FORMAT, p)
                        match deser_packet[2]:
                            case Channel.PT_OX | Channel.PT_FU | Channel.PT_HE:
                                deser_packet = unpack('<Qi4xQ', p)

                        name = df[df['ID'].astype(str) == str(deser_packet[2])]['Name'].iloc[0]


                        writer.write({
                            name: deser_packet[1],
                            f'{name}_time': offset + deser_packet[0] * 1000,
                        })
            except KeyboardInterrupt:
                pass
            finally:
                s.close()
                writer.close()
