#include "udp_multicast_manager.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define UDP_MC_INFO_INC_SIZE     10U
static int udp_mc_info_count = 0;
static int udp_mc_info_max = 0;
static UdpMcInfoType *udp_mc_info = NULL;

static void initialize_udp_mc_info_memory(char* p, int off, int inc_count)
{
  memset(
    &p[off * sizeof(UdpMcInfoType)],
    0,
    ( inc_count * sizeof(UdpMcInfoType) )
  );
  udp_mc_info_max += inc_count;
  return;
}

UdpMcInfoType* udp_mc_info_alloc(void* pcb)
{
  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  if (udp_mc_info_count >= udp_mc_info_max) {
    udp_mc_info = realloc(udp_mc_info, (udp_mc_info_max + UDP_MC_INFO_INC_SIZE) * sizeof(UdpMcInfoType));
    if (udp_mc_info == NULL) {
      CMSIS_IMPL_ERROR("%s %s() %d: udp_mc_info=NULL exit", __FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }
    initialize_udp_mc_info_memory((char*)udp_mc_info, udp_mc_info_max, UDP_MC_INFO_INC_SIZE);
  }
  int i;
  for (i = 0; i < udp_mc_info_max; i++) {
    if (udp_mc_info[i].pcb == NULL) {
      udp_mc_info[i].sd = -1;
      if (udp_mc_initialize(&udp_mc_info[i]) != 0) {
        return NULL;
      }
      udp_mc_info[i].pcb = pcb;
      udp_mc_info_count++;
      CMSIS_IMPL_DEBUG("%s %s() %d: mcp=%p exit", __FILE__, __FUNCTION__, __LINE__, &udp_mc_info[i]);
      return &udp_mc_info[i];
    }
  }
  return NULL;
}


void udp_mc_info_free(UdpMcInfoType* mcp)
{
  CMSIS_IMPL_DEBUG("%s %s() %d: enter", __FILE__, __FUNCTION__, __LINE__);
  if (mcp == NULL) {
    return;
  }
  if (mcp->pcb != NULL) {
    udp_mc_info_count--;
    mcp->pcb = NULL;
    mcp->sd = -1;
    //TODO socket close
  }
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return;
}
UdpMcInfoType* udp_mc_info_get(void* pcb)
{
  int i;
  for (i = 0; i < udp_mc_info_max; i++) {
    if (udp_mc_info[i].pcb == pcb) {
      return &udp_mc_info[i];
    }
  }
  return NULL;
}
err_t udp_mc_joingroup(uint32_t ifaddr, uint32_t groupaddr)
{
  int i;
  CMSIS_IMPL_DEBUG("%s %s() %d: enter ifaddr=0x%x groupaddr=0x%x", __FILE__, __FUNCTION__, __LINE__, ifaddr, groupaddr);
  for (i = 0; i < udp_mc_info_max; i++) {
    if (udp_mc_info[i].pcb == NULL) {
      continue;
    }
    if (udp_mc_info[i].local_ipaddr.addr != ifaddr) {
      continue;
    }
    int rc = udp_mc_bind_addr(&udp_mc_info[i], groupaddr);
    if (rc != 0) {
      CMSIS_IMPL_INFO("udp_mc_joingroup(): udp_mc_set_request() err=%d", rc);
      return ERR_ABRT;
    }
    CMSIS_IMPL_INFO("udp_mc_joingroup(): mcp=%p ifaddr=0x%x groupaddr=0x%x", &udp_mc_info[i], ifaddr, groupaddr);
  }
  CMSIS_IMPL_DEBUG("%s %s() %d: exit", __FILE__, __FUNCTION__, __LINE__);
  return ERR_OK;
}
