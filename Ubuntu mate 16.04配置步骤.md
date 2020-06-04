# 1. 换源

```
sudo vim /etc/apt/sources.list
```

添加清华大学软件源，把下面的内容复制到sources.list中，替换已有内容：

```
deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-updates main restricted universe multiverse
deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-updates main restricted universe multiverse

deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-security main restricted universe multiverse
deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-security main restricted universe multiverse

deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-backports main restricted universe multiverse
deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial-backports main restricted universe multiverse

deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial main universe restricted
deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial main universe restricted
```

```
sudo apt-get update
```

# 2.安装ROS

## 2.1 添加ROS源

```
sudo sh -c 'echo "deb http://mirrors.tuna.tsinghua.edu.cn/ros/ubuntu/ $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
```

```
sudo apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
```

```
sudo apt-get update
```

## 2.2 ROS配置

```
sudo apt-get install -y ros-kinetic-desktop-full
sudo rosdep init
rosdep update
echo "source /opt/ros/kinetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
sudo apt-get install python-rosinstall python-rosinstall-generator python-wstool build-essential
```
执行`rosdep update`时，若出现：

```
ERROR: error loading sources list:         <urlopen error <urlopen error [Errno 111] Connection refused>
```

执行以下步骤：

```
sudo vim /etc/resolv.conf
```

将原有的内容注释，并添加以下两行：

```
nameserver 8.8.8.8 #google域名服务器
nameserver 8.8.4.4 #google域名服务器
```

最后`sudo apt-get update`后重新执行`rosdep update`

## 2.3 kobuki开机启动

```
sudo apt-get install -y ros-kinetic-robot-upstart 
cd /opt/ros/kinetic/share
rosrun robot_upstart install turtlebot_bringup/launch/minimal.launch
sudo systemctl daemon-reload && sudo systemctl start turtlebot
```

> Tip: `rosrun robot_upstart install`后面必须直接更package的名字，所以，要进入包所在的文件里面执行该命令

# 3. OpenCV

## 3.1 配置交换分区

防止编译时内存不够用

https://blog.csdn.net/u010429286/article/details/79219230

## 3.2 编译OpenCV

源码：[OpenCV 3.4](https://opencv.org/opencv-3-4/)

进入网页，点击`Source`下载

```
unzip opencv-3.4.0.zip
cd opencv-3.4.0/
mkdir build
cd build 

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..

make -j4
sudo make install #安装
```

# 3. OpenCV获取USB摄像头数据卡顿

[用opencv调用USB摄像头帧率低问题解释](https://blog.csdn.net/ah_107/article/details/102844130)

# 4. 打包qt程序

## 4.1检查库是否可以找到

若执行

```
ldd ros_video_ctrl
```

出现OpenCV库没找到，那么

```
sudo vim /etc/ld.so.conf.d/opencv.conf
```

填入如下内容：

```
/usr/local/lib
```

该路径为OpenCV库的路径

```
sudo ldconfig
```

## 4.2开始打包

将qt编译好的release版本可执行程序和下面脚本放在一起，然后执行下面脚本

```
#!/bin/sh  
exe="video_server" #你需要发布的程序名称
des="./program" #创建文件夹的位置
deplist=$(ldd $exe | awk '{if (match($3,"/")){ printf("%s "),$3 } }')  
cp $deplist $des
```

# 5. 开机自启动

## 5.1 开机启动

### 5.1.1 方法一（推荐）

一般而言，qt开发的GUI界面程序，直接把`.desktop`文件拷贝到

`~/.config/autostart/`文件夹内下，

或者拷贝到`/etc/xdg/autostart/`文件夹下，

**记得加777权限**

### 5.1.2 方法二（麻烦）

编辑 `~/.profile`文件

```
vim ~/.profile
```

在文件最后加入脚本命令：

* Tip: 最后的`&`不可省，`video_server`是qt已发布好的可执行文件

```
export DISPLAY=:0 && bash -i -c /home/pi/work/DEBIAN/program/video_server &
```

这样即可，但这时候，通过SSH登录Ubuntu时，没启动一个SSH就会执行以下`.profile`文件，造成启动一次程序，这个时候需要处理一下，让上述脚本命令只执行一次，将上述命令修改为下面的：

```
if [ -f ".running" ]; then
        echo "do nothing"
else
        touch .running
        echo `date +"%Y-%m-%d %H:%M:%S"` >> .running
        export DISPLAY=:0 && bash -i -c /home/pi/work/DEBIAN/program/video_server &
fi
```

该逻辑为：

在执行`~/.profile`文件时，先检测是否有`.running`文件，没有该文件则说明是刚开机，可以执行启动脚本，并创建该文件，下次通过SSH接入Ubuntu执行`.profile`文件时，则不会再次执行启动程序脚本。

## 5.2 关机或重启时执行脚本（若选用5.1.1方式，下面忽略）

该功能是为了删除`.running`文件，下次开机时可以顺利执行启动脚本

## 5.2.1 创建systemd服务

参考文章：[Centos7关机和重启前执行自定义脚本](https://blog.csdn.net/a363344923/article/details/86533374)

```
sudo vim /usr/lib/systemd/system/stopSrv.service
```

加入下面内容：

```
[Unit]
Description=close services before reboot and shutdown
DefaultDependencies=no
Before=shutdown.target reboot.target halt.target
# This works because it is installed in the target and will be
#   executed before the target state is entered
# Also consider kexec.target

[Service]
Type=oneshot
ExecStart=/home/pi/.clean.sh  #your path and filename

[Install]
WantedBy=halt.target reboot.target shutdown.target
```

在`~/`文件夹下创建脚本`.clean.sh`

```
vim ~/.clean.sh
```

```
#!/bin/bash
rm -rf /home/pi/.running
```

给该脚本执行权限

```
chmod a+x ~/.clean.sh
```

## 5.2.2 启动服务

```
systemctl enable stopSrv
```









