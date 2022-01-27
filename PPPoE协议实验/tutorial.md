#  PPPoE 实验补充说明

> 编写：陈嘉杰、杨松涛、陈晟祺
>
> 参与测试：谭闻德、陈晟祺、靳子豪
>
> Ref: [基于 Linux 环境的 PPPoE 服务器搭建](https://zhuanlan.zhihu.com/p/41499761)

下列操作基于 Ubuntu 18.04，并在 Debian 10/11、Ubuntu 20.04 上均经过测试。

## 网络拓扑说明

你需要准备两个直接由以太网相连的结点来进行本实验，分别作为服务端和客户端。我们推荐以下的选择：

* 客户端为 Windows / Linux 宿主，服务端为 Linux 虚拟机：需要使用“Host-Only”模式配置虚拟机网络
* 客户端、服务端均为 Linux 虚拟机：也需要使用“Host-Only”模式
* 客户端为 Windows / Linux 物理机，服务端为 Linux 物理机：直接使用网线连接两个结点
* 客户端、服务端处于同一台主机的两个不同的 netns 中，使用 veth 相连：此方法配置较为复杂，但无需使用虚拟机，原理及具体配置可参见 [netns 使用方法](https://lab.cs.tsinghua.edu.cn/router/doc/local_test/#netns)。

通常 WSL2 也可以作为上面提到的“Linux 虚拟机”工作，但也可能遇到其他问题，请谨慎选择。

## PPPoE Linux Server 配置说明

1. 安装 PPPoE，执行 `sudo apt install pppoe`。

2. 配置 PPPoE。编辑 **/etc/ppp/options** 文件，删除 `#+chap` 行首的 `#`。

![](pic/s01.png)

3. 配置 PPPoE。编辑 **/etc/ppp/pppoe-server-options** 文件，写入如下内容：

```
require-chap
lcp-echo-interval 60
lcp-echo-failure 5
logfile /var/log/pppd.log
```

![](pic/s02.png)

4. 配置 PPPoE。编辑 **/etc/ppp/chap-secrets** 文件，在最后一行写入如下内容（也可以修改用户名和密码）：

```
test * 123456 *
```

![](pic/s03.png)

5. 启动 PPPoE Server。执行 `sudo pppoe-server -I enp0s8 -L 10.0.0.1 -R 10.0.0.2 -N 20`。注意，这里的 `enp0s8` 请使用本地的实际网卡名称（和 Client 相连的那一个），可以通过 `ip a` 进行查看。如果不了解参数的含义，可以通过 `man pppoe-server` 查看。

## PPPoE Linux Client 连接方法

### GUI

通常 Linux GUI 的网络配置中均可添加 PPPoE 连接，类型通常类似于 “PPPoE”、“ADSL”。

### 命令行

1. 安装 pppoeconf，执行 `sudo apt install pppoeconf`。
2. 启动 pppoeconf，执行 `sudo pppoeconf`。

![](pic/c011.png)

3. Popular Options。选否，输入用户名 `test` 和密码 `123456`。

![](pic/c0121.png)

![](pic/c0122.png)

![](pic/c0123.png)

4. Use Peer DNS。选是。

![](pic/c013.png)

5. Limited MSS Problem。选是。

![](pic/c014.png)

6. Start connection at boot time。选否。

![](pic/c015.png)

7. Establish a Connection。选否。

![](pic/c016.png)

8. 启动 PPPoE 连接，执行 `sudo pon dsl-provider`。可以执行 `sudo plog` 来查看日志。

![](pic/c02.png)

9. 此时应该可以在 Server 端观察到这个连接。

![](pic/c03.png)

![](pic/c04.png)

10. 执行 `sudo poff` 断开连接。（可以多执行几轮来观察 IP 分配变化。）

## PPPoE Windows 10 Client 连接方法

1. 进入网络和 Internet 设置。
2. 在左边选择拨号。

![](pic/c11.png)

3. 点击**设置新连接**。

![](pic/c12.png)

4. 点击**下一步**。

![](pic/c13.png)

5. 点击**设置新连接**。

![](pic/c14.png)

6. 点击**宽带（PPPoE）**。

![](pic/c15.png)

7. 输入密码，点**连接**。

![](pic/c16.png)

8. **跳过**正在测试 Internet 连接。
9. 点击**关闭**。

![](pic/c17.png)

10. 点击**宽带连接**，在菜单中点击**连接**。

![](pic/c18.png)

11. 重新输入用户名密码，忽略下方的_用户名或密码不正确_（如有兴趣，可以查阅资料了解原因）。
12. 连接成功后如图所示。

![](pic/c19.png)

13. 点击右边的**更改适配器选项**。

![](pic/c20.png)

14. 找到宽带连接 WAN Miniport (PPPOE)。
15. 右键，点击**状态**。

![](pic/c21.png)

16. 点**详细信息**，可以看到获取的 IPv4 地址。

![](pic/c22.png)

