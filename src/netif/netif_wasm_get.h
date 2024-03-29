#ifndef _NETIF_WASM_GET_H_
#define _NETIF_WASM_GET_H_

#include "cmsis_os.h"
#include "lwip/ip4_addr.h"
#include "lwip/udp.h"


#ifdef __cplusplus
extern "C" {
#endif

extern struct netif* netif_wasm_get(uint32_t ipaddr);

#ifdef __cplusplus
}
#endif

#endif /* _NETIF_WASM_GET_H_ */