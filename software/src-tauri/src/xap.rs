use std::fmt::Debug;
use std::fmt::Display;

use anyhow::{bail, Result};
use log::{info};
use hidapi::{DeviceInfo, HidApi, HidDevice};

const     XAP_USAGE_PAGE: u16   = 0xFF51;
const     XAP_USAGE     : u16   = 0x0058;
pub const XAP_REPORT_LEN: usize = 64;

// ===========================================================================
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

// ===========================================================================
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
    pub fn write(&self, report: &mut XAPReport) {
        info!("{}",
            match self.device.write(
                report.set_from_kb(0_u8)
                      .set_token()
                      .get_bytes()
            ) {
                Ok(_) => format!("Success {}", report),
                _     => format!("Error   {}", report)
            }
        )
    }

    pub fn read_timeout(&self, report: &mut XAPReport, timeout: i32) {
        info!("{}",
            match self.device.read_timeout(
                report.set_from_kb(1_u8)
                      .get_bytes(),
                timeout
            ) {
                Ok(_) => format!("Success {}", report),
                _     => format!("Error   {}", report)  //tried (not much) to proc this without success
            }
        )
    }
}

// ===========================================================================
use rand::Rng;
type XapReportT = [u8; XAP_REPORT_LEN];
pub(crate) struct XAPReport {
    raw_data: XapReportT,
    from_kb: u8,
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
            "{} >> Token: 0x{:04X} | {} | Length: {:02} | Payload: {:?}",
            match self.from_kb {
                1 => "receiving",
                0 => "sending  "
            },
            self.get_token(),
            match self.from_kb {
                1 => format!("Flags: {:08b}", self.get_flags()),
                0 => format!("               ")
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
            from_kb: 0
        }
    }

    pub fn from_route(route: &[u8]) -> Self {
        let len = route.len();
        let mut temp: XapReportT = [0; XAP_REPORT_LEN];

        //copy route into the payload section
        for i in 0..len {
            temp[i+3] = route[i];
        }

        //set payload length accordingly
        temp[2] = len as u8;

        Self {
            raw_data: temp,
            from_kb: 0
        }
    }

    pub fn get_bytes(&mut self) -> &mut [u8] {
        &mut self.raw_data
    }

    pub fn get_flags(&self) -> u8 {
        // NOTE: Weird behaviour for sent reports
        self.raw_data[2]
    }

    pub fn get_payload(&self) -> &[u8] {
        let len   = self.get_payload_len();
        let start = 3 + self.from_kb;

        &self.raw_data[start .. start+len]
    }

    pub fn get_payload_len(&self) -> usize {
        let mut index = 2 + self.from_kb;

        self.raw_data[index] as usize
    }

    pub fn get_token(&self) -> u16 {
        ((self.raw_data[0] as u16) << 8) | self.raw_data[1] as u16
    }

    // TODO: Common checker macro/function for `set_*` methods, and log that data wasn't changed because it was a received report

    pub fn set_bytes(&mut self, start: usize, data: &[u8]) -> &mut Self {
        if self.from_kb > 0{
            return self;
        }

        for i in 0..data.len() {
            self.raw_data[i+start] = data[i];
        }

        self
    }

    pub fn set_from_kb(&mut self, from_kb: u8) -> &mut Self{
        if self.from_kb > 0{
            return self;
        }

        self.from_kb = from_kb;

        self
    }

    pub fn set_token(&mut self) -> &mut Self {
        if self.from_kb > 0{
            return self;
        }

        let mut rng = rand::thread_rng();
        //according to the valid range 0x0100-0xFFFF defined on the spec
        self.raw_data[0] = rng.gen_range(0x01..0xFF);
        self.raw_data[1] = rng.gen_range(0x00..0xFF);

        self
    }

    //TODO: Implement `append_payload` to add values after routes
}

// ===========================================================================
pub(crate) struct XapRoute;
impl XapRoute {
    pub const XAP_VERSION_QUERY:                    &[u8] = &[0x00, 0x00];
    pub const XAP_CAPABILITIES_QUERY:               &[u8] = &[0x00, 0x01];
    pub const XAP_ENABLED_SUBSYSTEM_QUERY:          &[u8] = &[0x00, 0x02];
    pub const XAP_SECURE_STATUS:                    &[u8] = &[0x00, 0x03];
    pub const XAP_SECURE_UNLOCK:                    &[u8] = &[0x00, 0x04];
    pub const XAP_SECURE_LOCK:                      &[u8] = &[0x00, 0x05];
// ===========
    pub const QMK_VERSION_QUERY:                    &[u8] = &[0x01, 0x00];
    pub const QMK_CAPABILITIES_QUERY:               &[u8] = &[0x01, 0x01];
    pub const QMK_IDENTIFIERS:                      &[u8] = &[0x01, 0x02];
    pub const QMK_BOARD_MANUFACTURER:               &[u8] = &[0x01, 0x03];
    pub const QMK_PRODUCT_NAME:                     &[u8] = &[0x01, 0x04];
    pub const QMK_CONFIG_BLOB_LENGTH:               &[u8] = &[0x01, 0x05];
    pub const QMK_CONFIG_BLOB_CHUNK:                &[u8] = &[0x01, 0x06];
    pub const QMK_JUMP_TO_BOOTLOADER:               &[u8] = &[0x01, 0x07];
    pub const QMK_HARDWARE_IDENTIFIER:              &[u8] = &[0x01, 0x08];
    pub const QMK_REINITIALIZE_EEPROM:              &[u8] = &[0x01, 0x09];
// ===========
    pub const KEYBOARD:                             &[u8] = &[0x02, 0x00]; //reserved
// ===========
    pub const USER:                                 &[u8] = &[0x03, 0x00]; //reserved
// ===========
    pub const KEYMAP:                               &[u8] = &[0x04, 0x00]; //unused
    pub const KEYMAP_CAPABILITIES_QUERY:            &[u8] = &[0x04, 0x01];
    pub const KEYMAP_GET_LAYERCOUNT:                &[u8] = &[0x04, 0x02];
    pub const KEYMAP_GET_KEYCODE:                   &[u8] = &[0x04, 0x03];
    pub const KEYMAP_GET_ENCODER_KEYCODE:           &[u8] = &[0x04, 0x04];
// ===========
    pub const REMAP:                                &[u8] = &[0x05, 0x00]; //unused
    pub const REMAP_CAPABILITIES_QUERY:             &[u8] = &[0x05, 0x01];
    pub const REMAP_GET_LAYER_COUNT:                &[u8] = &[0x05, 0x02];
    pub const REMAP_SET_KEYCODE:                    &[u8] = &[0x05, 0x03];
    pub const REMAP_SET_ENCODER_KEYCODE:            &[u8] = &[0x05, 0x04];
// ===========
    pub const LIGHTING:                             &[u8] = &[0x06, 0x00]; //unused
    pub const LIGHTING_CAPABILITIES_QUERY:          &[u8] = &[0x06, 0x01];
// -----
    pub const BACKLIGHT:                            &[u8] = &[0x06, 0x02, 0x00]; //unused
    pub const BACKLIGHT_CAPABILITIES_QUERY:         &[u8] = &[0x06, 0x02, 0x01];
    pub const BACKLIGHT_GET_ENABLED_EFFECTS:        &[u8] = &[0x06, 0x02, 0x02];
    pub const BACKLIGHT_GET_CONFIG:                 &[u8] = &[0x06, 0x02, 0x03];
    pub const BACKLIGHT_SET_CONFIG:                 &[u8] = &[0x06, 0x02, 0x04];
    pub const BACKLIGHT_SAVE_CONFIG:                &[u8] = &[0x06, 0x02, 0x05];
// -----
    pub const RGBLIGHT:                             &[u8] = &[0x06, 0x03, 0x00]; //unused
    pub const RGBLIGHT_CAPABILITIES_QUERY:          &[u8] = &[0x06, 0x03, 0x01];
    pub const RGBLIGHT_GET_ENABLED_EFFECTS:         &[u8] = &[0x06, 0x03, 0x02];
    pub const RGBLIGHT_GET_CONFIG:                  &[u8] = &[0x06, 0x03, 0x03];
    pub const RGBLIGHT_SET_CONFIG:                  &[u8] = &[0x06, 0x03, 0x04];
    pub const RGBLIGHT_SAVE_CONFIG:                 &[u8] = &[0x06, 0x03, 0x05];
// -----
    pub const RGBMATRIX:                            &[u8] = &[0x06, 0x04, 0x00]; // unused
    pub const RGBMATRIX_CAPABILITIES_QUERY:         &[u8] = &[0x06, 0x04, 0x01];
    pub const RGBMATRIX_GET_ENABLE_DEFFECTS:        &[u8] = &[0x06, 0x04, 0x02];
    pub const RGBMATRIX_GET_CONFIG:                 &[u8] = &[0x06, 0x04, 0x03];
    pub const RGBMATRIX_SET_CONFIG:                 &[u8] = &[0x06, 0x04, 0x04];
    pub const RGBMATRIX_SAVE_CONFIG:                &[u8] = &[0x06, 0x04, 0x05];
}
