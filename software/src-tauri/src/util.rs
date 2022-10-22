// Would like LEN to work without importing on main, ie: replacing the const with its value on the macro
#[macro_export]
macro_rules! create_report {
    () => {
        [0; XAP_REPORT_LEN]
    }
}

#[macro_export]
macro_rules! get_device {
    () => {
        XAP_DEVICE.get().unwrap().lock().await
    }
}