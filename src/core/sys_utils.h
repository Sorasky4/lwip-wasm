#ifndef _SYS_UTILS_H_
#define _SYS_UTILS_H_

#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t sys_htonl(uint32_t val);
extern uint16_t sys_htons(uint16_t val);
extern uint16_t sys_ntohs(uint16_t val);


#ifdef __cplusplus
}
#endif

#endif /* _SYS_UTILS_H_ */