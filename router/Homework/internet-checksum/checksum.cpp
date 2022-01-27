#include "checksum.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

uint16_t cut_16(uint32_t result){
  uint16_t high = result >> 16, low = result;
  result = low + high;
  if (result >> 16)
    return cut_16(result);  
  else 
    return result;
}

bool validateAndFillChecksum(uint8_t *packet, size_t len) {
  // TODO
  struct ip6_hdr *ip6 = (struct ip6_hdr *)packet;
  std::vector<uint8_t> sum_array;
  for (int i = 0; i < 16; i++)
    sum_array.push_back(ip6->ip6_src.__in6_u.__u6_addr8[i]);
  for (int i = 0; i < 16; i++)
    sum_array.push_back(ip6->ip6_dst.__in6_u.__u6_addr8[i]);
  // check next header
  uint8_t nxt_header = ip6->ip6_nxt;
  if (nxt_header == IPPROTO_UDP) {
    // UDP
    struct udphdr *udp = (struct udphdr *)&packet[sizeof(struct ip6_hdr)];
    // length: udp->uh_ulen
    // checksum: udp->uh_sum
    uint32_t udp_length = ntohl(udp->uh_ulen);
    uint16_t udp_sum = ntohs(udp->uh_sum);
    udp->uh_sum = 0;
    sum_array.push_back((uint8_t)(udp_length >> 24));
    sum_array.push_back((uint8_t)(udp_length >> 16));
    sum_array.push_back((uint8_t)(udp_length >> 8));
    sum_array.push_back((uint8_t)(udp_length));
    for (int i = 0; i < 3; i++)
      sum_array.push_back(0x00);
    sum_array.push_back(nxt_header);
    uint8_t *u = (uint8_t *) udp;
    for (int i = 0; i < ntohs(udp->uh_ulen); i++)
      sum_array.push_back(u[i]);
    uint32_t sum = 0;
    for (int i = 1; i < sum_array.size(); i+=2){
      uint16_t high = sum_array[i-1], low = sum_array[i];
      uint16_t add_sum = (high << 8) + low;
      sum += (uint32_t) add_sum;
      if (i == sum_array.size()-2)
        sum += (((uint16_t)sum_array[i+1]) << 8);
    }
    uint16_t result = ~cut_16(sum);
    if (udp_sum == 0xffff && result == 0x0000) {
      udp->uh_sum = 0xffff;
      return true;
    }
    if (result == 0x0000) result = ~result;
    udp->uh_sum = htons(result);
    if (result == udp_sum) return true;
    else 
      return false;
  } else if (nxt_header == IPPROTO_ICMPV6) {
    // ICMPv6
    struct icmp6_hdr *icmp =
      (struct icmp6_hdr *)&packet[sizeof(struct ip6_hdr)];
    // length: len-sizeof(struct ip6_hdr)
    // checksum: icmp->icmp6_cksum
    uint32_t icmp_length = len-sizeof(struct ip6_hdr);
    uint16_t icmp_sum = ntohs(icmp->icmp6_cksum);
    icmp->icmp6_cksum = 0;
    sum_array.push_back((uint8_t)(icmp_length >> 24));
    sum_array.push_back((uint8_t)(icmp_length >> 16));
    sum_array.push_back((uint8_t)(icmp_length >> 8));
    sum_array.push_back((uint8_t)(icmp_length));
    for (int i = 0; i < 3; i++)
      sum_array.push_back(0x00);
    sum_array.push_back(nxt_header);
    uint8_t *c = (uint8_t *) icmp;
    for (int i = 0; i < icmp_length; i++)
      sum_array.push_back(c[i]);
    uint32_t sum = 0;
    for (int i = 1; i < sum_array.size(); i+=2){
      uint16_t high = sum_array[i-1], low = sum_array[i];
      uint16_t add_sum = (high << 8) + low;
      sum += (uint32_t) add_sum;
      if (i == sum_array.size()-2)
        sum += (((uint16_t)sum_array[i+1]) << 8);
    }
    uint16_t result = ~cut_16(sum);
    if (icmp_sum == 0xffff && result == 0x0000) return true;
    icmp->icmp6_cksum = htons(result);
    if (result == icmp_sum) return true;
    else 
      return false;
  } else {
    assert(false);
  }
  return true;
}


