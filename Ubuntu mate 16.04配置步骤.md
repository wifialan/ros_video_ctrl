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

## 1. 添加ROS源

```
sudo sh -c 'echo "deb http://mirrors.tuna.tsinghua.edu.cn/ros/ubuntu/ $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
```

```
sudo apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
```

```
sudo apt-get update
```

## 2. ROS配置

```
sudo apt-get install -y ros-kinetic-desktop-full
sudo rosdep init
rosdep update
echo "source /opt/ros/kinetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
sudo apt-get install python-rosinstall python-rosinstall-generator python-wstool build-essential
```

sudo sh -c 'echo "deb http://mirrors.tuna.tsinghua.edu.cn/ros/ubuntu/ $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'



# 3. OpenCV

## 1. 配置交换分区

防止编译时内存不够用

https://blog.csdn.net/u010429286/article/details/79219230

# 2. OpenCV获取USB摄像头数据卡顿

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



```
#!/bin/sh  
exe="video_server" #你需要发布的程序名称
des="./program" #创建文件夹的位置
deplist=$(ldd $exe | awk '{if (match($3,"/")){ printf("%s "),$3 } }')  
cp $deplist $des
```

# 5. 开机自启动

## 5.1 开机启动

一般而言，qt开发的GUI界面程序，直接把`.desktop`文件拷贝到`~/.config/autostart/`文件夹内就行，记得加777权限

但由于包含ros环境，在执行前需要加`bash -i -c`，这种情况在拷贝在按照上述办法就启动不起来了，需要登录界面后用脚本启动：

编辑 `~/.profile`文件即可

```
vim ~/.profile
```

在文件最后加入脚本命令：

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

## 5.2 关机或重启时执行脚本

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









