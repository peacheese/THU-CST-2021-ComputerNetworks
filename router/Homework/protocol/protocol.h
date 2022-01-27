#include "common.h"
#include <stdint.h>

// disassemble 函数的返回值定义如下
// 如果同时出现多种错误，返回满足下面错误描述的第一条错误
enum RipErrorCode {
  // 没有错误
  SUCCESS = 0,
  // IPv6 头中 next header 字段不是 UDP 协议
  ERR_IP_NEXT_HEADER_NOT_UDP,
  // UDP 头中源端口号或者目的端口号不是 521(RIPng)
  ERR_BAD_UDP_PORT,
  // IPv6 头、UDP 头和实际的 RIPng 路由项的长度出现错误或者不一致
  ERR_LENGTH,
  // RIPng 的 Command 字段错误
  ERR_RIP_BAD_COMMAND,
  // RIPng 的 Version 字段错误
  ERR_RIP_BAD_VERSION,
  // RIPng 的 Zero（Reserved） 字段错误
  ERR_RIP_BAD_ZERO,
  // RIPng 表项的 Metric 字段错误
  ERR_RIP_BAD_METRIC,
  // RIPng 表项的 Prefix Len 字段错误
  ERR_RIP_BAD_PREFIX_LEN,
  // RIPng 表项的 Route Tag 字段错误
  ERR_RIP_BAD_ROUTE_TAG,
  // RIPng 表项的 Prefix 和 Prefix Len 字段不符合要求
  ERR_RIP_INCONSISTENT_PREFIX_LENGTH,
};

// RIPng header 定义
typedef struct ripng_hdr {
  // 1 表示 request，2 表示 response
  uint8_t command;
  // 应当为 1
  uint8_t version;
  // 应当为 0
  uint16_t zero;
} ripng_hdr;

// (1500-40-8-4)/20=72
#define RIP_MAX_ENTRY 72

// RIPng entry 定义
typedef struct {
  // 当 Metric=0xFF 时记录了 Nexthop；当 Metric!=0xFF 时记录了 Prefix
  in6_addr prefix_or_nh;
  // 网络字节序存储
  uint16_t route_tag;

  
  uint8_t prefix_len;
  uint8_t metric;
} ripng_entry;

typedef struct {
  uint32_t numEntries;
  // 1 表示 request，2 表示 response
  uint8_t command;
  // 不用存储 `version`，因为它总是 1
  // 不用存储 `zero`，因为它总是 0
  ripng_entry entries[RIP_MAX_ENTRY];
} RipPacket;

static const char *rip_error_to_string(RipErrorCode err) {
  switch (err) {
  case RipErrorCode::ERR_IP_NEXT_HEADER_NOT_UDP:
    return "IP next header field is not UDP";
  case RipErrorCode::ERR_BAD_UDP_PORT:
    return "UDP port is not 521";
  case RipErrorCode::ERR_LENGTH:
    return "Length is inconsistent";
  case RipErrorCode::ERR_RIP_BAD_COMMAND:
    return "Command field is bad";
  case RipErrorCode::ERR_RIP_BAD_VERSION:
    return "Version field is bad";
  case RipErrorCode::ERR_RIP_BAD_ZERO:
    return "Zero(Reserved) field is bad";
  case RipErrorCode::ERR_RIP_BAD_METRIC:
    return "Metric field is bad";
  case RipErrorCode::ERR_RIP_BAD_PREFIX_LEN:
    return "Prefix len field is bad";
  case RipErrorCode::ERR_RIP_BAD_ROUTE_TAG:
    return "Route tag field is bad";
  case RipErrorCode::ERR_RIP_INCONSISTENT_PREFIX_LENGTH:
    return "Prefix field is inconsistent with Prefix len field";
  default:
    return "Unknown error code";
  }
}

/*
  你需要从 IPv6 packet 中解析出 RipPacket 结构体，也要从 RipPacket
  结构体构造出对应的 IPv6 packet。 由于 RIPng 本身不记录表项的个数，需要从 IPv6
  header 的长度中推断，所以在 RipPacket 中额外记录了个数。
*/

/**
 * @brief 从接受到的 IPv6 packet 解析出 RIPng 协议的数据
 * @param packet 接受到的 IPv6 包
 * @param len 即 packet 的长度
 * @param output 把解析结果写入 *output
 * @return 如果输入是一个合法的 RIPng 包，把它的内容写入 RipPacket 并且返回
 * RipErrorCode::SUCCESS；否则按照要求返回 RipErrorCode 的具体类型
 *
 * 你需要按照以下顺序检查：
 * 1. len 是否包括一个的 IPv6 header 的长度。
 * 2. IPv6 Header 中的 Payload Length 加上 Header 长度是否等于 len。
 * 3. IPv6 Header 中的 Next header 字段是否为 UDP 协议。
 * 4. IPv6 Header 中的 Payload Length 是否包括一个 UDP header 的长度。
 * 5. 检查 UDP 源端口和目的端口是否都为 521。
 * 6. 检查 UDP header 中 Length 是否等于 UDP header 长度加上 RIPng header
 * 长度加上 RIPng entry 长度的整数倍。
 * 7. 检查 RIPng header 中的 Command 是否为 1 或 2，
 * Version 是否为 1，Zero（Reserved） 是否为 0。
 * 8. 对每个 RIPng entry，当 Metric=0xFF 时，检查 Prefix Len
 * 和 Route Tag 是否为 0。
 * 9. 对每个 RIPng entry，当 Metric!=0xFF 时，检查 Metric 是否属于
 * [1,16]，并检查 Prefix Len 是否属于 [0,128]，是否与 IPv6 prefix 字段组成合法的
 * IPv6 前缀。
 */
RipErrorCode disassemble(const uint8_t *packet, uint32_t len,
                         RipPacket *output);

/**
 * @brief 从 RipPacket 的数据结构构造出 RIPng 协议的二进制格式
 * @param rip 一个 RipPacket 结构体
 * @param buffer 一个足够大的缓冲区，你要把 RIPng 协议的数据写进去
 * @return 写入 buffer 的数据长度
 *
 * 在构造二进制格式的时候，你需要把 RipPacket 中没有保存的一些固定值补充上，
 * 包括 Version 和 Zero 字段。
 * 你写入 buffer 的数据长度和返回值都应该是四个字节的 RIPng 头，加上每项 20
 * 字节。 需要注意一些没有保存在 RipPacket 结构体内的数据的填写。
 */
uint32_t assemble(const RipPacket *rip, uint8_t *buffer);