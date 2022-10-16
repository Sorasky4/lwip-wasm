#include "sys_utils.h"
#include <arpa/inet.h>

uint32_t sys_htonl(uint32_t val)
{
  uint32_t ret = htonl(val);
  return ret;
}
uint16_t sys_htons(uint16_t val)
{
  uint16_t ret = htons(val);
  return ret;
}
uint16_t sys_ntohs(uint16_t val)
{
  uint16_t ret = ntohs(val);
  return ret;
}
