use super::process::{ProcessState, CURRENT, MAX_PROCS, PROCESS_TABLE, SCHED_ACTIVE};

/// Called from timer_handler_asm after `pushad`. Saves the old stack pointer
/// into the current process, picks the next ready process, and returns its
/// saved stack pointer. The caller loads it, then `popad; iretd`.
#[no_mangle]
pub extern "C" fn sched_switch(old_esp: usize) -> usize {
    unsafe {
        if !SCHED_ACTIVE {
            return old_esp;
        }

        if let Some(p) = PROCESS_TABLE[CURRENT].as_mut() {
            p.context.esp = old_esp as u32;
            if p.state == ProcessState::Running {
                p.state = ProcessState::Ready;
            }
        }

        let mut next = CURRENT;
        let mut i = 1;
        while i <= MAX_PROCS {
            let idx = (CURRENT + i) % MAX_PROCS;
            if let Some(p) = PROCESS_TABLE[idx].as_ref() {
                if p.state == ProcessState::Ready {
                    next = idx;
                    break;
                }
            }
            i += 1;
        }

        CURRENT = next;
        if let Some(p) = PROCESS_TABLE[next].as_mut() {
            p.state = ProcessState::Running;
            return p.context.esp as usize;
        }
        old_esp
    }
}
