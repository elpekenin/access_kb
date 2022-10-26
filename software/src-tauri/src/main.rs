#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

mod util;
mod xap;

use std::time::Duration;

use anyhow::Result;
use log::{info, LevelFilter};
use once_cell::sync::OnceCell;
use tauri::async_runtime::Mutex;

use xap::{XAPClient, XAPDevice, XAPReport, XapRoute};

static XAP_DEVICE: OnceCell<Mutex<XAPDevice>> = OnceCell::new();

#[tauri::command]
async fn find_xap() -> Option<String> {
    let device = get_device!();
    Some(format!("Connected to {}", device))
}

#[tauri::command]
async fn send_test() {
    let device = get_device!();

    device.write(
        &mut XAPReport::from_route(XapRoute::QMK_VERSION_QUERY)
    );

    device.read_timeout(&mut XAPReport::new(), 500);
}

fn main() -> Result<()> {
    env_logger::builder()
        .format_timestamp(None)
        .filter_level(LevelFilter::Info)
        .init();

    let mut xap_client = XAPClient::new()?;

    info!("querying for compatible XAP devices");
    let device = loop {
        if let Ok(device) = xap_client.get_first_xap_device() {
            info!("Found a device");
            break device;
        } else {
            info!(".");
            std::thread::sleep(Duration::from_secs(1));
        }
    };

    XAP_DEVICE
        .set(Mutex::new(device))
        .expect("couldn't move XAP device into Mutex");

    tauri::Builder::default()
        .invoke_handler(
            tauri::generate_handler![
                find_xap,
                send_test
            ]
        )
        .run(tauri::generate_context!())
        .expect("error while running tauri application");

    Ok(())
}
