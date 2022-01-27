#include "eui64.h"
#include <stdint.h>
#include <stdlib.h>

in6_addr eui64(const ether_addr mac) {
  in6_addr res = {0};
  // TODO
  res.s6_addr[0] = 0xfe;
  res.s6_addr[1] = 0x80;
  for (int i = 2; i <= 7; i++)
    res.s6_addr[i] = 0x00;
  for (int i = 8; i <= 10; i++)
    res.s6_addr[i] = mac.ether_addr_octet[i-8];
  for (int i = 13; i <= 15; i++)
    res.s6_addr[i] = mac.ether_addr_octet[i-10];
  res.s6_addr[8] = res.s6_addr[8]^0x02;
  res.s6_addr[11] = 0xff;
  res.s6_addr[12] = 0xfe;

  return res;
}