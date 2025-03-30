import synnax as sy
import constants
import logging
log = logging.getLogger(' Zeroer')
logging.basicConfig(level=logging.INFO)

NUM_SAMPLES = 1000

client = sy.Synnax(
    host=constants.SYNNAX_IP,
    port=constants.SYNNAX_PORT,
    username="Bill",
    password="Bill",
    secure=False,
)
log.info(f' Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

with client.open_streamer([constants.LOX_CHANNEL_NAME, constants.FUEL_CHANNEL_NAME, constants.HELIUM_CHANNEL_NAME]) as s:
    counts  = [0, 0, 0]
    sums    = [0.0, 0.0, 0.0]

    for frame in s:
        for f in frame[constants.LOX_CHANNEL_NAME]:
            counts[0] += 1
            sums[0]   += f
        for f in frame[constants.FUEL_CHANNEL_NAME]:
            counts[1] += 1
            sums[1]   += f

        for f in frame[constants.HELIUM_CHANNEL_NAME]:
            counts[2] += 1
            sums[2]   += f
        if any(c >= NUM_SAMPLES for c in counts):
            break

print(f'lox:    {sums[0] / NUM_SAMPLES:.7}')
print(f'fuel:   {sums[1] / NUM_SAMPLES:.7}')
print(f'helium: {sums[2] / NUM_SAMPLES:.7}')
