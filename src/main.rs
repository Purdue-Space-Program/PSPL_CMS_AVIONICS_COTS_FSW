mod config; // global constants (pin numbers, etc.)

mod ads1263;
mod bang_bang;
mod daq;
mod data_writer;

use std::panic;
use std::process;
use std::sync::{Arc, RwLock};

fn main() {
    let orig_hook = panic::take_hook();
    panic::set_hook(Box::new(move |panic_info| {
        // so that panic exists the process on any thread
        orig_hook(panic_info);
        process::exit(1);
    }));

    let fu_pressure = Arc::new(RwLock::new(0));
    let ox_pressure = Arc::new(RwLock::new(0));

    let daq_rx = daq::start_daq(fu_pressure.clone(), ox_pressure.clone());
    data_writer::start_data_writer(daq_rx);

    let (fu_config, ox_config) = bang_bang::start_bang_bang(fu_pressure, ox_pressure);
}
