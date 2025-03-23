import synnax as sy
import pandas as pd
import command as cmd

adc_to_v_slope  =  0.00000000235714724017
adc_to_v_offset = -0.01390133824020600000

client = sy.Synnax(
    host='128.46.118.59',
    port=9090,
    username='Bill',
    password='Bill',
)

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx', 'channels')

for _, row in df.iterrows():
    datatype = sy.DataType.UINT64

    match row["Channel Type"]:
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

    if not pd.isna(row['Slope']):
        expr = f'return (((get(\"{row['Name']}\") * {adc_to_v_slope}) - {adc_to_v_offset}) * {row['Slope']}) + {row['Offset']}'
        ch = client.channels.create(
            name=f'{row['Name']}_calc',
            data_type=datatype,
            expression=expr,
            requires=[ch.key],
            retrieve_if_name_exists=True,
        )
        print(expr)

bb_fu_cmd_chan = client.channels.create(
    name='bb_fu_cmd',
    data_type=sy.DataType.UINT8,
    retrieve_if_name_exists=True,
    virtual=True,
)

bb_ox_cmd_chan = client.channels.create(
    name='bb_ox_cmd',
    data_type=sy.DataType.UINT8,
    retrieve_if_name_exists=True,
    virtual=True,
)

if __name__ == '__main__':
    with client.open_streamer(['bb_fu_cmd', 'bb_ox_cmd']) as s:
        for frame in s:
            for f in frame['bb_fu_cmd']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_FU_STATE_OPEN.name)
                if f == 0:
                    cmd.send_command(cmd.Command.SET_FU_STATE_ISOLATE.name)
            for f in frame['bb_ox_cmd']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_OX_STATE_OPEN.name)
                if f == 0:
                    cmd.send_command(cmd.Command.SET_OX_STATE_ISOLATE.name)
