#include "protocol.h"
#include "common.h"
#include "lookup.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

RipErrorCode disassemble(const uint8_t *packet, uint32_t len,
                         RipPacket *output) {
  // TODO
  if (len < sizeof(struct ip6_hdr)) return RipErrorCode::ERR_LENGTH;
  struct ip6_hdr *ip6 = (struct ip6_hdr *)packet;
  if (ntohs(ip6->ip6_plen) + sizeof(struct ip6_hdr) != len) return RipErrorCode::ERR_LENGTH;
  if (ip6->ip6_nxt != IPPROTO_UDP) return RipErrorCode::ERR_IP_NEXT_HEADER_NOT_UDP;
  if (ntohs(ip6->ip6_plen) < sizeof(struct udphdr)) return RipErrorCode::ERR_LENGTH;
  struct udphdr *udp = (struct udphdr *)&packet[sizeof(struct ip6_hdr)];
  if (ntohs(udp->uh_sport) != 521 || ntohs(udp->uh_dport) != 521) return RipErrorCode::ERR_BAD_UDP_PORT;
  if ((ntohs(udp->uh_ulen) - sizeof(struct udphdr) - sizeof(struct ripng_hdr)) % sizeof(ripng_entry) != 0) return RipErrorCode::ERR_LENGTH; 
  struct ripng_hdr *rhd = (struct ripng_hdr *)&packet[sizeof(struct ip6_hdr)+sizeof(struct udphdr)];
  if (rhd->command != 1 && rhd->command != 2) return RipErrorCode::ERR_RIP_BAD_COMMAND;
  output->command = rhd->command;
  if (rhd->version != 1) return RipErrorCode::ERR_RIP_BAD_VERSION;
  if (rhd->zero != 0) return RipErrorCode::ERR_RIP_BAD_ZERO;
  int entry_num = (ntohs(udp->uh_ulen) - sizeof(struct udphdr) - sizeof(struct ripng_hdr)) / sizeof(ripng_entry);
  output->numEntries = entry_num;
  if (entry_num) {
    for (int i = 0; i < entry_num; i++){
      ripng_entry *e = (ripng_entry *)&packet[sizeof(struct ip6_hdr)+sizeof(struct udphdr)+sizeof(struct ripng_hdr)+i*sizeof(ripng_entry)];
      if (e->metric == 0xff) {
        if (e->prefix_len != 0) return RipErrorCode::ERR_RIP_BAD_PREFIX_LEN;
        if (e->route_tag != 0) return RipErrorCode::ERR_RIP_BAD_ROUTE_TAG;
      }
      else {
        if (e->metric < 1 || e->metric > 16) return RipErrorCode::ERR_RIP_BAD_METRIC;
        if (e->prefix_len < 0 || e->prefix_len > 128) return RipErrorCode::ERR_RIP_BAD_PREFIX_LEN;
        if ((len_to_mask(e->prefix_len) & e->prefix_or_nh) != e->prefix_or_nh) return RipErrorCode::ERR_RIP_INCONSISTENT_PREFIX_LENGTH;
      }
      output->entries[i] = *e;
    }
  }
  return RipErrorCode::SUCCESS;
}


uint32_t assemble(const RipPacket *rip, uint8_t *buffer) {
  // TODO
  struct ripng_hdr *rhd = (struct ripng_hdr *)buffer;
  rhd->command = rip->command;
  rhd->version = 1;
  rhd->zero = 0;
  for (int i = 0; i < rip->numEntries; i++) {
    ripng_entry *e = (ripng_entry *)&buffer[sizeof(struct ripng_hdr)+i*sizeof(ripng_entry)];
    e->metric = rip->entries[i].metric;
    e->prefix_len = rip->entries[i].prefix_len;
    e->prefix_or_nh = rip->entries[i].prefix_or_nh;
    e->route_tag = rip->entries[i].route_tag;
  }
  return sizeof(struct ripng_hdr) + rip->numEntries*sizeof(ripng_entry);
}