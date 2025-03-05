use crate::config;

use std::sync::{Arc, RwLock};
use std::thread;
use std::time::Instant;

use gpiod::{Lines, Options, Output};

// control state for a system
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum ControlState {
    Isolate,  // close valve
    Regulate, // regulate pressure
    Open,     // open valve
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum ValvePos {
    Open,
    Closed,
}

#[derive(Copy, Clone, Debug)]
pub struct SystemConfig {
    pub control_state: ControlState, // control state
    pub upper_setpoint: u64,         // upper setpoint pressure
    pub lower_setpoint: u64,         // lower setpoint pressure
}

// state for a single bang bang system (ox/fu)
struct SystemState {
    position: ValvePos, // current valve position
    last_set: Instant,  // time when this position was set
}

impl SystemState {
    pub fn new() -> Self {
        SystemState {
            position: ValvePos::Closed,
            last_set: Instant::now(),
        }
    }

    pub fn update(&mut self, desired_pos: ValvePos) {
        if desired_pos != self.position
            && self.last_set.elapsed() > config::SOLENOID_MIN_SWITCH_TIME
        {
            self.position = desired_pos;
            self.last_set = Instant::now();
        }
    }

    pub fn get_position(&self) -> ValvePos {
        self.position
    }
}

pub struct System {
    config: Arc<RwLock<SystemConfig>>,
    pressure: Arc<RwLock<u64>>,
    gpio: Lines<Output>,
    state: SystemState,
}

impl System {
    pub fn new(
        config: Arc<RwLock<SystemConfig>>,
        pressure: Arc<RwLock<u64>>,
        gpio: Lines<Output>,
    ) -> Self {
        System {
            config,
            pressure,
            gpio,
            state: SystemState::new(),
        }
    }

    fn update(&mut self) {
        let config = *self.config.read().unwrap();
        let pressure = *self.pressure.read().unwrap();

        match config.control_state {
            ControlState::Isolate => {
                self.state.update(ValvePos::Closed);
            }
            ControlState::Regulate => {
                if pressure > config.upper_setpoint {
                    self.state.update(ValvePos::Closed);
                } else if pressure < config.lower_setpoint {
                    self.state.update(ValvePos::Open);
                }
            }
            ControlState::Open => {
                self.state.update(ValvePos::Open);
            }
        }

        // valves are normally closed
        match self.state.get_position() {
            ValvePos::Open => self.gpio.set_values(&[true]).unwrap(),
            ValvePos::Closed => self.gpio.set_values(&[false]).unwrap(),
        }
    }
}

pub fn start_bang_bang() -> (
    Arc<RwLock<SystemConfig>>,
    Arc<RwLock<SystemConfig>>,
    Arc<RwLock<u64>>,
    Arc<RwLock<u64>>,
) {
    let chip = gpiod::Chip::new("/dev/gpiochip0").expect("Failed to open GPIO chip");

    // bang-bang setup
    let fu_config = SystemConfig {
        control_state: ControlState::Isolate,
        upper_setpoint: 0,
        lower_setpoint: 0,
    };
    let ox_config = SystemConfig {
        control_state: ControlState::Isolate,
        upper_setpoint: 0,
        lower_setpoint: 0,
    };

    let fu_config = Arc::new(RwLock::new(fu_config));
    let ox_config = Arc::new(RwLock::new(ox_config));

    let fu_pressure = Arc::new(RwLock::new(0));
    let ox_pressure = Arc::new(RwLock::new(0));

    let fu_gpio = chip
        .request_lines(Options::output([config::BB_FU_GPIO_PIN]).consumer("bb_fu"))
        .unwrap();
    let ox_gpio = chip
        .request_lines(Options::output([config::BB_OX_GPIO_PIN]).consumer("bb_ox"))
        .unwrap();

    let fu_system = System::new(fu_config.clone(), fu_pressure.clone(), fu_gpio);
    let ox_system = System::new(ox_config.clone(), ox_pressure.clone(), ox_gpio);

    thread::spawn(move || {
        bang_bang_controller([fu_system, ox_system]);
    });

    (fu_config, ox_config, fu_pressure, ox_pressure)
}

fn bang_bang_controller<const N: usize>(mut systems: [System; N]) {
    let mut next_time = Instant::now();

    loop {
        for system in &mut systems {
            system.update();
        }

        next_time += config::BANG_BANG_LOOP_PERIOD;
        let now = Instant::now();
        if next_time > now {
            thread::sleep(next_time - now);
        } else {
            // behind schedule
            next_time = now;
        }
    }
}
