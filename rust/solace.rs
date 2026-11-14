//! Rust half of the Solace kernel: freestanding, no allocator, no std.
#![no_std]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

/// Internet checksum (RFC 1071) over `len` bytes at `data`.
///
/// Every multi-byte field in Solace packets is stored in network order, so
/// the 16-bit words are built big-endian here too. A caller verifies a
/// received header by checksumming the whole thing and expecting 0.
#[no_mangle]
pub extern "C" fn net_checksum(data: *const u8, len: u32) -> u16 {
    let mut sum: u32 = 0;
    let mut i: u32 = 0;

    while i + 1 < len {
        let hi = unsafe { *data.add(i as usize) } as u32;
        let lo = unsafe { *data.add(i as usize + 1) } as u32;
        sum += (hi << 8) | lo;
        i += 2;
    }
    if i < len {
        sum += (unsafe { *data.add(i as usize) } as u32) << 8;
    }

    while sum >> 16 != 0 {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    !(sum as u16)
}
