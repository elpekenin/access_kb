match broadcast.broadcast_type() {
    // -- Otros eventos recortados --
    Ok(XAPEvent::ReceivedUserBroadcast{broadcast, id}) => {
        user::broadcast_callback(broadcast, id, &state);
    }
}