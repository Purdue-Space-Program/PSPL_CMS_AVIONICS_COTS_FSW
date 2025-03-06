mod config; // global constants (pin numbers, etc.)

mod ads1263;
mod daq;
mod bang_bang;

use std::panic;
use std::process;

fn main() {
    let orig_hook = panic::take_hook();
    panic::set_hook(Box::new(move |panic_info| {
        // so that panic exists the process on any thread
        orig_hook(panic_info);
        process::exit(1);
    }));

    let (fu_config, ox_config, fu_pressure, ox_pressure) = bang_bang::start_bang_bang();
    let daq_rx = daq::start_daq(fu_pressure, ox_pressure);
}
