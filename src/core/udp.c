#include "cmsis_os.h"
#include "lwip/udp.h"

#include "netif_posix_get.h"
#include "sys_utils.h"
#include "udp_multicast_manager.h"

#include <string.h>
#include <errno.h>

struct udp_pcb* udp_pcbs;
static void thread_udp_recv(void* argument);
/*
 * config
 */
#define MAX_SEND_PACKET_SIZE    4096
#define MAX_RECV_PACKET_SIZE    4096
//#define DEBUG_SEND_PACKET
//#define DEBUG_RECV_PACKET

#if ( (defined DEBUG_SEND_PACKET) || (defined DEBUG_RECV_PACKET))
static void udp_packet_debug_print(const char* buffer, int len);
#endif

void
udp_init(void)
{
  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  udp_pcbs = NULL;
  return;
}

/**
 * @ingroup udp_raw
 * Send data to a specified address using UDP.
 *
 * @param pcb UDP PCB used to send the data.
 * @param p chain of pbuf's to be sent.
 * @param dst_ip Destination IP address.
 * @param dst_port Destination UDP port.
 *
 * dst_ip & dst_port are expected to be in the same byte order as in the pcb.
 *
 * If the PCB already has a remote address association, it will
 * be restored after the data is sent.
 *
 * @return lwIP error code (@see udp_send for possible error codes)
 *
 * @see udp_disconnect() udp_send()
 */
err_t
udp_sendto(struct udp_pcb* pcb, struct pbuf* p,
           const ip_addr_t* dst_ip, u16_t dst_port)
{
  UdpMcInfoType* mcp = udp_mc_info_get(pcb);
  int off = 0;
  int len;
  struct pbuf* entry = p;
  char buffer[MAX_SEND_PACKET_SIZE];
  struct sockaddr_in addr;
  if (mcp == NULL) {
    CMSIS_IMPL_ERROR("udp_sendto():mcp = NULL");
    return ERR_ABRT;
  }
  if (mcp->sd < 0) {
    return ERR_CONN;
  }

  int sendmsg_len = 0;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = dst_ip->addr;
  addr.sin_port = sys_htons(dst_port);
  len = 0;
  do {
    memcpy(&buffer[off], entry->payload, entry->len);
    off += entry->len;
    len += entry->len;
    entry = entry->next;
  } while (entry != NULL);
  sendmsg_len = sendto(mcp->sd, buffer, len, 0,
                       (struct sockaddr*)&(addr),
                       sizeof(addr));
  if (sendmsg_len != len) {
    CMSIS_IMPL_ERROR("udp_sendto():sd=%d sendmsg_len = %d len=%d addr=0x%x p->payload=%p errno=%d",
                     mcp->sd, sendmsg_len, p->len, mcp->local_ipaddr.addr, p->payload, errno);
    return ERR_CONN;
  }
#ifdef DEBUG_SEND_PACKET
  CMSIS_IMPL_DEBUG("udp_sendto():mcp=%p sendmsg_len = %d len=%d addr=0x%x port=%d p->payload=%p errno=%d",
                   mcp, sendmsg_len, len, addr.sin_addr.s_addr, dst_port, p->payload, errno);
  udp_packet_debug_print(buffer, sendmsg_len);
#endif
  return ERR_OK;
}

static err_t
udp_bind_socket(uint32_t addr, UdpMcInfoType* mcp)
{
  struct sockaddr_in tmp_addr;
  tmp_addr.sin_family = AF_INET;
  //tmp_addr.sin_addr.s_addr = addr;
  tmp_addr.sin_addr.s_addr = sys_htonl(INADDR_ANY);
  tmp_addr.sin_port = sys_htons(mcp->port);
  int localerr = bind(mcp->sd, (struct sockaddr*)&tmp_addr, sizeof(tmp_addr));
  if (localerr != 0) {
    return ERR_ABRT;
  }
  return ERR_OK;
}

/**
 * @ingroup udp_raw
 * Bind an UDP PCB.
 *
 * @param pcb UDP PCB to be bound with a local address ipaddr and port.
 * @param ipaddr local IP address to bind with. Use IP_ANY_TYPE to
 * bind to all local interfaces.
 * @param port local UDP port to bind with. Use 0 to automatically bind
 * to a random port between UDP_LOCAL_PORT_RANGE_START and
 * UDP_LOCAL_PORT_RANGE_END.
 *
 * ipaddr & port are expected to be in the same byte order as in the pcb.
 *
 * @return lwIP error code.
 * - ERR_OK. Successful. No error occurred.
 * - ERR_USE. The specified ipaddr and port are already bound to by
 * another UDP PCB.
 *
 * @see udp_disconnect()
 */
err_t
udp_bind(struct udp_pcb* pcb, const ip_addr_t* ipaddr, u16_t port)
{
  struct udp_pcb* ipcb;
  u8_t rebind;

  /* Don't propagate NULL pointer (IPv4 ANY) to subsequent functions */
  if (ipaddr == NULL) {
    ipaddr = IP4_ADDR_ANY;
  }
  CMSIS_IMPL_DEBUG("%s %s() %d: ipaddr=0x%x port=%d enter", __FILE__, __FUNCTION__, __LINE__, ipaddr->addr, port);

  LWIP_ERROR("udp_bind: invalid pcb", pcb != NULL, return ERR_ARG);

  LWIP_DEBUGF(UDP_DEBUG | LWIP_DBG_TRACE, ("udp_bind(ipaddr = "));
  ip_addr_debug_print(UDP_DEBUG | LWIP_DBG_TRACE, ipaddr);
  LWIP_DEBUGF(UDP_DEBUG | LWIP_DBG_TRACE, (", port = %"U16_F")\n", port));

  UdpMcInfoType* mcp = udp_mc_info_get(pcb);
  if (mcp == NULL) {
    CMSIS_IMPL_DEBUG("%s %s() %d: mcp null exit", __FILE__, __FUNCTION__, __LINE__);
    return ERR_ABRT;
  } else {
    CMSIS_IMPL_DEBUG("%s %s() %d: mcp exist %p", __FILE__, __FUNCTION__, __LINE__, mcp);
  }


  rebind = 0;
  /* Check for double bind and rebind of the same pcb */
  for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) {
    /* is this UDP PCB already on active list? */
    if (pcb == ipcb) {
      rebind = 1;
      break;
    }
  }

  /* no port specified? */
  if (port == 0) {
    LWIP_DEBUGF(UDP_DEBUG, ("udp_bind: out of free UDP ports\n"));
    return ERR_USE;
  } else {
    for (ipcb = udp_pcbs; ipcb != NULL; ipcb = ipcb->next) {
      if (pcb != ipcb) {
        /* By default, we don't allow to bind to a port that any other udp
           PCB is already bound to, unless *all* PCBs with that port have tha
           REUSEADDR flag set. */
#if SO_REUSE
        if (!ip_get_option(pcb, SOF_REUSEADDR) ||
            !ip_get_option(ipcb, SOF_REUSEADDR))
#endif /* SO_REUSE */
        {
          /* port matches that of PCB in list and REUSEADDR not set -> reject */
          if ((ipcb->local_port == port) &&
              /* IP address matches or any IP used? */
              (ip_addr_cmp(&ipcb->local_ip, ipaddr) || ip_addr_isany(ipaddr) ||
               ip_addr_isany(&ipcb->local_ip))) {
            /* other PCB already binds to this local IP and port */
            LWIP_DEBUGF(UDP_DEBUG,
                        ("udp_bind: local port %"U16_F" already bound by another pcb\n", port));
            return ERR_USE;
          }
        }
      }
    }
  }
  ip_addr_set_ipaddr(&pcb->local_ip, ipaddr);
  pcb->local_port = port;

  struct netif* inf = netif_posix_get(ipaddr->addr);
  if (inf == NULL) {
    CMSIS_IMPL_ERROR("%s %s() %d: not found netif ipaddr=0x%x port=%d", __FILE__, __FUNCTION__, __LINE__, ipaddr->addr, port);
    return ERR_ABRT;
  }

  mcp->local_ipaddr.addr = inf->ip_addr.addr;
  mcp->port = port;

  err_t err = udp_bind_socket(inf->ip_addr.addr, mcp);
  if (err != ERR_OK) {
    CMSIS_IMPL_ERROR("udp_bind() bind() err=%d", errno);
    return ERR_ABRT;
  }

  /* pcb not active yet? */
  if (rebind == 0) {
    /* place the PCB on the active list if not already there */
    pcb->next = udp_pcbs;
    udp_pcbs = pcb;
  }
  LWIP_DEBUGF(UDP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("udp_bind: bound to "));
  ip_addr_debug_print_val(UDP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, pcb->local_ip);
  LWIP_DEBUGF(UDP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, (", port %"U16_F")\n", pcb->local_port));
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return ERR_OK;
}
/**
 * @ingroup udp_raw
 * Set a receive callback for a UDP PCB.
 * This callback will be called when receiving a datagram for the pcb.
 *
 * @param pcb the pcb for which to set the recv callback
 * @param recv function pointer of the callback function
 * @param recv_arg additional argument to pass to the callback function
 */
void
udp_recv(struct udp_pcb* pcb, udp_recv_fn recv, void* recv_arg)
{
  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  LWIP_ERROR("udp_recv: invalid pcb", pcb != NULL, return);

  /* remember recv() callback and user data */
  pcb->recv = recv;
  pcb->recv_arg = recv_arg;

  osThreadId_t id = osThreadNew(thread_udp_recv, (void*)pcb, NULL);
  if (id == NULL) {
    CMSIS_IMPL_ERROR("udp_recv: osThreadNew() returns NULL");
  }
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return;
}
/**
 * @ingroup udp_raw
 * Removes and deallocates the pcb.
 *
 * @param pcb UDP PCB to be removed. The PCB is removed from the list of
 * UDP PCB's and the data structure is freed from memory.
 *
 * @see udp_new()
 */
void
udp_remove(struct udp_pcb* pcb)
{
  struct udp_pcb* pcb2;

  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  LWIP_ERROR("udp_remove: invalid pcb", pcb != NULL, return);

  /* pcb to be removed is first in list? */
  if (udp_pcbs == pcb) {
    /* make list start at 2nd pcb */
    udp_pcbs = udp_pcbs->next;
    /* pcb not 1st in list */
  } else {
    for (pcb2 = udp_pcbs; pcb2 != NULL; pcb2 = pcb2->next) {
      /* find pcb in udp_pcbs list */
      if (pcb2->next != NULL && pcb2->next == pcb) {
        /* remove pcb from list */
        pcb2->next = pcb->next;
        break;
      }
    }
  }
  udp_mc_info_free(udp_mc_info_get(pcb));
  memp_free(MEMP_UDP_PCB, pcb);
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return;
}
/**
 * @ingroup udp_raw
 * Creates a new UDP pcb which can be used for UDP communication. The
 * pcb is not active until it has either been bound to a local address
 * or connected to a remote address.
 *
 * @return The UDP PCB which was created. NULL if the PCB data structure
 * could not be allocated.
 *
 * @see udp_remove()
 */
struct udp_pcb*
udp_new(void)
{
  struct udp_pcb* pcb;

  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  pcb = (struct udp_pcb*)memp_malloc(MEMP_UDP_PCB);
  /* could allocate UDP PCB? */
  if (pcb != NULL) {
    /* UDP Lite: by initializing to all zeroes, chksum_len is set to 0
     * which means checksum is generated over the whole datagram per default
     * (recommended as default by RFC 3828). */
    /* initialize PCB to all zeroes */
    memset(pcb, 0, sizeof(struct udp_pcb));
    pcb->ttl = UDP_TTL;
#if LWIP_MULTICAST_TX_OPTIONS
    udp_set_multicast_ttl(pcb, UDP_TTL);
#endif /* LWIP_MULTICAST_TX_OPTIONS */
    UdpMcInfoType* mcp = udp_mc_info_alloc(pcb);
    if (mcp == NULL) {
      memp_free(MEMP_UDP_PCB, pcb);
      return NULL;
    }
    mcp->ttl = pcb->ttl;
  }
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return pcb;
}


static void thread_udp_recv(void* argument)
{
  struct udp_pcb* pcb = (struct udp_pcb*)argument;
  UdpMcInfoType* mcp = udp_mc_info_get(pcb);

  CMSIS_IMPL_INFO("thread_udp_recv:UP: mcp=%p", mcp);

  while (true) {
    static char recv_msg[MAX_RECV_PACKET_SIZE + 1];
    int recv_msglen = 0;
    struct sockaddr_in remote_addr;
    if (mcp->sd < 0) {
      osDelay(1000);
      continue;
    }

    //CMSIS_IMPL_INFO("thread_udp_recv:START: mcp=%p", mcp);
    /* Receive a single datagram from the server */
    socklen_t socklen = sizeof(remote_addr);
    recv_msglen = recvfrom(mcp->sd, recv_msg, MAX_RECV_PACKET_SIZE, 0, (struct sockaddr*)&remote_addr, &socklen);
    if (recv_msglen < 0) {
      CMSIS_IMPL_ERROR("thread_udp_recv: line:%d sd=%d %s", __LINE__, mcp->sd, strerror(errno));
      osDelay(1000);
      continue;
    }
    struct pbuf* p = pbuf_alloc(PBUF_RAW, recv_msglen, PBUF_RAM);
    if (p == NULL) {
      CMSIS_IMPL_ERROR("thread_udp_recv: line:%d can not alloc memory size=%d", __LINE__, recv_msglen);
      osDelay(1000);
      continue;
    }
#ifdef DEBUG_RECV_PACKET
    CMSIS_IMPL_DEBUG("RECV IN: mcp=%p len=%d addr=0x%x port=%d", mcp, p->len, remote_addr.sin_addr.s_addr, remote_addr.sin_port);
    udp_packet_debug_print(recv_msg, recv_msglen);
#endif
    memcpy(p->payload, recv_msg, recv_msglen);
    pcb->recv(pcb->recv_arg, pcb, p, (ip_addr_t*)&remote_addr.sin_addr.s_addr, remote_addr.sin_port);
  }
  return;
}

#if ( (defined DEBUG_SEND_PACKET) || (defined DEBUG_RECV_PACKET))
static void udp_packet_debug_print(const char* buffer, int len)
{
  int i;
  for (i = 0; i < len; ) {
    int j;
    for (j = 0; j < 8; j++) {
      if (i >= len) {
        goto done;
      }
      unsigned char c = buffer[i++];
      printf("0x%02x ", c);
    }
    printf("\n");
  }
done:
  printf("\n");
  return;
}
#endif
