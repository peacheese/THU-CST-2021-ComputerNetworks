#include "lookup.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


std::vector<RoutingTableEntry> RoutingTable;

void update(bool insert, const RoutingTableEntry entry) {
  // TODO
  for (auto it = RoutingTable.begin(); it != RoutingTable.end(); it++){
    if (entry.addr == (*it).addr && entry.len == (*it).len) {
      if (insert) {
        (*it).if_index = entry.if_index;
        (*it).nexthop = entry.nexthop;
        return ;
      } 
      else {
        RoutingTable.erase(it);
        return ;
      }
    }
  }
  if (insert) 
    RoutingTable.push_back(entry);
  return ;
}

bool prefix_query(const in6_addr addr, in6_addr *nexthop, uint32_t *if_index) {
  // TODO
  int max_prefix = -1, max_len = -1;
  bool is_searched = false;
  for (auto it = RoutingTable.begin(); it != RoutingTable.end(); it++){
    int prefix = 0;
    for (int i = 0; i <= 15; i++) {
      if (((*it).addr.__in6_u.__u6_addr8[i] ^ addr.__in6_u.__u6_addr8[i]) == 0) {
        prefix += 8;
      }
      else {
        int len = 0;
        uint8_t target = (*it).addr.__in6_u.__u6_addr8[i] ^ addr.__in6_u.__u6_addr8[i];
        for (int i = 7; i >= 0; i--) {
          if ((target & (1 << i)) == 0) {
            len ++;
          }
          else 
            break;
        }
        prefix += len;
        break;
      }
    }
    if (prefix >= (*it).len && (prefix > max_prefix || (prefix == max_prefix && (*it).len > max_len))){
      is_searched = true;
      max_prefix = prefix;
      max_len = (*it).len;
      *nexthop = (*it).nexthop;
      *if_index = (*it).if_index;
    }
  }
  return is_searched;
}

int mask_to_len(const in6_addr mask) {
  // TODO
  for (int i = 15; i >= 0; i--) {
    if (mask.__in6_u.__u6_addr8[i] == 0) 
      continue;
    else {
      for (int j = i-1; j >= 0; j--) {
        if (mask.__in6_u.__u6_addr8[j] != 255)
          return -1;
      }
      int len = 0;
      uint8_t target = ~(mask.__in6_u.__u6_addr8[i]);
      while (true) {
        if (target % 2) {
          target >>= 1;
          len ++;
        }
        else {
          if (target == 0) return 8 - len + 8*i;
          else return -1;
        }
      }
    }
  }
  return 0;
}

in6_addr len_to_mask(int len) {
  // TODO
  in6_addr mask;
  int prefix = len / 8, left = len % 8;
  if (prefix > 0)
    for (int i = 0; i < prefix; i++)
      mask.__in6_u.__u6_addr8[i] = 255;
  if (15-prefix > 0)
    for (int i = prefix; i <= 15; i++)
      mask.__in6_u.__u6_addr8[i] = 0;
  if (left > 0) 
    mask.__in6_u.__u6_addr8[prefix] = (((1 << left)-1) << (8-left));
  return mask;
}
