mod config; // global constants (pin numbers, etc.)

mod ads1263;
mod bang_bang;
mod command_server;
mod daq;
mod data_writer;

use std::mem::MaybeUninit;
use std::panic;
use std::process;
use std::sync::{Arc, RwLock};
use std::time::Instant;
use std::time::SystemTime;

static mut START_SYS: MaybeUninit<SystemTime> = MaybeUninit::uninit();
static mut START_INST: MaybeUninit<Instant> = MaybeUninit::uninit();

pub fn inst_to_unix_micros(inst: Instant) -> u64 {
    let start_inst = unsafe { START_INST.assume_init() };
    let start_sys = unsafe { START_SYS.assume_init() };

    let from_start = inst.duration_since(start_inst);
    let sys_now = start_sys.checked_add(from_start).unwrap();
    let unix_now = sys_now.duration_since(SystemTime::UNIX_EPOCH).unwrap();
    unix_now.as_micros() as u64
}

fn main() {
    unsafe {
        START_SYS = MaybeUninit::new(SystemTime::now());
        START_INST = MaybeUninit::new(Instant::now());
    }

    let orig_hook = panic::take_hook();
    panic::set_hook(Box::new(move |panic_info| {
        // so that panic exists the process on any thread
        orig_hook(panic_info);
        process::exit(1);
    }));

    let fu_pressure = Arc::new(RwLock::new(0));
    let ox_pressure = Arc::new(RwLock::new(0));

    let (tx, rx) = daq::start_daq(fu_pressure.clone(), ox_pressure.clone());
    data_writer::start_data_writer(rx);
    let (fu_config, ox_config) = bang_bang::start_bang_bang(fu_pressure, ox_pressure, tx.clone());
    command_server::start_command_server(fu_config, ox_config);
}
