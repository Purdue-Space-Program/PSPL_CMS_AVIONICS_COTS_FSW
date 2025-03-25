import synnax as sy
import command as cmd
# necessary to add channels
# import channels
import logging
log = logging.getLogger(' Channel Factory')
logging.basicConfig(level=logging.INFO)

FUEL_SOLENOID_NAME = 'SV-HE-201'
LOX_SOLENOID_NAME  = 'SV-HE-202'
SYNNAX_IP   = '128.46.118.59'
SYNNAX_PORT = 9090

client = sy.Synnax(
    host=SYNNAX_IP,
    port=SYNNAX_PORT,
    username='Bill',
    password='Bill',
)
log.info(f' Connected to Synnax at {SYNNAX_IP}:{SYNNAX_PORT}')

if __name__ == '__main__':
    with client.open_streamer([FUEL_SOLENOID_NAME + '-CMD', LOX_SOLENOID_NAME + '-CMD']) as s:
        for frame in s:
            for f in frame[FUEL_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_FU_STATE_OPEN.name)
                if f == 0:
                    cmd.send_command(cmd.Command.SET_FU_STATE_ISOLATE.name)
            for f in frame[LOX_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_OX_STATE_OPEN.name)
                if f == 0:
                    cmd.send_command(cmd.Command.SET_OX_STATE_ISOLATE.name)
