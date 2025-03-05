use std::time::Duration;

// minimum time to wait before a valve state change
pub const SOLENOID_MIN_SWITCH_TIME: Duration = Duration::from_millis(50);

// bang-bang control loop period
pub const BANG_BANG_LOOP_PERIOD: Duration = Duration::from_millis(10); // 100 Hz

// GPIO pin numbers
pub const BB_FU_GPIO_PIN: u32 = 20;
pub const BB_OX_GPIO_PIN: u32 = 21;
