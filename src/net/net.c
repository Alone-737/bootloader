#include "net.h"
#include "rtl8139.h"
#include "timer.h"
#include "vga.h"

/* Implemented in rust/solace.rs. */
extern uint16_t net_checksum(const uint8_t *data, uint32_t len);

/* shortcut: static addressing for QEMU's slirp network instead of DHCP.
   Replace with a DHCP client when this leaves QEMU's default setup. */
static const uint8_t our_ip[4]     = {10, 0, 2, 15};
static const uint8_t gateway_ip[4] = {10, 0, 2, 2};

#define ETH_IPV4 0x0800
#define ETH_ARP  0x0806

static uint8_t our_mac[6];
static uint8_t frame[1600];
static uint16_t ip_id;
static int ready;

static void put16(uint8_t *p, uint16_t v)
{
    p[0] = v >> 8;
    p[1] = v & 0xFF;
}

static void copy6(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 6; i++)
        dst[i] = src[i];
}

static void copy4(uint8_t *dst, const uint8_t *src)
{
    for (int i = 0; i < 4; i++)
        dst[i] = src[i];
}

static int same4(const uint8_t *a, const uint8_t *b)
{
    for (int i = 0; i < 4; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

static void eth_header(uint8_t *f, const uint8_t *dst_mac, uint16_t type)
{
    copy6(f, dst_mac);
    copy6(f + 6, our_mac);
    put16(f + 12, type);
}

static int arp_send(const uint8_t target[4])
{
    static const uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t *a = frame + 14;

    eth_header(frame, broadcast, ETH_ARP);
    put16(a + 0, 1);          /* htype: ethernet */
    put16(a + 2, ETH_IPV4);   /* ptype */
    a[4] = 6;
    a[5] = 4;
    put16(a + 6, 1);          /* request */
    copy6(a + 8, our_mac);
    copy4(a + 14, our_ip);
    for (int i = 0; i < 6; i++)
        a[18 + i] = 0;
    copy4(a + 24, target);

    return rtl8139_send(frame, 14 + 28);
}

static int arp_resolve(const uint8_t ip[4], uint8_t mac[6])
{
    for (int attempt = 0; attempt < 3; attempt++)
    {
        if (arp_send(ip) < 0)
            return -1;

        uint32_t deadline = timer_get_ticks() + 20;  /* 200 ms */
        while (timer_get_ticks() < deadline)
        {
            int n = rtl8139_recv(frame, sizeof(frame));
            if (n < 14 + 28)
                continue;
            if (frame[12] != 0x08 || frame[13] != 0x06)
                continue;
            uint8_t *a = frame + 14;
            if (a[6] != 0 || a[7] != 2)              /* reply */
                continue;
            if (!same4(a + 14, ip))                  /* sender address */
                continue;
            copy6(mac, a + 8);
            return 0;
        }
    }
    return -1;
}

static int ip_send(const uint8_t next_hop_mac[6], const uint8_t dst_ip[4],
                   uint8_t proto, const uint8_t *payload, int len)
{
    uint8_t *ip = frame + 14;

    eth_header(frame, next_hop_mac, ETH_IPV4);
    ip[0] = 0x45;
    ip[1] = 0;
    put16(ip + 2, (uint16_t)(20 + len));
    put16(ip + 4, ++ip_id);
    put16(ip + 6, 0);
    ip[8] = 64;
    ip[9] = proto;
    put16(ip + 10, 0);
    copy4(ip + 12, our_ip);
    copy4(ip + 16, dst_ip);
    put16(ip + 10, net_checksum(ip, 20));

    for (int i = 0; i < len; i++)
        ip[20 + i] = payload[i];

    /* shortcut: no fragmentation, so anything larger than this is refused. */
    return rtl8139_send(frame, (uint16_t)(14 + 20 + len));
}

int net_init(void)
{
    if (rtl8139_init() < 0)
    {
        vga_puts("net: init failed\n");
        return -1;
    }
    rtl8139_mac(our_mac);
    ready = 1;
    return 0;
}

/* Off-subnet traffic goes to the gateway's MAC, not the destination's. */
static void next_hop_mac(const uint8_t dst[4], uint8_t mac[6], int *ok)
{
    *ok = 0;
    if (dst[0] == our_ip[0] && dst[1] == our_ip[1] && dst[2] == our_ip[2])
        *ok = arp_resolve(dst, mac) == 0;
    else
        *ok = arp_resolve(gateway_ip, mac) == 0;
}

int net_ping(const uint8_t dst[4], int count)
{
    if (!ready)
    {
        vga_puts("ping: no network card\n");
        return -1;
    }

    uint8_t hop_mac[6];
    int ok;
    next_hop_mac(dst, hop_mac, &ok);
    if (!ok)
    {
        vga_printf("ping: %d.%d.%d.%d unreachable (no arp reply)\n",
                   dst[0], dst[1], dst[2], dst[3]);
        return -1;
    }

    const uint16_t id = 0x5A5A;
    int received = 0;

    for (int seq = 1; seq <= count; seq++)
    {
        uint8_t icmp[32];
        icmp[0] = 8;  /* echo request */
        icmp[1] = 0;
        put16(icmp + 2, 0);
        put16(icmp + 4, id);
        put16(icmp + 6, (uint16_t)seq);
        for (int i = 8; i < (int)sizeof(icmp); i++)
            icmp[i] = (uint8_t)('a' + (i % 26));
        put16(icmp + 2, net_checksum(icmp, sizeof(icmp)));

        uint32_t sent_at = timer_get_ticks();
        if (ip_send(hop_mac, dst, 1, icmp, sizeof(icmp)) < 0)
        {
            vga_printf("ping: send failed on seq=%d\n", seq);
            return received;
        }

        int replied = 0;
        uint32_t deadline = sent_at + 100;  /* 1 s */
        while (!replied && timer_get_ticks() < deadline)
        {
            int n = rtl8139_recv(frame, sizeof(frame));
            if (n < 14 + 20 + 8)
                continue;
            if (frame[12] != 0x08 || frame[13] != 0x00)
                continue;

            uint8_t *ip = frame + 14;
            int ihl = (ip[0] & 0x0F) * 4;
            int total = (ip[2] << 8) | ip[3];
            if (ip[9] != 1 || ihl < 20 || total < ihl + 8 || total > n - 14)
                continue;
            if (!same4(ip + 12, dst))
                continue;
            if (net_checksum(ip, ihl) != 0)
                continue;

            uint8_t *ic = ip + ihl;
            if (ic[0] != 0)  /* echo reply */
                continue;
            if (((ic[4] << 8) | ic[5]) != id || ((ic[6] << 8) | ic[7]) != seq)
                continue;

            replied = 1;
            received++;
            vga_printf("reply from %d.%d.%d.%d: icmp_seq=%d time=%u ms\n",
                       dst[0], dst[1], dst[2], dst[3], seq,
                       (unsigned)((timer_get_ticks() - sent_at) * 10));
        }

        if (!replied)
            vga_printf("ping: icmp_seq=%d timeout\n", seq);

        uint32_t pause = timer_get_ticks() + 100;
        while (timer_get_ticks() < pause)
            ;
    }

    vga_printf("--- %d.%d.%d.%d: %d sent, %d received ---\n",
               dst[0], dst[1], dst[2], dst[3], count, received);
    return received;
}
