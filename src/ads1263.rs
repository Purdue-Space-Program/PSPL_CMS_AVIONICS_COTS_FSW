use anyhow::anyhow;

use core::time::Duration;
use gpiod::{Input, Lines, Options, Output};
use spidev::{SpiModeFlags, Spidev, SpidevOptions, SpidevTransfer};
use std::io;
use std::io::prelude::*;
use std::thread::sleep;
use std::time::Instant;

#[derive(Copy, Clone, Debug)]
pub enum Reg {
    /*Register address, followed by reset the default values */
    Id = 0,    // xxh
    Power,     // 11h
    Interface, // 05h
    Mode0,     // 00h
    Mode1,     // 80h
    Mode2,     // 04h
    InpMux,    // 01h
    OfCal0,    // 00h
    OfCal1,    // 00h
    OfCal2,    // 00h
    FsCal0,    // 00h
    FsCal1,    // 00h
    FsCal2,    // 40h
    IdacMux,   // BBh
    IdacMag,   // 00h
    RefMux,    // 00h
    TdacP,     // 00h
    TdacN,     // 00h
    GpioCon,   // 00h
    GpioDir,   // 00h
    GpioData,  // 00h
    Adc2Cfg,   // 00h
    Adc2Mux,   // 01h
    Adc2Ofc0,  // 00h
    Adc2Ofc1,  // 00h
    Adc2Fsc0,  // 00h
    Adc2Fsc1,  // 40h
}

#[repr(u8)]
#[derive(Copy, Clone, Debug)]
pub enum Cmd {
    Reset = 0x06,   // Reset the ADC, 0000 011x (06h or 07h)
    Start1 = 0x08,  // Start ADC1 conversions, 0000 100x (08h or 09h)
    Stop1 = 0x0A,   // Stop ADC1 conversions, 0000 101x (0Ah or 0Bh)
    Start2 = 0x0C,  // Start ADC2 conversions, 0000 110x (0Ch or 0Dh)
    Stop2 = 0x0E,   // Stop ADC2 conversions, 0000 111x (0Eh or 0Fh)
    RData1 = 0x12,  // Read ADC1 data, 0001 001x (12h or 13h)
    RData2 = 0x14,  // Read ADC2 data, 0001 010x (14h or 15h)
    SyoCal1 = 0x16, // ADC1 system offset calibration, 0001 0110 (16h)
    SygCal1 = 0x17, // ADC1 system gain calibration, 0001 0111 (17h)
    SfoCal1 = 0x19, // ADC1 self offset calibration, 0001 1001 (19h)
    SyoCal2 = 0x1B, // ADC2 system offset calibration, 0001 1011 (1Bh)
    SygCal2 = 0x1C, // ADC2 system gain calibration, 0001 1100 (1Ch)
    SfoCal2 = 0x1E, // ADC2 self offset calibration, 0001 1110 (1Eh)
    RReg(Reg),      // Read registers 001r rrrr (20h+000r rrrr)
    WReg(Reg),      // Write registers 010r rrrr (40h+000r rrrr)
}

impl Into<u8> for Cmd {
    fn into(self) -> u8 {
        match self {
            Cmd::Reset => 0x06,
            Cmd::Start1 => 0x08,
            Cmd::Stop1 => 0x0A,
            Cmd::Start2 => 0x0C,
            Cmd::Stop2 => 0x0E,
            Cmd::RData1 => 0x12,
            Cmd::RData2 => 0x14,
            Cmd::SyoCal1 => 0x16,
            Cmd::SygCal1 => 0x17,
            Cmd::SfoCal1 => 0x19,
            Cmd::SyoCal2 => 0x1B,
            Cmd::SygCal2 => 0x1C,
            Cmd::SfoCal2 => 0x1E,
            Cmd::RReg(reg) => 0x20 | ((reg as u8) & 0x1F),
            Cmd::WReg(reg) => 0x40 | ((reg as u8) & 0x1F),
        }
    }
}

pub enum Gain {
    Gain1 = 0,  /*GAIN  1 */
    Gain2 = 1,  /*GAIN  2 */
    Gain4 = 2,  /*GAIN  4 */
    Gain8 = 3,  /*GAIN  8 */
    Gain16 = 4, /*GAIN  16 */
    Gain32 = 5, /*GAIN  32 */
    Gain64 = 6, /*GAIN  64 */
}

pub enum DataRate {
    Sps2_5 = 0,
    Sps5,
    Sps10,
    Sps16_6,
    Sps20,
    Sps50,
    Sps60,
    Sps100,
    Sps400,
    Sps1200,
    Sps2400,
    Sps4800,
    Sps7200,
    Sps14400,
    Sps19200,
    Sps38400,
}

pub enum Delay {
    Delay0s = 0,
    Delay8_7us,
    Delay17us,
    Delay35us,
    Delay169us,
    Delay139us,
    Delay278us,
    Delay555us,
    Delay1_1ms,
    Delay2_2ms,
    Delay4_4ms,
    Delay8_8ms,
}

pub enum Adc2DataRate {
    Sps10 = 0,
    Sps100,
    Sps400,
    Sps800,
}

pub enum Adc2Gain {
    Gain1 = 0,
    Gain2,
    Gain4,
    Gain8,
    Gain16,
    Gain32,
    Gain64,
    Gain128,
}

pub struct Ads1263 {
    spi: Spidev,
    drdy: Lines<Input>,
    cs: Lines<Output>,
    reset: Lines<Output>,
}

pub const VCOM_CHANNEL: u8 = 10;

impl Ads1263 {
    pub fn new(spi_dev: &str, baud: u32, drdy_pin: u32, cs_pin: u32, reset_pin: u32) -> Self {
        let mut spi = Spidev::open(spi_dev).expect("Failed to open SPI device");
        let spi_opts = SpidevOptions::new()
            .bits_per_word(8)
            .max_speed_hz(baud)
            .mode(SpiModeFlags::SPI_MODE_1)
            .build();
        spi.configure(&spi_opts)
            .expect("Failed to configure SPI device");

        let chip = gpiod::Chip::new("/dev/gpiochip0").expect("Failed to open GPIO chip");

        let drdy_opts = Options::input([drdy_pin])
            .consumer("ads1263_drdy")
            .edge(gpiod::EdgeDetect::Falling);
        let drdy = chip
            .request_lines(drdy_opts)
            .expect("Failed to request DRDY line");

        let cs_opts = Options::output([cs_pin]).consumer("ads1263_cs");
        let cs = chip
            .request_lines(cs_opts)
            .expect("Failed to request CS line");

        let reset_opts = Options::output([reset_pin]).consumer("ads1263_reset");
        let reset = chip
            .request_lines(reset_opts)
            .expect("Failed to request RESET line");

        Self {
            spi,
            drdy,
            cs,
            reset,
        }
    }

    fn cs(&mut self, value: bool) {
        self.cs.set_values([!value]).unwrap();
    }

    pub fn reset(&mut self) {
        self.reset.set_values([true]).unwrap();
        sleep(Duration::from_millis(300));
        self.reset.set_values([false]).unwrap();
        sleep(Duration::from_millis(300));
        self.reset.set_values([true]).unwrap();
        sleep(Duration::from_millis(300));
    }

    pub fn write_cmd(&mut self, cmd: Cmd) {
        self.cs(false);
        self.spi.write(&[cmd.into()]).unwrap();
        self.cs(true);
    }

    pub fn wreg(&mut self, reg: Reg, data: u8) {
        self.cs(false);
        self.spi
            .write(&[Cmd::WReg(reg).into(), 0x00, data])
            .unwrap();
        self.cs(true);
    }

    pub fn rreg(&mut self, reg: Reg) -> u8 {
        self.cs(false);
        let tx = [Cmd::RReg(reg).into(), 0x00, 0x00];
        let mut rx = [0; 3];
        let mut xfer = SpidevTransfer::read_write(&tx, &mut rx);
        self.spi.transfer(&mut xfer).unwrap();
        self.cs(true);

        rx[2]
    }

    pub fn wreg_checked(&mut self, reg: Reg, data: u8) -> anyhow::Result<()> {
        self.wreg(reg, data);
        sleep(Duration::from_millis(1));
        let ret = self.rreg(reg);

        if self.checksum(data as u32, ret) {
            Ok(())
        } else {
            Err(anyhow!("wreg fail: expected {}, got {}", data, ret))
        }
    }

    fn checksum(&self, mut val: u32, crc: u8) -> bool {
        let mut sum = 0;
        let mask = 0xff;

        while val != 0 {
            sum += val & mask;
            val >>= 8;
        }

        sum += 0x9b;
        return (sum as u8) ^ crc == 0x00;
    }

    fn wait_drdy(&mut self) -> Duration {
        let ev = self.drdy.read_event().unwrap();
        ev.time
    }

    pub fn chip_id(&mut self) -> u8 {
        let ret = self.rreg(Reg::Id);
        ret >> 5
    }

    pub fn config_adc1(&mut self, gain: Gain, drate: DataRate, delay: Delay) {
        let mode2 = 0x80 | ((gain as u8) << 4) | (drate as u8);
        self.wreg_checked(Reg::Mode2, mode2)
            .expect("Failed to write Mode2 register");

        let refmux = 0x24; // 0x00:+-2.5V as REF, 0x24:VDD,VSS as REF
        self.wreg_checked(Reg::RefMux, refmux)
            .expect("Failed to write RefMux register");

        let mode0 = delay;
        self.wreg_checked(Reg::Mode0, mode0 as u8)
            .expect("Failed to write Mode0 register");

        let mode1 = 0x84; // Digital Filter; 0x84:FIR, 0x64:Sinc4, 0x44:Sinc3,
        // 0x24:Sinc2, 0x04:Sinc1

        self.wreg_checked(Reg::Mode1, mode1)
            .expect("Failed to write Mode1 register");
    }

    pub fn config_adc2(&mut self, gain: Adc2Gain, drate: Adc2DataRate, delay: Delay) {
        let adc2cfg = 0x20 | ((drate as u8) << 6) | gain as u8;
        self.wreg_checked(Reg::Adc2Cfg, adc2cfg)
            .expect("Failed to write Adc2Cfg register");

        let mut0 = 0x00;
        self.wreg_checked(Reg::Adc2Mux, mut0)
            .expect("Failed to write Adc2Mux register");
    }

    pub fn init_adc1(&mut self, drate: DataRate) {
        self.reset();
        if self.chip_id() != 0x01 {
            panic!("Invalid chip ID");
        }

        self.write_cmd(Cmd::Stop1);
        self.config_adc1(Gain::Gain1, drate, Delay::Delay35us);
        self.write_cmd(Cmd::Start1);
    }

    pub fn init_adc2(&mut self, drate: Adc2DataRate) {
        self.reset();
        if self.chip_id() != 0x01 {
            panic!("Invalid chip ID");
        }

        self.write_cmd(Cmd::Stop2);
        self.config_adc2(Adc2Gain::Gain1, drate, Delay::Delay35us);
        // TODO: investigate this
        // self.write_cmd(Cmd::Start2); // this isn't in the reference code, not sure why
    }

    pub fn set_channel(&mut self, pos: u8, neg: u8) {
        if pos >= 10 || neg >= 10 {
            panic!("Invalid channel");
        }

        let inpmux = (pos << 4) | neg;
        self.wreg_checked(Reg::InpMux, inpmux)
            .expect("Failed to write InpMux register");
    }

    pub fn set_channel_adc2(&mut self, pos: u8, neg: u8) {
        if pos >= 10 || neg >= 10 {
            panic!("Invalid channel");
        }

        let inpmux = (pos << 4) | neg;
        self.wreg_checked(Reg::Adc2Mux, inpmux)
            .expect("Failed to write Adc2Mux register");
    }

    pub fn read_adc1(&mut self) -> u32 {
        // uses direct method, not command method

        // assert that DRDY is low
        if self.drdy.get_values([false]).unwrap()[0] {
            panic!("DRDY is not low");
        }

        let mut rx = [0; 6];
        self.cs(false);
        let read = self.spi.read(&mut rx).unwrap();
        self.cs(true);

        let status = rx[0];
        if status & (1 << 6) != 0 {
            panic!("ADC1 read failed");
        }
        let data = ((rx[1] as u32) << 24)
            | ((rx[2] as u32) << 16)
            | ((rx[3] as u32) << 8)
            | (rx[4] as u32);

        let crc = rx[5];
        if !self.checksum(data, crc) {
            panic!("ADC1 Checksum failed");
        }
        data
    }

    pub fn read_adc2(&mut self) -> u32 {
        // can't use direct method for ADC2, must use command method
        if self.drdy.get_values([false]).unwrap()[0] {
            panic!("DRDY is not low");
        }

        let tx = [Cmd::RData2.into(), 0, 0, 0, 0, 0, 0];
        let mut rx = [0; 7];

        self.wait_drdy();
        self.cs(false);
        let mut xfer = SpidevTransfer::read_write(&tx, &mut rx);
        self.spi.transfer(&mut xfer).unwrap();
        self.cs(true);
        let status = rx[1];
        if status & (1 << 7) != 0 {
            panic!("ADC2 read failed");
        }
        let data = ((rx[2] as u32) << 16) | ((rx[3] as u32) << 8) | (rx[4] as u32);
        let crc = rx[6];
        if !self.checksum(data, crc) {
            panic!("ADC2 Checksum failed");
        }
        data
    }

    pub fn read_channel_adc1(&mut self, pos: u8, neg: u8) -> (Duration, u32) {
        self.set_channel(pos, neg);
        let res = self.wait_drdy();
        (res, self.read_adc1())
    }
}
