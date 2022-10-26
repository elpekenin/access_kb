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
                report.set_from_kb(false)
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
                report.set_from_kb(true)
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
            "{} >> Token: 0x{:04X} | {} | Length: {:02} | Payload: {:?}",
            match self.from_kb {
                true  => "receiving",
                false => "sending  "
            },
            self.get_token(),
            match self.from_kb {
                true  => format!("Flags: {:08b}", self.get_flags()),
                false => format!("               ")
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
        ((self.raw_data[0] as u16) << 8) | self.raw_data[1] as u16
    }

    pub fn set_bytes(&mut self, start: usize, data: &[u8]) -> &mut Self {
        if self.from_kb {
            return self;
        }

        for i in 0..data.len() {
            self.raw_data[i+start] = data[i];
        }

        self
    }

    pub fn set_from_kb(&mut self, from_kb: bool) -> &mut Self{
        if self.from_kb {
            return self;
        }

        self.from_kb = from_kb;

        self
    }

    pub fn set_token(&mut self) -> &mut Self {
        if self.from_kb {
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
    pub const XAP_VERSION_QUERY:                    &[u8] = &[00, 00];
    pub const XAP_CAPABILITIES_QUERY:               &[u8] = &[00, 01];
    pub const XAP_ENABLED_SUBSYSTEM_QUERY:          &[u8] = &[00, 02];
    pub const XAP_SECURE_STATUS:                    &[u8] = &[00, 03];
    pub const XAP_SECURE_UNLOCK:                    &[u8] = &[00, 04];
    pub const XAP_SECURE_LOCK:                      &[u8] = &[00, 05];
// ===========
    pub const QMK_VERSION_QUERY:                    &[u8] = &[01, 00];
    pub const QMK_CAPABILITIES_QUERY:               &[u8] = &[01, 01];
    pub const QMK_IDENTIFIERS:                      &[u8] = &[01, 02];
    pub const QMK_BOARD_MANUFACTURER:               &[u8] = &[01, 03];
    pub const QMK_PRODUCT_NAME:                     &[u8] = &[01, 04];
    pub const QMK_CONFIG_BLOB_LENGTH:               &[u8] = &[01, 05];
    pub const QMK_CONFIG_BLOB_CHUNK:                &[u8] = &[01, 06];
    pub const QMK_JUMP_TO_BOOTLOADER:               &[u8] = &[01, 07];
    pub const QMK_HARDWARE_IDENTIFIER:              &[u8] = &[01, 08];
    pub const QMK_REINITIALIZE_EEPROM:              &[u8] = &[01, 09];
// ===========
    pub const KEYBOARD:                             &[u8] = &[02, 00]; //reserved
// ===========
    pub const USER:                                 &[u8] = &[03, 00]; //reserved
// ===========
    pub const KEYMAP:                               &[u8] = &[04, 00]; //unused
    pub const KEYMAP_CAPABILITIES_QUERY:            &[u8] = &[04, 01];
    pub const KEYMAP_GET_LAYERCOUNT:                &[u8] = &[04, 02];
    pub const KEYMAP_GET_KEYCODE:                   &[u8] = &[04, 03];
    pub const KEYMAP_GET_ENCODER_KEYCODE:           &[u8] = &[04, 04];
// ===========
    pub const REMAP:                                &[u8] = &[05, 00]; //unused
    pub const REMAP_CAPABILITIES_QUERY:             &[u8] = &[05, 01];
    pub const REMAP_GET_LAYER_COUNT:                &[u8] = &[05, 02];
    pub const REMAP_SET_KEYCODE:                    &[u8] = &[05, 03];
    pub const REMAP_SET_ENCODER_KEYCODE:            &[u8] = &[05, 04];
// ===========
    pub const LIGHTING:                             &[u8] = &[06, 00]; //unused
    pub const LIGHTING_CAPABILITIES_QUERY:          &[u8] = &[06, 01];
// -----
    pub const BACKLIGHT:                            &[u8] = &[06, 02, 00]; //unused
    pub const BACKLIGHT_CAPABILITIES_QUERY:         &[u8] = &[06, 02, 01];
    pub const BACKLIGHT_GET_ENABLED_EFFECTS:        &[u8] = &[06, 02, 02];
    pub const BACKLIGHT_GET_CONFIG:                 &[u8] = &[06, 02, 03];
    pub const BACKLIGHT_SET_CONFIG:                 &[u8] = &[06, 02, 04];
    pub const BACKLIGHT_SAVE_CONFIG:                &[u8] = &[06, 02, 05];
// -----
    pub const RGBLIGHT:                             &[u8] = &[06, 03, 00]; //unused
    pub const RGBLIGHT_CAPABILITIES_QUERY:          &[u8] = &[06, 03, 01];
    pub const RGBLIGHT_GET_ENABLED_EFFECTS:         &[u8] = &[06, 03, 02];
    pub const RGBLIGHT_GET_CONFIG:                  &[u8] = &[06, 03, 03];
    pub const RGBLIGHT_SET_CONFIG:                  &[u8] = &[06, 03, 04];
    pub const RGBLIGHT_SAVE_CONFIG:                 &[u8] = &[06, 03, 05];
// -----
    pub const RGBMATRIX:                            &[u8] = &[06, 04, 00]; // unused
    pub const RGBMATRIX_CAPABILITIES_QUERY:         &[u8] = &[06, 04, 01];
    pub const RGBMATRIX_GET_ENABLE_DEFFECTS:        &[u8] = &[06, 04, 02];
    pub const RGBMATRIX_GET_CONFIG:                 &[u8] = &[06, 04, 03];
    pub const RGBMATRIX_SET_CONFIG:                 &[u8] = &[06, 04, 04];
    pub const RGBMATRIX_SAVE_CONFIG:                &[u8] = &[06, 04, 05];
}
