from enum import IntEnum
from struct import calcsize, pack, unpack
from socket import socket, AF_INET, SOCK_STREAM

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
    NOOP = 13

FORMAT_NO_ARGS      = 'B'
FORMAT_NO_ARGS_SIZE = calcsize(FORMAT_NO_ARGS)

FORMAT_1_ARG      = 'BQ'
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

### TODO: GIVE ME A RETURN TYPE </3
def send_command(cmd: str, *args, sock: socket | None = None):
    # if socket given, use it
    #   else open and close one
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(("192.168.1.103", 1234))
        
        packet = pack(commands[cmd][1], commands[cmd][0].value)
        s.send(packet)

        val = s.recv(1)
        print(val)

if __name__ == '__main__':
    try:
        while True:
            inp = input('> ').lower().strip(' \n').split(' ')

            if inp[0] == 'help' or inp[0] == 'h':
                print_help()
                continue
            
            if inp[0] in commands:
                send_command(inp[0])
            elif inp[0] != '':
                print('command not recognized!')

    except KeyboardInterrupt:
        print()
        pass
