match broadcast.broadcast_type() {
    // -- Otros tipos recortados --
    BroadcastType::User => {
    event_channel
        .send(XAPEvent::ReceivedUserBroadcast { id, broadcast })
        .expect("failed to send user broadcast event!");
}
