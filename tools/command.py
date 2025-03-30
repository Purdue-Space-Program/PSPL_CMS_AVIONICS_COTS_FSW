from enum import IntEnum
from struct import calcsize, pack, unpack
from socket import socket, AF_INET, SOCK_STREAM
from time import sleep
import pandas as pd

ADC_V_SLOPE  =  0.00000000235714724017
ADC_V_OFFSET = -0.01390133824020600000
AVI_IP   = '128.46.118.59'
AVI_PORT = 1234

class Command(IntEnum):
    SET_FU_UPPER_SETP = 0
    SET_FU_LOWER_SETP = 1
    SET_OX_UPPER_SETP = 2
    SET_OX_LOWER_SETP = 3
    SET_FU_STATE_REGULATE = 4
    SET_FU_STATE_ISOLATE  = 5
    SET_FU_STATE_OPEN     = 6
    SET_OX_STATE_REGULATE = 7
    SET_OX_STATE_ISOLATE  = 8
    SET_OX_STATE_OPEN     = 9
    SET_BB_STATE_REGULATE = 10
    SET_BB_STATE_ISOLATE  = 11
    SET_BB_STATE_OPEN     = 12
    NOOP  = 13
    START = 14
    ABORT = 15

class Status(IntEnum):
    SUCCESS = 0
    NOT_ENOUGH_ARGS = 1
    TOO_MANY_ARGS   = 2
    UNRECOGNIZED_COMMAND = 3

FORMAT_NO_ARGS      = 'B'
FORMAT_NO_ARGS_SIZE = calcsize(FORMAT_NO_ARGS)

FORMAT_1_ARG      = '<BQ'
FORMAT_1_ARG_SIZE = calcsize(FORMAT_1_ARG)

### gets converted to str: (Command, Format)
command_fmts = {
    Command.SET_FU_UPPER_SETP: FORMAT_1_ARG,
    Command.SET_FU_LOWER_SETP: FORMAT_1_ARG,
    Command.SET_OX_UPPER_SETP: FORMAT_1_ARG,
    Command.SET_OX_LOWER_SETP: FORMAT_1_ARG,
    Command.SET_FU_STATE_REGULATE: FORMAT_NO_ARGS,
    Command.SET_FU_STATE_ISOLATE:  FORMAT_NO_ARGS,
    Command.SET_FU_STATE_OPEN:     FORMAT_NO_ARGS,
    Command.SET_OX_STATE_REGULATE: FORMAT_NO_ARGS,
    Command.SET_OX_STATE_ISOLATE:  FORMAT_NO_ARGS,
    Command.SET_OX_STATE_OPEN:     FORMAT_NO_ARGS,
    Command.SET_BB_STATE_REGULATE: FORMAT_NO_ARGS,
    Command.SET_BB_STATE_ISOLATE:  FORMAT_NO_ARGS,
    Command.SET_BB_STATE_OPEN:     FORMAT_NO_ARGS,
    Command.NOOP:                  FORMAT_NO_ARGS,
}

commands = { cmd.name.lower(): (cmd, command_fmts[cmd]) for cmd in command_fmts }

aliases = {
    Command.SET_FU_STATE_REGULATE: ('bb_fu_reg', 'bb_fu_regulate'),
    Command.SET_FU_STATE_ISOLATE:  ('bb_fu_iso', 'bb_fu_isolate'),
    Command.SET_FU_STATE_OPEN:      'bb_fu_open',
    Command.SET_OX_STATE_REGULATE: ('bb_ox_reg', 'bb_ox_regulate'),
    Command.SET_OX_STATE_ISOLATE:  ('bb_ox_iso', 'bb_ox_isolate'),
    Command.SET_OX_STATE_OPEN:      'bb_ox_open',
    Command.SET_FU_UPPER_SETP:      'fu_set_upper',
    Command.SET_FU_LOWER_SETP:      'fu_set_lower',
    Command.SET_OX_UPPER_SETP:      'ox_set_upper',
    Command.SET_OX_LOWER_SETP:      'ox_set_lower',
}

for cmd, alias in aliases.items():
    if isinstance(alias, tuple):
        for a in alias:
            commands[a] = (cmd, command_fmts[cmd])
    else:
        commands[alias] = (cmd, command_fmts[cmd])

def print_help() -> None:
    print('Help!')

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx', 'channels')

### TODO: GIVE ME A RETURN TYPE </3
def send_command(cmd: str, args: list[str] | None = None, sock = None) -> Status:
    cmd = cmd.lower()
    int_args = [int(a) for a in args] if args else []

    cmd_id = commands[cmd][0].value

    # THIS SHIT IS INCORRECT ACTUALLY, WE NEED TO SEND THE ADC VALUE AND REVERSE THE CONVERSION FUUUUCK
    match Command(cmd_id):
        case Command.SET_FU_UPPER_SETP | Command.SET_FU_LOWER_SETP:
            row = df[df['Name'] == 'PT-FU-201']
            if args:
                int_args = [(((i - row['Offset']) / row['Slope']) - ADC_V_OFFSET) / ADC_V_SLOPE for i in int_args]
        case Command.SET_OX_UPPER_SETP | Command.SET_OX_LOWER_SETP:
            row = df[df['Name'] == 'PT-OX-201']
            if args:
                int_args = [(((i - row['Offset']) / row['Slope']) - ADC_V_OFFSET) / ADC_V_SLOPE for i in int_args]

    packet = pack(commands[cmd.lower()][1], cmd_id, *int_args)
    if sock:
        sock.send(packet)

        val = int.from_bytes(sock.recv(1), byteorder='big')
    else:
        with socket(AF_INET, SOCK_STREAM) as s:
            s.connect((AVI_IP, AVI_PORT))

            s.send(packet)

            val = int.from_bytes(s.recv(1), byteorder='big')
            return Status(val)

if __name__ == '__main__':
    try:
        while True:
            inp = input('> ').lower().strip(' \n').split(' ')

            if inp[0] == 'help' or inp[0] == 'h':
                print_help()
                continue

            if inp[0] in commands:
                print(f'Status: {send_command(inp[0], inp[1:]).name}')
            elif inp[0] != '':
                print('Command not recognized!!')

    except KeyboardInterrupt:
        pass
