use super::context::CpuContext;
use super::pid::{alloc_pid, Pid};

pub const MAX_PROCS: usize = 64;
pub const STACK_SIZE: usize = 16384;

#[derive(Clone, Copy, PartialEq)]
pub enum ProcessState {
    Ready,
    Running,
    Sleeping,
    Zombie,
}

pub struct Process {
    pub pid: Pid,
    pub state: ProcessState,
    pub context: CpuContext,
    pub kernel_stack: usize,
    pub user_stack: usize,
    pub entry_point: usize,
    pub cr3: usize,
}

pub(crate) static mut PROCESS_TABLE: [Option<Process>; MAX_PROCS] = [const { None }; MAX_PROCS];
pub(crate) static mut CURRENT: usize = 0;
pub(crate) static mut SCHED_ACTIVE: bool = false;

static mut STACKS: [u8; MAX_PROCS * STACK_SIZE] = [0; MAX_PROCS * STACK_SIZE];

fn stack_top(slot: usize) -> usize {
    unsafe { STACKS.as_ptr().add((slot + 1) * STACK_SIZE) as usize }
}

/// Build the frame `popad; iretd` expects for a task that has never run.
/// Memory from low to high: pushad frame (edi..eax), then eip, cs, eflags.
fn make_frame(top: usize, entry: usize) -> usize {
    unsafe {
        let mut p = (top as *mut u32).sub(11);
        *p = 0;                 // edi
        p = p.add(1); *p = 0;   // esi
        p = p.add(1); *p = 0;   // ebp
        p = p.add(1); *p = top as u32; // esp
        p = p.add(1); *p = 0;   // ebx
        p = p.add(1); *p = 0;   // edx
        p = p.add(1); *p = 0;   // ecx
        p = p.add(1); *p = 0;   // eax
        p = p.add(1); *p = entry as u32; // eip
        p = p.add(1); *p = 0x08; // kernel code segment
        p = p.add(1); *p = 0x202; // eflags, IF set
        (top as *mut u32).sub(11) as usize
    }
}

fn create(entry: usize) -> Option<(usize, Pid)> {
    let slot = unsafe { PROCESS_TABLE.iter().position(|p| p.is_none()) }?;
    let pid = alloc_pid();
    let top = stack_top(slot);
    let sp = make_frame(top, entry);

    unsafe {
        PROCESS_TABLE[slot] = Some(Process {
            pid,
            state: ProcessState::Ready,
            context: CpuContext {
                esp: sp as u32,
                eip: entry as u32,
                eflags: 0x202,
                ..Default::default()
            },
            kernel_stack: top,
            user_stack: 0,
            entry_point: entry,
            cr3: 0,
        });
    }
    Some((slot, pid))
}

fn idle() -> ! {
    loop {
        unsafe { core::arch::asm!("hlt"); }
    }
}

/// Create PID 0 (idle) and PID 1 (shell). Returns the stack pointer the
/// assembly entry loads to start the shell.
#[no_mangle]
pub extern "C" fn proc_init_shell(shell_entry: usize) -> usize {
    unsafe {
        for slot in PROCESS_TABLE.iter_mut() {
            *slot = None;
        }
        CURRENT = 0;
        SCHED_ACTIVE = false;
    }

    let _ = create(idle as usize);
    let (shell_slot, _) = match create(shell_entry) {
        Some(v) => v,
        None => loop {
            unsafe { core::arch::asm!("hlt"); }
        },
    };

    unsafe {
        CURRENT = shell_slot;
        let p = PROCESS_TABLE[shell_slot].as_mut().unwrap();
        p.state = ProcessState::Running;
        let sp = p.context.esp as usize;
        SCHED_ACTIVE = true;
        sp
    }
}

#[no_mangle]
pub extern "C" fn proc_spawn(entry: usize) -> u32 {
    match create(entry) {
        Some((_, pid)) => pid.0,
        None => 0,
    }
}
