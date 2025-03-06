use core::{mem, slice};
use std::fs;
use std::io::Write;
use std::path::Path;
use std::sync::mpsc;
use std::thread;
use std::time::{SystemTime, UNIX_EPOCH};

use crate::{config, daq};

pub fn start_data_writer(rx: mpsc::Receiver<daq::SensorPacket>) {
    let start = SystemTime::now();
    let unix_secs = start
        .duration_since(UNIX_EPOCH)
        .expect("time went backwards")
        .as_secs();

    let data_dir = Path::new(config::DATA_DIR);
    fs::create_dir_all(data_dir).unwrap();

    let filename = format!("data_{}.bin", unix_secs);
    let file_path = data_dir.join(filename);

    if file_path.exists() {
        panic!("File already exists: {:?}", file_path);
    }

    let symlink_path = data_dir.join("latest.bin");
    if symlink_path.exists() {
        fs::remove_file(&symlink_path).unwrap();
    }
    std::os::unix::fs::symlink(&file_path, &symlink_path).unwrap();

    let mut file = fs::File::create(file_path).unwrap();
    thread::spawn(move || {
        let mut i: usize = 0;
        loop {
            let packet = rx.recv().unwrap();
            let data: &[u8] = unsafe {
                slice::from_raw_parts(
                    (&packet as *const daq::SensorPacket) as *const u8,
                    mem::size_of::<daq::SensorPacket>(),
                )
            };

            file.write_all(data).unwrap();

            // flush every so often
            if (i % 256) == 0 {
                file.flush().unwrap();
            }

            i += 1;
        }
    });
}
