use std::time::Duration;

use crate::ads1263;

// minimum time to wait before a valve state change
pub const SOLENOID_MIN_SWITCH_TIME: Duration = Duration::from_millis(50);

// bang-bang control loop period
pub const BANG_BANG_LOOP_PERIOD: Duration = Duration::from_millis(10); // 100 Hz

// GPIO pin numbers
pub const BB_FU_GPIO_PIN: u32 = 20;
pub const BB_OX_GPIO_PIN: u32 = 21;

// AI Channel numbers, tuples of (+, -), differential inputs
pub const NUM_AI_CHANNELS: usize = 5;

pub const CHANNEL_PT_HE: (u8, u8) = (0, 1);
pub const CHANNEL_PT_FU: (u8, u8) = (2, 3);
pub const CHANNEL_PT_OX: (u8, u8) = (4, 5);
pub const CHANNEL_TC_0: (u8, u8) = (6, 7);
pub const CHANNEL_TC_1: (u8, u8) = (8, 9);

pub const ADC_CHANNELS: [(u8, u8); NUM_AI_CHANNELS] = [
    CHANNEL_PT_HE,
    CHANNEL_PT_FU,
    CHANNEL_PT_OX,
    CHANNEL_TC_0,
    CHANNEL_TC_1,
];

// ADS1263 configuration
pub const ADC_SPI_DEVICE: &str = "/dev/spidev0.0";
pub const ADC_SPI_SPEED: u32 = 500_000; // 500 kHz
pub const ADC_DRDY_PIN: u32 = 17;
pub const ADC_RST_PIN: u32 = 18;
pub const ADC_CS_PIN: u32 = 22;
pub const ADC_SAMPLE_RATE: ads1263::DataRate = ads1263::DataRate::Sps4800; // 4800 SPS, 960 per ch

// Data writer config
pub const DATA_DIR: &str = "/var/lib/psp_fsw";

// Sensor IDs
pub const ADC_SENSOR_IDS: [u64; NUM_AI_CHANNELS] = [0, 1, 2, 3, 4];
pub const OX_USP_SENSOR_ID: u64 = 5; // upper setpoint
pub const OX_LSP_SENSOR_ID: u64 = 6; // lower setpoint
pub const FU_USP_SENSOR_ID: u64 = 7; // upper setpoint
pub const FU_LSP_SENSOR_ID: u64 = 8; // lower setpoint
pub const OX_VALVE_SENSOR_ID: u64 = 9;
pub const FU_VALVE_SENSOR_ID: u64 = 10;
