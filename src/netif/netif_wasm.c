#include "netif_wasm_add.h"
#include "netif_wasm_get.h"
#include <sys/socket.h>
#include <arpa/inet.h>

static struct netif netif_wasm;
struct netif* netif_default = NULL;

int netif_wasm_add(const char* ipaddr, const char* netmask)
{
  if (netif_default != NULL) {
    return -1;
  }
  // in_addr_t addr = inet_addr(ipaddr);
  // in_addr_t mask = inet_addr(netmask);
  in_addr_t addr;
  in_addr_t mask;
  inet_pton(AF_INET, ipaddr, &addr);
  inet_pton(AF_INET, netmask, &mask);
  netif_wasm.ip_addr.addr = addr;
  netif_wasm.netmask.addr = mask;
  printf("ipaddr=0x%08x\n", addr);
  printf("mask=0x%08x\n", mask);

  netif_default = &netif_wasm;
  return 0;
}

struct netif* netif_wasm_get(uint32_t ipaddr)
{
  return netif_default;
}
