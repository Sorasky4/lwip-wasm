#ifndef _UDP_MULTICAST_MANAGER_H_
#define _UDP_MULTICAST_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "udp_multicast.h"

extern UdpMcInfoType* udp_mc_info_alloc(void* pcb);
extern void udp_mc_info_free(UdpMcInfoType* mcp);
extern UdpMcInfoType* udp_mc_info_get(void* pcb);
extern err_t udp_mc_joingroup(uint32_t ifaddr, uint32_t groupaddr);


#ifdef __cplusplus
}
#endif


#endif /* _UDP_MULTICAST_MANAGER_H_ */