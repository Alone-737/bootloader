use core::sync::atomic::{AtomicU32, Ordering};

/// Typed process ID. A raw integer cannot be passed where a PID is expected.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct Pid(pub u32);

static NEXT_PID: AtomicU32 = AtomicU32::new(1);

pub fn alloc_pid() -> Pid {
    Pid(NEXT_PID.fetch_add(1, Ordering::SeqCst))
}
