#include "lwip/igmp.h"
#include "netif_posix_get.h"
#include "udp_multicast_manager.h"

/**
 * @ingroup igmp
 * Join a group on one network interface.
 *
 * @param ifaddr ip address of the network interface which should join a new group
 * @param groupaddr the ip address of the group which to join
 * @return ERR_OK if group was joined on the netif(s), an err_t otherwise
 */
err_t
igmp_joingroup(const ip4_addr_t* ifaddr, const ip4_addr_t* groupaddr)
{
  struct netif* inf = netif_posix_get(ifaddr->addr);
  if (inf == NULL) {
    CMSIS_IMPL_ERROR("%s %s() %d: not found netif ipaddr=0x%x", __FILE__, __FUNCTION__, __LINE__, ifaddr->addr);
    return ERR_ABRT;
  }
  return udp_mc_joingroup(inf->ip_addr.addr, groupaddr->addr);
}
