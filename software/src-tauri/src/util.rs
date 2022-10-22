#[macro_export]
macro_rules! get_device {
    () => {
        XAP_DEVICE.get().unwrap().lock().await
    }
}