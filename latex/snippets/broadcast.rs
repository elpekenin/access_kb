#[derive(BinRead, Debug)]
pub struct ReceivedUserBroadcast {
    pub x: u16,
    pub y: u16,
}

impl XAPBroadcast for ReceivedUserBroadcast {}
