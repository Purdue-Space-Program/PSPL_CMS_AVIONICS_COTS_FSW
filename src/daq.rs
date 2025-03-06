use std::sync::{Arc, RwLock, mpsc};
use std::thread;

use crate::ads1263;
use crate::config;

#[repr(C)]
pub struct SensorPacket {
    pub timestamp: u64, // microseconds
    pub data: u64,
    pub sensor_id: u64,
}

pub fn start_daq(
    fu_pressure: Arc<RwLock<u64>>,
    ox_pressure: Arc<RwLock<u64>>,
) -> mpsc::Receiver<SensorPacket> {
    let mut adc = ads1263::Ads1263::new(
        config::ADC_SPI_DEVICE,
        config::ADC_SPI_SPEED,
        config::ADC_DRDY_PIN,
        config::ADC_CS_PIN,
        config::ADC_RST_PIN,
    );

    adc.init_adc1(config::ADC_SAMPLE_RATE);
    let (tx, rx) = mpsc::channel::<SensorPacket>();

    thread::spawn(move || {
        loop {
            for i in 0..config::NUM_AI_CHANNELS {
                let sid = config::ADC_SENSOR_IDS[i];
                let ch = config::ADC_CHANNELS[i];
                let (ts, data) = adc.read_channel_adc1(ch.0, ch.1);

                let packet = SensorPacket {
                    timestamp: ts.as_micros() as u64,
                    data: data as u64,
                    sensor_id: sid,
                };

                match ch {
                    config::CHANNEL_PT_FU => {
                        let mut p = fu_pressure.write().unwrap();
                        *p = data as u64;
                    }
                    config::CHANNEL_PT_OX => {
                        let mut p = ox_pressure.write().unwrap();
                        *p = data as u64;
                    }
                    _ => {}
                }

                tx.send(packet).unwrap();
            }
        }
    });

    rx
}
