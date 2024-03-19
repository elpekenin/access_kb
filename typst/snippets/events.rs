pub(crate) enum XAPEvent {
    HandleUserBroadcast {
        broadcast: BroadcastRaw,
        id: Uuid,
    },
    // -- Otros eventos recortados --
}
