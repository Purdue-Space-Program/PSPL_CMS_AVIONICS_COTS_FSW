import synnax as sy
from synnax.channel import ChannelKeys, ChannelName
import pandas as pd
import constants
import logging
log = logging.getLogger(' Channel Factory')
logging.basicConfig(level=logging.INFO)

ducers = [
    'PT-OX-201',
    'PT-FU-201',
    'PT-HE-201',
]

client = sy.Synnax(
    host=constants.SYNNAX_IP,
    port=constants.SYNNAX_PORT,
    username='Bill',
    password='Bill',
)
log.info(f' Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx', 'channels')

# with client.open_streamer([LOX_CHANNEL_NAME, FUEL_CHANNEL_NAME, HELIUM_CHANNEL_NAME]) as s:
#     # Zero data
#     counts  = [0.0, 0.0, 0.0]
#     sums    = [0, 0, 0]
#
#     for frame in s:
#         for f in frame[LOX_CHANNEL_NAME]:
#             counts[0] += 1
#             sums[0]   += f
#         for f in frame[FUEL_CHANNEL_NAME]:
#             counts[1] += 1
#             sums[1]   += f
#
#         for f in frame[HELIUM_CHANNEL_NAME]:
#             counts[2] += 1
#             sums[2]   += f
#         if any(c >= NUM_SAMPLES for c in counts):
#             break
#
#
# ch = client.channels.retrieve('PT-OX-201_calc')
# client.channels.create(
#     name='PT-OX-201_zeroed',
#     data_type=sy.DataType.FLOAT64,
#     expression=f'return get(\"PT-OX-201_calc\") - {sums[0] / NUM_SAMPLES} + 14.7',
#     requires=[ch.key],
#     retrieve_if_name_exists=True,
# )
# print(f'lox: {sums[0] / NUM_SAMPLES}')
#
# ch = client.channels.retrieve('PT-FU-201_calc')
# client.channels.create(
#     name='PT-FU-201_zeroed',
#     data_type=sy.DataType.FLOAT64,
#     expression=f'return get(\"PT-FU-201_calc\") - {sums[1] / NUM_SAMPLES} + 14.7',
#     requires=[ch.key],
#     retrieve_if_name_exists=True,
# )
# print(f'fuel: {sums[1] / NUM_SAMPLES}')
#
# ch = client.channels.retrieve('PT-HE-201_calc')
# client.channels.create(
#     name='PT-HE-201_zeroed',
#     data_type=sy.DataType.FLOAT64,
#     expression=f'return get(\"PT-HE-201_calc\") - {sums[2] / NUM_SAMPLES} + 14.7',
#     requires=[ch.key],
#     retrieve_if_name_exists=True,
# )
# print(f'helium: {sums[2] / NUM_SAMPLES}')

for _, row in df.iterrows():
    datatype = sy.DataType.UINT64

    match row['Channel Type']:
        case 'f64':
            datatype = sy.DataType.FLOAT64
        case 'f32':
            datatype = sy.DataType.FLOAT32
        case 'u8':
            datatype = sy.DataType.UINT8
        case 'u16':
            datatype = sy.DataType.UINT16
        case 'u32':
            datatype = sy.DataType.UINT32
        case 'u64':
            datatype = sy.DataType.UINT64
        case 'i8':
            datatype = sy.DataType.INT8
        case 'i16':
            datatype = sy.DataType.INT16
        case 'i32':
            datatype = sy.DataType.INT32
        case 'i64':
            datatype = sy.DataType.INT64
        case 'str':
            datatype = sy.DataType.STRING
        case _:
            raise Exception("fuck")

    time_ch = client.channels.create(
        name=f'{row['Name']}_time',
        data_type=sy.DataType.TIMESTAMP,
        is_index=True,
        retrieve_if_name_exists=True,
    )

    ch = client.channels.create(
        name=str(row['Name']),
        data_type=datatype,
        index=time_ch.key,
        retrieve_if_name_exists=True,
    )
    log_msg = f' Added channel: {row['Name']:<24}\t{row['Channel Type']}'

    if not bool(pd.isna(row['Slope'])):
        expr = f'return (((get(\"{row['Name']}\") * {constants.ADC_V_SLOPE}) - {constants.ADC_V_OFFSET}) * {row['Slope']}) + {row['Offset']}'
        ch = client.channels.create(
            name=ChannelName(f'{row['Name']}_calc'),
            data_type=datatype,
            expression=expr,
            requires=ChannelKeys([ch.key]),
            retrieve_if_name_exists=bool(True),
        )
        log_msg += f'\n\tCalculated:    {expr}'
    log.info(log_msg)

command_channels = [
    constants.FUEL_SOLENOID_NAME + '-CMD',
    constants.LOX_SOLENOID_NAME + '-CMD',
    constants.BB_OPEN_CHANNEL_NAME,
    constants.BB_ISO_CHANNEL_NAME,
    constants.BB_REG_CHANNEL_NAME,
    constants.BB_FU_UPPER_SETP_NAME,
    constants.BB_OX_UPPER_SETP_NAME,
    constants.BB_FU_LOWER_SETP_NAME,
    constants.BB_OX_LOWER_SETP_NAME,
    constants.BB_FU_UPPER_REDLINE_NAME,
    constants.BB_OX_UPPER_REDLINE_NAME,
    constants.BB_FU_LOWER_REDLINE_NAME,
    constants.BB_OX_LOWER_REDLINE_NAME,
]

for ch in command_channels:
    client.channels.create(
        name=ch,
        data_type=sy.DataType.UINT8,
        retrieve_if_name_exists=bool(True),
        virtual=bool(True),
    )
    log.info(f' Added command channel: {ch}\t')

client.close()
