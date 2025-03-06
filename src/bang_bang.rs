use crate::daq::SensorPacket;
use crate::{config, daq, inst_to_unix_micros};

use std::sync::{Arc, RwLock, mpsc};
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

    tx: mpsc::Sender<daq::SensorPacket>,
    usp_sensor_id: u64,
    lsp_sensor_id: u64,
    valve_sensor_id: u64,
}

impl System {
    pub fn new(
        config: Arc<RwLock<SystemConfig>>,
        pressure: Arc<RwLock<u64>>,
        gpio: Lines<Output>,
        tx: mpsc::Sender<daq::SensorPacket>,
        usp_sensor_id: u64,
        lsp_sensor_id: u64,
        valve_sensor_id: u64,
    ) -> Self {
        System {
            config,
            pressure,
            gpio,
            state: SystemState::new(),
            tx,
            usp_sensor_id,
            lsp_sensor_id,
            valve_sensor_id,
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
        let valve_pos = self.state.get_position();
        match valve_pos {
            ValvePos::Open => self.gpio.set_values(&[true]).unwrap(),
            ValvePos::Closed => self.gpio.set_values(&[false]).unwrap(),
        }

        let timestamp = inst_to_unix_micros(Instant::now());
        self.tx
            .send(SensorPacket {
                timestamp,
                sensor_id: self.usp_sensor_id,
                data: config.upper_setpoint,
            })
            .unwrap();
        self.tx
            .send(SensorPacket {
                timestamp,
                sensor_id: self.lsp_sensor_id,
                data: config.lower_setpoint,
            })
            .unwrap();
        self.tx
            .send(SensorPacket {
                timestamp,
                sensor_id: self.valve_sensor_id,
                data: match valve_pos {
                    ValvePos::Open => 1,
                    ValvePos::Closed => 0,
                },
            })
            .unwrap();
    }
}

pub fn start_bang_bang(
    fu_pressure: Arc<RwLock<u64>>,
    ox_pressure: Arc<RwLock<u64>>,
    tx: mpsc::Sender<daq::SensorPacket>,
) -> (Arc<RwLock<SystemConfig>>, Arc<RwLock<SystemConfig>>) {
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

    let fu_gpio = chip
        .request_lines(Options::output([config::BB_FU_GPIO_PIN]).consumer("bb_fu"))
        .unwrap();
    let ox_gpio = chip
        .request_lines(Options::output([config::BB_OX_GPIO_PIN]).consumer("bb_ox"))
        .unwrap();

    let fu_system = System::new(
        fu_config.clone(),
        fu_pressure.clone(),
        fu_gpio,
        tx.clone(),
        config::FU_USP_SENSOR_ID,
        config::FU_LSP_SENSOR_ID,
        config::FU_VALVE_SENSOR_ID,
    );
    let ox_system = System::new(
        ox_config.clone(),
        ox_pressure.clone(),
        ox_gpio,
        tx,
        config::OX_USP_SENSOR_ID,
        config::OX_LSP_SENSOR_ID,
        config::OX_VALVE_SENSOR_ID,
    );

    thread::spawn(move || {
        bang_bang_controller([fu_system, ox_system]);
    });

    (fu_config, ox_config)
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
