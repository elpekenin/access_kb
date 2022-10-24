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
pub enum XAPMessages {
    XapVersion = 0x0000,              // 0x0000
    XapCapabilities,                  // 0x0001
    XapEnabledSubsystem,              // 0x0002
    XapSecureStatus,                  // 0x0003
    XapSecureUnlock,                  // 0x0004
    XapSecureLock,                    // 0x0005
// ===================================
    QmkVersion = 0x0100,              // 0x0100
    QmkCapabilities,                  // 0x0101
    QmkIdentifiers,                   // 0x0102
    QmkManufacturer,                  // 0x0103
    QmkProduct,                       // 0x0104
    QmkConfigLength,                  // 0x0105
    QmkConfigChunk,                   // 0x0106
    QmkJumpBootloader,                // 0x0107
    QmkHardwareIdentifier,            // 0x0108
    QmkResetEeprom,                   // 0x0109
// ===================================
    Kb = 0x0200,                      // 0x0200 (reserved)
// ===================================
    User = 0x0300,                    // 0x0300 (reserved)
// ===================================
    Km = 0x0400,                      // 0x0400 (ununsed)
    KmCapabilities,                   // 0x0401
    KmLayerCount,                     // 0x0402
    KmGetKeycode,                     // 0x0403
    KmGetEncoder,                     // 0x0404
// ===================================
    Remap = 0x0500,                   // 0x0500 (ununsed)
    RemapCapabilities,                // 0x0501
    RemapLayerCount,                  // 0x0502
    RemapKeycode,                     // 0x0503
    RemapEncoder,                     // 0x0504
// ===================================
    Lighting = 0x0600,                // 0x0600 (unused)
    LightingCapabilities,             // 0x0601
// -------------------
    Backlight = 0x060200,             // 0x060200 (unused)
    BacklightCapabilities,            // 0x060201
    BacklightEnabledEffects,          // 0x060202
    BacklightGetConfig,               // 0x060203
    BacklightSetConfig,               // 0x060204
    BacklightSaveConfig,              // 0x060205
// -------------------
    Rgblight = 0x060300,              // 0x060300 (unused)
    RgblightCapabilities,             // 0x060301
    RgblightEnabledEffects,           // 0x060302
    RgblightGetConfig,                // 0x060303
    RgblightSetConfig,                // 0x060304
    RgblightSaveConfig,               // 0x060305
// -------------------
    Rgbmatrix = 0x060400,             // 0x060400 (unused)
    RgbmatrixCapabilities,            // 0x060401
    RgbmatrixEnabledEffects,          // 0x060402
    RgbmatrixGetConfig,               // 0x060403
    RgbmatrixSetConfig,               // 0x060404
    RgbmatrixSaveConfig,              // 0x060405
}

// ===========================================================================
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
        for i in 0..data.len() {
            self.raw_data[i+start] = data[i];
        }

        self
    }

    pub fn set_from_kb(&mut self, from_kb: bool) -> &mut Self{
        self.from_kb = from_kb;

        self
    }
}
