import synnax as sy
import logging
log = logging.getLogger(' Zeroer')
logging.basicConfig(level=logging.INFO)

SYNNAX_IP    = '128.46.118.59'
SYNNAX_PORT  = 9090
LOX_CHANNEL_NAME    = 'PT-OX-201_calc'
FUEL_CHANNEL_NAME   = 'PT-FU-201_calc'
HELIUM_CHANNEL_NAME = 'PT-HE-201_calc'
NUM_SAMPLES = 1000

client = sy.Synnax(
    host=SYNNAX_IP,
    port=SYNNAX_PORT,
    username="Bill",
    password="Bill",
    secure=False,
)
log.info(f' Connected to Synnax at {SYNNAX_IP}:{SYNNAX_PORT}')

with client.open_streamer([LOX_CHANNEL_NAME, FUEL_CHANNEL_NAME, HELIUM_CHANNEL_NAME]) as s:
    counts  = [0, 0, 0]
    sums    = [0.0, 0.0, 0.0]

    for frame in s:
        for f in frame[LOX_CHANNEL_NAME]:
            counts[0] += 1
            sums[0]   += f
        for f in frame[FUEL_CHANNEL_NAME]:
            counts[1] += 1
            sums[1]   += f

        for f in frame[HELIUM_CHANNEL_NAME]:
            counts[2] += 1
            sums[2]   += f
        if any(c >= NUM_SAMPLES for c in counts):
            break

print(f'lox:    {sums[0] / NUM_SAMPLES:.7}')
print(f'fuel:   {sums[1] / NUM_SAMPLES:.7}')
print(f'helium: {sums[2] / NUM_SAMPLES:.7}')
