#include "udp_multicast.h"
#include "sys_utils.h"
#include <errno.h>


int udp_mc_initialize(UdpMcInfoType* info)
{

  info->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (info->sd < 0) {
    CMSIS_IMPL_ERROR("udp_mc_initialize() socket() err=%d", errno);
    return -1;
  }
  int set_option_on = 1;

  int res = setsockopt(info->sd, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option_on,
                       sizeof(set_option_on));
  if (res < 0) {
    CMSIS_IMPL_ERROR("udp_mc_initialize() setsockopt() err=%d", errno);
    return -1;
  }
  return 0;
}

static int udp_mc_enable(UdpMcInfoType* info)
{
  int rc = 0;
  in_addr_t ipaddr;

  ipaddr = info->local_ipaddr.addr;

  //set eth multicast mode
  rc = setsockopt(info->sd, IPPROTO_IP, IP_MULTICAST_IF,
                  (char*)&ipaddr, sizeof(ipaddr));
  if (rc != 0) {
    CMSIS_IMPL_ERROR("udp_mc_enable() setsockopt(IP_MULTICAST_IF) err=%d", errno);
    return -1;
  }
#if 0 /* not used loop callback */
  u_char loop = 0;
  rc = setsockopt(info->sd, IPPROTO_IP, IP_MULTICAST_LOOP,
                  (char*)&loop, sizeof(loop));
  if (rc != 0) {
    CMSIS_IMPL_ERROR("udp_mc_enable() setsockopt(IP_MULTICAST_IF) err=%d", errno);
    return -1;
  }
#endif
  //set TTL
  rc = setsockopt(info->sd, IPPROTO_IP, IP_MULTICAST_TTL,
                  (void*)&(info->ttl), sizeof(info->ttl));
  if (rc != 0) {
    CMSIS_IMPL_ERROR("udp_mc_enable() setsockopt(TTL) err=%d", errno);
    return -1;
  }

  return 0;
}

int udp_mc_bind_addr(UdpMcInfoType* info, uint32_t groupaddr)
{
  int err = udp_mc_enable(info);
  if (err != 0) {
    CMSIS_IMPL_INFO("udp_mc_bind_addr(): udp_mc_enable() err=%d", err);
    return -1;
  }

  //bind multicast addr on local ipaddr
  info->multicast_request.imr_multiaddr.s_addr = groupaddr;
  info->multicast_request.imr_interface.s_addr = info->local_ipaddr.addr;
  int rc = setsockopt(info->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                      (void*)&info->multicast_request, sizeof(struct ip_mreq));
  if (rc < 0) {
    CMSIS_IMPL_ERROR("can not set multicast setting: errno=%d", errno);
    return -1;
  }
  return 0;
}
