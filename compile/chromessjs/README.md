

# chromessjs 须知

> 一款改编自[phantomjs](https://github.com/ariya/phantomjs)的无界面浏览器

## Problem

由于某些网站的js会检测到浏览器环境是否含有phantom关键字，导致访问失败。

因此，chromessjs只是简单粗暴把phantomjs源代码里边的所有phantom关键字替换掉而已

## SourceCode

- [chromessjs](https://github.com/SHAU-LOK/phantomjs/tree/kill_module), 分支*kill_module*

- 打包工具 [qtbase](https://github.com/SHAU-LOK/qtbase/tree/old_change_name), 分支*old_change_name*

- 打包工具 [qtwebkit](https://github.com/SHAU-LOK/qtwebkit/tree/change_old_name), 分支*chang_old_name*

- 打包工具 [phantomjs-3rdparty-win](https://github.com/SHAU-LOK/phantomjs-3rdparty-win/tree/change_old_name), 分支*change_old_name*


## Build

### Requirements

#### Linux requirements

On Debian-based distro (tested on Ubuntu 14.04 and Debian 7.0), run:

```
sudo apt-get install build-essential g++ flex bison gperf ruby perl \
  libsqlite3-dev libfontconfig1-dev libicu-dev libfreetype6 libssl-dev \
  libpng-dev libjpeg-dev python libx11-dev libxext-dev
```

On Fedora-based distro (tested on CentOS 6), run:

```
sudo yum -y install gcc gcc-c++ make flex bison gperf ruby \
  openssl-devel freetype-devel fontconfig-devel libicu-devel sqlite-devel \
  libpng-devel libjpeg-devel
```

### Compile

```
git clone git://github.com/SHAU-LOK/phantomjs.git
cd phantomjs
git checkout kill_module
git submodule init
git submodule update

python build.py
```

### Document

[Command Line Interface](http://phantomjs.org/api/command-line.html)

[Examples](http://phantomjs.org/examples/index.html)

[Quick Start](http://phantomjs.org/quick-start.html)

### Versions

- chromessjs-2.1.1  将代码里边的phantom关键字全部替换成chromess
- chromessjs-2.1.2-blacklist  增加url黑名单机制
- chromessjs-2.1.3-js-proxy  增加js资源文件使用另外的代理代理IP


