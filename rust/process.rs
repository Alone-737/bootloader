//! Process table for Solace. Fixed array, round-robin, no allocator.

pub const MAX_PROCS: usize = 64;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct Pid(pub u32);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ProcessState {
    Unused,
    Ready,
    Running,
    Sleeping,
    Zombie,
}

/// Field order is the order pushad saves registers. switch.asm reads this
/// layout directly, so it must stay repr(C) and keep this order.
#[repr(C)]
#[derive(Default, Clone, Copy)]
pub struct CpuContext {
    pub edi: u32,
    pub esi: u32,
    pub ebp: u32,
    pub esp: u32,
    pub ebx: u32,
    pub edx: u32,
    pub ecx: u32,
    pub eax: u32,
    pub eip: u32,
    pub eflags: u32,
}

#[derive(Clone, Copy)]
pub struct Process {
    pub pid: Pid,
    pub name: [u8; 32],
    pub state: ProcessState,
    pub context: CpuContext,
    pub kernel_stack: usize,
    pub user_stack: usize,
    pub entry_point: usize,
    pub wakeup_tick: u64,
    pub runtime_ticks: u64,
    pub parent: Option<Pid>,
    pub exit_code: i32,
    /// Same value for every process today; becomes per-process CR3 later.
    pub page_directory: usize,
}

impl Process {
    pub fn new(pid: Pid, name: &str, entry: usize) -> Self {
        let mut proc = Self {
            pid,
            name: [0; 32],
            state: ProcessState::Ready,
            context: CpuContext::default(),
            kernel_stack: 0,
            user_stack: 0,
            entry_point: entry,
            wakeup_tick: 0,
            runtime_ticks: 0,
            parent: None,
            exit_code: 0,
            page_directory: 0,
        };

        let bytes = name.as_bytes();
        let mut i = 0;
        while i < bytes.len() && i < 32 {
            proc.name[i] = bytes[i];
            i += 1;
        }
        proc
    }
}

pub static mut PROCESS_TABLE: [Option<Process>; MAX_PROCS] = [const { None }; MAX_PROCS];

pub struct ProcessManager {
    next_pid: u32,
}

impl ProcessManager {
    pub const fn new() -> Self {
        Self { next_pid: 1 }
    }

    pub fn allocate_pid(&mut self) -> Pid {
        let pid = Pid(self.next_pid);
        self.next_pid += 1;
        pid
    }
}

pub struct Scheduler {
    current: usize,
}

impl Scheduler {
    pub const fn new() -> Self {
        Self { current: 0 }
    }

    /// Walk table slots, not PIDs. PID and slot number diverge once a
    /// slot is freed, so `current_pid` cannot be used as an index.
    pub fn next(&mut self, table: &[Option<Process>; MAX_PROCS]) -> Option<usize> {
        let mut i = 1;
        while i <= MAX_PROCS {
            let idx = (self.current + i) % MAX_PROCS;
            match &table[idx] {
                Some(p) if p.state == ProcessState::Ready => {
                    self.current = idx;
                    return Some(idx);
                }
                _ => {}
            }
            i += 1;
        }
        None
    }

    pub fn current(&self) -> usize {
        self.current
    }
}

/// Convert milliseconds to timer ticks. TIMER_FREQ is 100 Hz, so adding
/// milliseconds straight to the tick counter would sleep 10x too long.
pub const fn ms_to_ticks(ms: u32, hz: u32) -> u64 {
    ((ms as u64) * (hz as u64) + 999) / 1000
}
