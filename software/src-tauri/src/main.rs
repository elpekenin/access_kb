#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

extern crate hidapi;
use hidapi::HidApi;

// XAP
// const USAGE_PAGE: u16 = 0xFF51;
// const USAGE     : u16 = 0x0058;

// CONSOLE (for testing)
const USAGE_PAGE: u16 = 0xFF31;
const USAGE     : u16 = 0x0074;

// Learn more about Tauri commands at https://tauri.app/v1/guides/features/command
#[tauri::command]
fn find_xap() -> String{
    match HidApi::new() {
        Ok(api) => {
            for device in api.device_list() {
                if device.usage_page() == USAGE_PAGE && device.usage() == USAGE {
                    return format!(
                        "Connected to: {} {}",
                        device.manufacturer_string().unwrap(),
                        device.product_string().unwrap()
                    );
                }
            }
        },
        Err(e) => {
            eprintln!("Error: {}", e);
        },
    }
    return "Couldn't find a XAP device".to_string();
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![find_xap])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
