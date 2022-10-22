use std::fmt::Debug;
use std::fmt::Display;

use anyhow::{bail, Result};
use hidapi::{DeviceInfo, HidApi, HidDevice, HidResult};

const     XAP_USAGE_PAGE: u16   = 0xFF51;
const     XAP_USAGE     : u16   = 0x0058;
pub const XAP_REPORT_LEN: usize = 64;

pub(crate) struct XAPClient {
    hid: HidApi,
}

pub(crate) struct XAPDevice {
    info: DeviceInfo,
    device: HidDevice,
}

pub(crate) type XapReportT = [u8; XAP_REPORT_LEN];
pub(crate) struct XAPReport {
    raw_data: XapReportT,
    from_kb: bool,
}


impl XAPClient {
    pub fn new() -> Result<Self> {
        Ok(XAPClient {
            hid: HidApi::new()?,
        })
    }

    pub fn get_first_xap_device(&mut self) -> Result<XAPDevice> {
        self.hid.refresh_devices()?;

        match self
            .hid
            .device_list()
            .find(|info| info.usage_page() == XAP_USAGE_PAGE && info.usage() == XAP_USAGE) {
                Some(info) => Ok(XAPDevice {
                    info: info.clone(),
                    device: info.open_device(&self.hid)?,
                }),
                None => bail!("No XAP compatible device found!"),
            }
    }
}


impl Debug for XAPDevice {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self}")
    }
}

impl Display for XAPDevice {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "VID: {:04x}, PID: {:04x}, Serial: {}, Product name: {}, Manufacturer: {}",
            self.info.vendor_id(),
            self.info.product_id(),
            match self.info.serial_number() {
                Some(s) => s,
                _ => "<COULD NOT FETCH>",
            },
            match self.info.product_string() {
                Some(s) => s,
                _ => "<COULD NOT FETCH>",
            },
            match self.info.manufacturer_string() {
                Some(s) => s,
                _ => "<COULD_NOT_FETCH>",
            }
        )
    }
}

impl XAPDevice {
    pub fn write(&self, data: &[u8]) -> HidResult<usize>{
        self.device.write(&data)
    }

    pub fn read(&self, buf: &mut [u8]) -> HidResult<usize> {
        self.device.read(buf)
    }

    pub fn read_timeout(&self, buf: &mut [u8], timeout: i32) -> HidResult<usize> {
        self.device.read_timeout(buf, timeout)
    }
}

impl Debug for XAPReport {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self}")
    }
}

impl Display for XAPReport {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "Report: {}",
            self.get_report()
        )
    }
}

impl XAPReport {
    pub fn new() -> Self {
        Self {
            raw_data: [0; XAP_REPORT_LEN],
            from_kb: false 
        }
    }

    pub fn from_data(data: XapReportT) -> Self {
        Self { 
            raw_data: data,
            from_kb: true 
        }
    }

    fn get_token(&self) -> u16 {
        return ((self.raw_data[1] as u16) << 8) | self.raw_data[0] as u16;
    }

    fn get_flags(&self) -> u8 {
        if self.from_kb {
            return self.raw_data[2];
        }

        return u8::MAX;
    }

    fn get_payload_len(&self) -> usize {
        let mut index = 2;
        if self.from_kb {
            index = 3;
        }

        return self.raw_data[index] as usize;
    }

    fn get_payload(&self) -> &[u8] {
        let len   = self.get_payload_len();
        let mut start = 3;
        if self.from_kb {
            start = 4;
        }

        return &self.raw_data[start .. start+len];
    }

    pub fn get_report(&self) -> String {
        if self.from_kb {
            return format!(
                "||Token: {} | Flags: {} | Length: {} | Payload: {:?}||",
                self.get_token(),
                self.get_flags(),
                self.get_payload_len(),
                self.get_payload()
            )
        }

        return format!(
            "||Token: {} | Length: {} | Payload: {:?}||",
            self.get_token(),
            self.get_payload_len(),
            self.get_payload()
        )
    } 
}