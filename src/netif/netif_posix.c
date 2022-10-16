#include "netif_posix_add.h"
#include "netif_posix_get.h"
#include <sys/socket.h>
#include <arpa/inet.h>

static struct netif netif_posix;
struct netif* netif_default = NULL;

int netif_posix_add(const char* ipaddr, const char* netmask)
{
  if (netif_default != NULL) {
    return -1;
  }
  in_addr_t addr = inet_addr(ipaddr);
  in_addr_t mask = inet_addr(netmask);
  netif_posix.ip_addr.addr = addr;
  netif_posix.netmask.addr = mask;
  printf("ipaddr=0x%08x\n", addr);
  printf("mask=0x%08x\n", mask);

  netif_default = &netif_posix;
  return 0;
}

struct netif* netif_posix_get(uint32_t ipaddr)
{
  return netif_default;
}
