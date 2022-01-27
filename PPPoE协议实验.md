<center>
 <h3>
   PPPoE 协议实验报告
  </h3> 
</center>

<center>
  甘乔尹 计92 2019011240
</center>

#### 一、报文捕获

**1、PADS 报文**

<img src="/Users/ganqiaoyin/Library/Containers/com.tencent.xinWeChat/Data/Library/Application Support/com.tencent.xinWeChat/2.0b4.0.9/36c8b48ccd52bfdbf6d631504531d199/Message/MessageTemp/9e20f478899dc29eb19741386f9343c8/Image/4821636391664_.pic_hd.jpg" alt="4821636391664_.pic_hd" style="zoom:50%;" />

##### 2、PPP-CHAP response 报文 

<img src="/Users/ganqiaoyin/Library/Containers/com.tencent.xinWeChat/Data/Library/Application Support/com.tencent.xinWeChat/2.0b4.0.9/36c8b48ccd52bfdbf6d631504531d199/Message/MessageTemp/9e20f478899dc29eb19741386f9343c8/Image/4831636391718_.pic_hd.jpg" alt="4831636391718_.pic_hd" style="zoom:50%;" />

**其中加密摘要字段为高亮部分。**

##### 3、PPP-IPCP request 报文

<img src="/Users/ganqiaoyin/Library/Containers/com.tencent.xinWeChat/Data/Library/Application Support/com.tencent.xinWeChat/2.0b4.0.9/36c8b48ccd52bfdbf6d631504531d199/Message/MessageTemp/9e20f478899dc29eb19741386f9343c8/Image/4841636391782_.pic_hd.jpg" alt="4841636391782_.pic_hd" style="zoom:50%;" />

#### 二、思考题

**1、给出 Wireshark 捕获的 PADS 报文、PPP-CHAP response 报文、PPP-IPCP request(携带分配后地址的)报文的截图，并指出 PPP-CHAP response 中的加密摘要字段。**

答：截图如上所示，PPP-CHAP response 中的加密摘要字段为第二幅图中的高亮部分。

**2、在通常的以太网(MTU=1500)上，使用 PPPoE 协议传递 UDP 数据报(IP 头不包含可选字段)。每个报文可以携带的上层应用的数据容量至多为多少? 解释计算过程。**

答：PPPoE 头占 $(4+4+8+16+16)/8=6~Byte$，
		PPP 帧头部可以减少到 2 或 4 个字节，所以最少为 $2~Byte$，
		IP占 $20~Byte$ ，UDP 头占 $8~Byte$，
		所以每个报文可携带的上层应用的数据容量至多为 $1500-6-2-20-8=1464~Byte$。

**3、观察捕捉的报文可以发现，用 PPPoE 封装的 PPP 帧头部不包含标志、地址和控制字段，为什么?**

答：PPP 帧的标志、地址和控制字段值为常量，所以这些字段就没有任何实际的含义，传输时把这些字段舍弃，还可以节约空间，提高传输效率。

**4、PPP LCP 协商中的 MRU 值受到哪些因素的影响?**

答：受到配置 MTU、本端 MRU、对端 MRU 的影响，协商后取三者的最小值。

**5、查阅相关资料，说明应该如何在 PPPoE 链路上进行 IPv6 协议的配置，并给出涉及到的协议名称、相关 RFC 编号。(本题不止一种方案，言之有理即可)**

答：IPv6 的 PPP 拨号过程被分为五个阶段：发现阶段、PPP协商阶段、IPv6 地址配置阶段、数据传输阶段、PPPoE 结束阶段。首先在发现阶段，发送 PADI、PADO、PADR、PADS 报文；在 PPP 协商阶段，PPP协商和普通的 PPP 协商方式一致，主要区别是在网络控制协议协商阶段，使用 IPv6CP( $\rm RFC5072$) 进行Interface ID的协商
；IPv6地址配置阶段需要得到全球单播IPv6地址。因为 IPv6CP 协议并没有涉及对 IPv6 的其它参数配置，可能需要和其他协议联动。例如 $\rm RFC4862$ (NDRA方式) 或 $\rm RFC3315$ (DHCPV6方式) 。后两个阶段和 IPv4 的过程类似，只是数据包类型发生改变。

**6、你认为 PPPoE 有哪些优点和缺点?(开放式问题，言之有理即可)**

答：优点：PPPoE 可以根据时间或流量来计算PPPoE，计费方式灵活；PPPoE 分配给用户的 IP 地址可以定位用户在网络中的动态。
		缺点：PPP 协议需要再次封装，效率低；PPPoE 的广播流量消耗大，对网络性能要求高。