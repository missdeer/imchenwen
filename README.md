# imchenwen（中文名：某橙）

可以简单地认为这是[Bilibili Mac Client](https://typcn.com/legacy/bilimac/)的跨平台版本，目前支持Windows和macOS版本，Linux应该也能支持，只是我手上没有机器。

所以只支持B站吗？不是的，通过外部解析器，大概支持几十上百个站吧。

某橙关注播放和观看，可以使用程序内置的播放器，调用外部其他多媒体播放器，或者投屏到DLNA播放器（比如Chromecast之类的）进行播放。

所以不能下载视频！不能下载视频！不能下载视频！

## [功能列表](https://bitbucket.org/missdeer/imchenwen/wiki/Features)

* 解析视频真实地址，解析VIP视频地址。
* 使用某橙内置播放器，在Windows和macOS上使用硬件解码，降低功耗，减少机器发热，延长续航时间。
* 使用外部多媒体播放器。
* 投屏到DLNA播放器。目前只在一款叫“嘉视丽”的投屏器上测试过。

## [使用说明](https://bitbucket.org/missdeer/imchenwen/wiki/Usage)

### 下载安装

* 从 https://minidump.info/imchenwen/download/ 下载对应版本的某橙，目前有32位Windows、64位Windows以及macOS版本，仅在Windows7、Windows10、macOS 10.13、macOS 10.14上开发测试，其他平台不保证能正常工作。

#### Windows系统

* 从 https://www.python.org/ 下载安装Python 3
* 从 https://github.com/iawia002/annie/releases 下载最新的Windows版annie。
* 从 https://ffmpeg.zeranoe.com/builds/ 下载最新的Windows版ffmpeg，如果你不使用投屏功能，可以跳过这一步。
* 运行如下命令安装解析器：

```
pip3 install --upgrade ykdl you-get youtube-dl
```

#### macOS系统

运行如下命令安装解析器：

```
brew install python3 annie mpv youtube-dl ffmpeg
pip3 install --upgrade ykdl you-get
```

### 初次使用设置

运行某橙，选择菜单项“偏好设置”:

![preferences](https://cdn.jsdelivr.net/gh/missdeer/blog@gh-pages/media/2018-12-01/aboutmenu.png)

“解析器”页面，分别将`ykdl`，`you-get`，`youtube-dl`，`annie`，`ffmpeg`的路径填入对应的编辑框，如下所示：

![resolver](https://cdn.jsdelivr.net/gh/missdeer/blog@gh-pages/media/2018-12-01/configuration.png)

其中`VIP视频解析器订阅`和`快捷方式订阅`可以参照`https://cdn.jsdelivr.net/gh/missdeer/imchenwen@master/vip.txt`和`https://cdn.jsdelivr.net/gh/missdeer/imchenwen@master/websites.xml`自行定义，并放到网上，通过网络地址（比如https://... ）提供。

### 解析并播放

从菜单“快捷方式”或地址栏输入视频网站地址，打开某一个视频的网页，选择主菜单“文件”-“解析并使用播放器播放”，或者快捷键`Ctrl+P`（macOS系统则是`Cmd+P`），弹出“播放器和视频资源”对话框：

![选择播放器和视频资源](https://cdn.jsdelivr.net/gh/missdeer/blog@gh-pages/media/2018-11-13/resolved.png)

选择自己想用的播放器，以及想播放的视频资源（主要是不同分辨率），点击对话框右下方“播放”按钮，即会调用相应播放器播放视频，某橙内置支持硬件解码的播放器，兼容性好，功耗低，发热少，续航长，但功能少：

![某橙内置支持硬件解码的播放器](https://cdn.jsdelivr.net/gh/missdeer/blog@gh-pages/media/2018-11-13/builtinplayer.jpg)

外部播放器强烈推荐[mpv](https://mpv.io)，与某橙内置播放器相同内核，功能多，操作简便。

### 解析VIP并播放

打开一个VIP视频网页，选择主菜单“文件”-“解析VIP并使用播放器播放”，或者快捷键`Ctrl+Shift+V`（macOS系统则是`Cmd+Shift+V`），大多数当前热播VIP资源都能解析出一个到几个地址，之后跟“解析并播放”相同操作。

### 播放电视直播

从工具栏最右侧按钮下拉菜单选择对应的电视直播源，点击菜单项后，弹出“播放器”对话框选择自己想用的播放器，**注意：直播不支持DLNA投屏，目前技术未突破**：

![电视直播](https://cdn.jsdelivr.net/gh/missdeer/blog@gh-pages/media/2018-11-13/livetv.png)

## [常见问题](https://bitbucket.org/missdeer/imchenwen/wiki/FAQ)

### 这玩意儿是干嘛用的？

A： 可能是个没啥用的玩意儿。对我来说，家里妹子拿手机和iPad看视频看得头疼，我就想让她在电视机上看，解决头疼的问题，这是刚需。所以这就是个让人可以不在网页/手机/平板上看网络视频的软件。你可以：

1. 在电视机上看视频（不是所有电视机都是智能电视，不是所有视频网站/app都支持投屏
2. 在电脑上用自己喜欢的播放器看视频（自己喜欢就是最大的理由
3. 在电脑上用硬解码，低功耗的播放器看视频，苹果家的笔记本可以少发热，延长续航时间
4. 实在编不下去了……

### 某橙是开源的吗？免费的吗？

A： 不开源。目前免费使用。

### 使用某橙是否侵犯视频版权？

A： 某橙并不下载、存储、分发、传播版权视频资源，视频资源版权仍然归版权方所有，所有功能由第三方提供实现，视频资源由各视频网站提供，解析功能由各解析器提供，某橙就是一个网页浏览器和多媒体播放器的结合体。如果说某橙侵犯了谁的版权，那么世界上所有的网页浏览器（IE、Chrome、Firefox、Opera等）和所有的多媒体播放器（Windows Media Player、VLC、MPlayer、mpv等）都侵犯了ta的版权，这是不现实的。

### 某橙设置好复杂？

A： 一次设置，终生使用（雾。某橙自己不提供解析功能，需要最终用户自己添加相应的解析器和解析工具，所有选择由用户自己决定。

### 如何下载视频？

A： 某橙关注播放和观看视频，目前没有下载功能。

### 我发现了一个bug？

A： 到 https://bitbucket.org/missdeer/imchenwen/issues/new 记录bug出现时播放的网址以及bug的现象。

## [技术支持](https://bitbucket.org/missdeer/imchenwen/wiki/Support)

加入Telegram群： https://t.me/joinchat/A3aHAwDk6efjf6t6ZDNRJw
