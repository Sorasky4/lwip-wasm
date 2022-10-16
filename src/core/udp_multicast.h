#ifndef _UDP_MULTICAST_H_
#define _UDP_MULTICAST_H_

#include "cmsis_os.h"
#include "lwip/ip4_addr.h"
#include "lwip/udp.h"
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void* pcb; //udp_new
  uint8_t ttl; //udp_new
  int sd; //udp_new
  ip_addr_t local_ipaddr; //udp_bind
  uint16_t port; //udp_bind
  struct ip_mreq multicast_request; //igmp_joingroup
} UdpMcInfoType;

extern int udp_mc_initialize(UdpMcInfoType* info);
extern int udp_mc_bind_addr(UdpMcInfoType* info, uint32_t groupaddr);

#ifdef __cplusplus
}
#endif

#endif /* _UDP_MULTICAST_H_ */