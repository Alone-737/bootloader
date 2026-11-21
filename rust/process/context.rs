/// Saved register state for a context switch.
///
/// Field order matches the order `pushad` saves registers. switch.asm reads
/// this layout directly, so the repr and the order must not change.
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
