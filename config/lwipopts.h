#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Common settings used in most of the pico_w examples
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details)

// allow override in some examples
#ifndef NO_SYS
#define NO_SYS                      0
#endif

// allow override in some examples
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 1
#endif

#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC is incompatible with non polling versions
#define MEM_LIBC_MALLOC             0
#endif

#define MEM_ALIGNMENT               4
#define MEM_SIZE                    8000
#define MEMP_NUM_TCP_SEG            8
#define MEMP_NUM_ARP_QUEUE          6
#define MEMP_NUM_SYS_TIMEOUT        16
#define PBUF_POOL_SIZE              8
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_WND                     (2 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (2 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                1
#define MEMP_NUM_NETCONN            5
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0

// if (NO_SYS==1), then LWIP_NETIF_API must be 0
#define LWIP_NETIF_API              0

#define LWIP_IGMP                   0
#define LWIP_DNS                    1
#define LWIP_UDP                    1
#define LWIP_TCP                    1
#define LWIP_PROVIDE_ERRNO          1

// For FreeRTOS
#define TCPIP_THREAD_STACKSIZE      2048
#define DEFAULT_THREAD_STACKSIZE    2048
#define TCPIP_MBOX_SIZE             16
#define DEFAULT_UDP_RECVMBOX_SIZE   8
#define DEFAULT_TCP_RECVMBOX_SIZE   8
#define DEFAULT_ACCEPTMBOX_SIZE     8
#define LWIP_TCPIP_CORE_LOCKING     1
#define LWIP_COMPAT_MUTEX           1
#define LWIP_COMPAT_MUTEX_ALLOWED   1

#define LWIP_TIMEVAL_PRIVATE        0

// Enable checksum validation
#define CHECKSUM_CHECK_IP           1
#define CHECKSUM_CHECK_UDP          1
#define CHECKSUM_CHECK_TCP          1
#define CHECKSUM_CHECK_ICMP         1

// For PPP
#define PPP_SUPPORT                 0

// Minimal HTTPD
#define LWIP_HTTPD                  0

#define LWIP_SNTP                   1
#define SNTP_SERVER_DNS             1
#define SNTP_UPDATE_DELAY           86400000
#define SNTP_RETRY_TIMEOUT          5000
#define SNTP_MAX_SERVERS            3
#define SNTP_SERVER_ADDRESS         "pool.ntp.org"
#define SNTP_DEBUG                  LWIP_DBG_ON
#define SNTP_SET_SYSTEM_TIME(sec)   sntp_set_system_time(sec, 0)

#include "sntp_callbacks.h"

// For DHCP
#define LWIP_DHCP                   1
#define LWIP_DHCP_CHECK_LINK_UP     1

#ifndef NDEBUG
#define LWIP_DEBUG                  0
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0
#endif

#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                 LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                  LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

#endif /* _LWIPOPTS_H */