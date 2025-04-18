from socket import socket, AF_INET, SOCK_STREAM
from struct import unpack
from enum import IntEnum
import constants
from constants import TELEM_FORMAT, TELEM_SIZE, SYNNAX_IP, SYNNAX_PORT, AVI_IP, AVI_CMD_PORT, AVI_TELEM_PORT

import pandas as pd
import synnax as sy

from utils import get_logger, get_synnax_client, get_telem_configs
log = get_logger('Telemtry')

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
    BB_FU_LOWER_SETP   = 15
    BB_OX_UPPER_SETP   = 16
    BB_OX_LOWER_SETP   = 17
    FREE_SPACE = 18


df = get_telem_configs()
channels = [item for name in df['Name'] for item in [name, f'{name}_time']]

client = get_synnax_client()
log.info(f' Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

def main():
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect((AVI_IP, AVI_TELEM_PORT))
        log.info(f' Connected to Avionics system at {AVI_IP}:{AVI_TELEM_PORT}')

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
                        id = deser_packet[2]
                        value = deser_packet[1]
                        match id:
                            case Channel.PT_OX | Channel.PT_FU | Channel.PT_HE:
                                deser_packet = unpack('<Qi4xQ', p)

                        name = df[df['ID'].astype(str) == str(id)]['Name'].iloc[0]

                        writer.write({
                            name: value,
                            f'{name}_time': offset + deser_packet[0] * 1000,
                        })
            except KeyboardInterrupt:
                pass
            finally:
                s.close()
                writer.close()

if __name__ == '__main__':
    main()
