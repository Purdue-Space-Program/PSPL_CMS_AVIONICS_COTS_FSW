import synnax as sy
import command as cmd
import constants
# necessary to add channels
# import channels
import logging
log = logging.getLogger(' Bang Bang GSE')
logging.basicConfig(level=logging.INFO)

client = sy.Synnax(
    host=constants.SYNNAX_IP,
    port=constants.SYNNAX_PORT,
    username='Bill',
    password='Bill',
)
log.info(f' Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

if __name__ == '__main__':
    with client.open_streamer([constants.FUEL_SOLENOID_NAME + '-CMD', constants.LOX_SOLENOID_NAME + '-CMD']) as s:
        for frame in s:
            for f in frame[constants.FUEL_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_FU_STATE_OPEN.name)
                    log.info(' Fuel: OPEN')
                if f == 0:
                    cmd.send_command(cmd.Command.SET_FU_STATE_ISOLATE.name)
                    log.info(' Fuel: CLOSE')
            for f in frame[constants.LOX_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_OX_STATE_OPEN.name)
                    log.info(' Lox: OPEN')
                if f == 0:
                    cmd.send_command(cmd.Command.SET_OX_STATE_ISOLATE.name)
                    log.info(' Lox: CLOSE')
