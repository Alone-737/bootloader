#include "rtl8139.h"
#include "io.h"

#define RTL_VENDOR 0x10EC
#define RTL_DEVICE 0x8139

#define R_IDR0    0x00
#define R_TSD0    0x10
#define R_TSAD0   0x20
#define R_RBSTART 0x30
#define R_CR      0x37
#define R_CAPR    0x38
#define R_IMR     0x3C
#define R_ISR     0x3E
#define R_RCR     0x44
#define R_CONFIG1 0x52

#define CR_RST 0x10
#define CR_RE  0x08
#define CR_TE  0x04

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC

#define RX_SIZE 8192

/* The identity map makes these physical addresses too, which DMA needs. */
static uint8_t rx_buf[RX_SIZE + 16] __attribute__((aligned(16)));
static uint8_t tx_buf[2048] __attribute__((aligned(16)));
static uint16_t io_base;
static uint8_t mac[6];
static uint8_t tx_slot;
static uint16_t rx_off;
static int ready;

/* shortcut: bus 0 / function 0 only, which is where QEMU puts the card. */
static uint32_t pci_read(uint8_t slot, uint8_t off)
{
    outl(PCI_ADDR, 0x80000000u | ((uint32_t)slot << 11) | (off & 0xFC));
    return inl(PCI_DATA);
}

static void pci_write(uint8_t slot, uint8_t off, uint32_t val)
{
    outl(PCI_ADDR, 0x80000000u | ((uint32_t)slot << 11) | (off & 0xFC));
    outl(PCI_DATA, val);
}

static int find_card(void)
{
    for (uint8_t slot = 0; slot < 32; slot++)
    {
        uint32_t id = pci_read(slot, 0x00);
        if ((uint16_t)id == 0xFFFF)
            continue;
        if ((uint16_t)id != RTL_VENDOR || (uint16_t)(id >> 16) != RTL_DEVICE)
            continue;

        uint32_t bar0 = pci_read(slot, 0x10);
        if (!(bar0 & 1))
            continue;

        /* Without bus mastering every DMA read returns zeros. */
        uint32_t cmd = pci_read(slot, 0x04) & 0xFFFF;
        pci_write(slot, 0x04, cmd | 0x0005);
        return (int)(bar0 & ~3u);
    }
    return -1;
}

int rtl8139_init(void)
{
    int bar = find_card();
    if (bar < 0)
        return -1;
    io_base = (uint16_t)bar;

    outb(io_base + R_CONFIG1, 0x00);
    outb(io_base + R_CR, CR_RST);
    for (uint32_t spin = 0; inb(io_base + R_CR) & CR_RST; spin++)
        if (spin > 1000000)
            return -1;

    for (int i = 0; i < 6; i++)
        mac[i] = inb(io_base + R_IDR0 + i);

    outl(io_base + R_RBSTART, (uint32_t)rx_buf);
    outw(io_base + R_IMR, 0x0005);
    outw(io_base + R_ISR, 0xFFFF);
    outl(io_base + R_RCR, 0x8F);   /* 8K buffer, accept bcast/mcast/physical, wrap */
    outb(io_base + R_CR, CR_RE | CR_TE);

    ready = 1;
    return 0;
}

void rtl8139_mac(uint8_t out[6])
{
    for (int i = 0; i < 6; i++)
        out[i] = mac[i];
}

int rtl8139_send(const void *frame, uint16_t len)
{
    if (!ready || len > sizeof(tx_buf))
        return -1;

    const uint8_t *src = frame;
    uint16_t n = len < 60 ? 60 : len;
    for (uint16_t i = 0; i < n; i++)
        tx_buf[i] = i < len ? src[i] : 0;

    outw(io_base + R_ISR, 0x0005);
    outl(io_base + R_TSAD0 + 4 * tx_slot, (uint32_t)tx_buf);
    outl(io_base + R_TSD0 + 4 * tx_slot, n);

    for (uint32_t spin = 0; spin < 10000000; spin++)
        if (inw(io_base + R_ISR) & 0x0004)
        {
            outw(io_base + R_ISR, 0x0004);
            /* QEMU services the descriptors round-robin, so advance on success. */
            tx_slot = (tx_slot + 1) & 3;
            return 0;
        }
    return -1;
}

int rtl8139_recv(void *buf, int max)
{
    if (!ready || (inb(io_base + R_CR) & 0x01))
        return 0;

    uint16_t off = rx_off;
    uint8_t *p = rx_buf + off;
    uint16_t status = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    uint16_t len = (uint16_t)p[2] | ((uint16_t)p[3] << 8);

    int out = 0;
    if (status & 0x01)
    {
        out = len < max ? len : max;
        for (int i = 0; i < out; i++)
            ((uint8_t *)buf)[i] = rx_buf[(off + 4 + i) % RX_SIZE];
    }

    rx_off = (uint16_t)((off + 4 + len + 3) & ~3u);
    if (rx_off >= RX_SIZE)
        rx_off -= RX_SIZE;
    /* CAPR is off by 16 on the real chip and in QEMU. */
    outw(io_base + R_CAPR, rx_off - 16);
    outw(io_base + R_ISR, 0x0001);
    return out;
}
