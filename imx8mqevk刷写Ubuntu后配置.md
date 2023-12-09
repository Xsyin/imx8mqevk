# imx8mq-evk ubuntu 正常启动后配置

1 根分区内存显示只有3G

```
# cfdisk /dev/mmcblk0
//调整分区/dev/mmcblk0p2为14.6G
# resize2fs -p /dev/mmcblk0p2
```

2 .eth0网卡配置和挂载内核, 写入/etc/init.d/netpl中

```
// ifconfig看不见eth0执行
dhclient eth0
mount -t vfat /dev/mmcblk0p1 /mnt
```

3. 修改ssh配置

```
PermitRootLogin yes
```

4. 国内清华源：

   ```
   deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic main restricted universe multiverse
   # deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic main restricted universe multiverse
   deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-updates main restricted universe multiverse
   # deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-updates main restricted universe multiverse
   deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-backports main restricted universe multiverse
   # deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-backports main restricted universe multiverse
   deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-security main restricted universe multiverse
   # deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ bionic-security main restricted universe multiverse
   
   ```


5. 安装docker

```
$ apt-get install  ca-certificates  curl gnupg lsb-release
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
$ echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" |  tee /etc/apt/sources.list.d/docker.list > /dev/null
$ apt-get update
$ apt-get install docker-ce docker-ce-cli containerd.io
```

6. 镜像

```
# https://cr.console.aliyun.com/cn-hangzhou/instances/mirrors
sudo mkdir -p /etc/docker
sudo tee /etc/docker/daemon.json <<-'EOF'
{
  "registry-mirrors": ["https://lq4u41xh.mirror.aliyuncs.com"]
}
EOF
sudo systemctl daemon-reload
sudo systemctl restart docker
```

7. 容器启动提速

```
apt install haveged
systemctl start haveged
systemctl enable haveged

vim /etc/default/haveged
# 将其中的参数改为：DAEMON_ARGS="-d 32 -w 1024"
# 详见 https://zhuanlan.zhihu.com/p/57007892

reboot
```

8. 生成benchmark容器

```
# docker run -it ubuntu bash
docker 容器内操作：
# apt update
# apt install git
# apt install build-essential
# git clone https://ghproxy.com/https://github.com/kdlucas/byte-unixbench.git   // 下载byte-unix, 国内源
# cd byte-unixbench/UnixBench
# make
# ./Run              // 耗时很久
导出容器：
docker container ls -a
docker export 7691a814370e > benchmark.tar
cat benchmark.tar | docker import - benchmark-test
```

















