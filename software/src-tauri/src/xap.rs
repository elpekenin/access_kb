use std::fmt::Debug;
use std::fmt::Display;

use anyhow::{bail, Result};
use hidapi::{DeviceInfo, HidApi, HidDevice, HidResult};

const     XAP_USAGE_PAGE: u16   = 0xFF51;
const     XAP_USAGE     : u16   = 0x0058;
pub const XAP_REPORT_LEN: usize = 64;

// ============================================================================
pub(crate) struct XAPClient {
    hid: HidApi,
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

// ============================================================================
pub(crate) struct XAPDevice {
    info: DeviceInfo,
    device: HidDevice,
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

    pub fn read_timeout(&self, buf: &mut [u8], timeout: i32) -> HidResult<usize> {
        self.device.read_timeout(buf, timeout)
    }
}

// ============================================================================
pub(crate) type XapReportT = [u8; XAP_REPORT_LEN];
pub(crate) struct XAPReport {
    raw_data: XapReportT,
    from_kb: bool,
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
            "{} |- Token: {}{}, Length: {}, Payload: {:?} -|",
            match self.from_kb {
                true  => "Received",
                false => "Sent    "
            },
            self.get_token(),
            match self.from_kb {
                true  => format!(", Flags: {}", self.get_flags()),
                false => String::new()
            },
            self.get_payload_len(),
            self.get_payload()
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

    pub fn get_bytes(&mut self) -> &mut [u8] {
        &mut self.raw_data
    }

    pub fn get_flags(&self) -> u8 {
        if self.from_kb {
            return self.raw_data[2];
        }

        u8::MAX
    }

    pub fn get_payload(&self) -> &[u8] {
        let len   = self.get_payload_len();
        let mut start = 3;
        if self.from_kb {
            start = 4;
        }

        &self.raw_data[start .. start+len]
    }
    
    pub fn get_payload_len(&self) -> usize {
        let mut index = 2;
        if self.from_kb {
            index = 3;
        }

        self.raw_data[index] as usize
    }
    
    pub fn get_token(&self) -> u16 {
        ((self.raw_data[1] as u16) << 8) | self.raw_data[0] as u16
    }

    pub fn set_bytes(&mut self, start: usize, data: &[u8]) {
        for i in 0..data.len() {
            self.raw_data[i+start] = data[i];
        }
    }

    pub fn set_from_kb(&mut self, from_kb: bool) {
        self.from_kb = from_kb;
    }
}