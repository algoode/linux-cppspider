# linux-cppspider



## 项目描述

基于linux平台下，使用C/C++语言进行编写，实现网页的爬取，分析和保存，其中用到了Linux多路复用技术（Epoll模型），socket，多线程、正则表达式、守护进程、Linux动态库加载等Linux系统开发技术。



## 运行方法

- make
- ./spider
- 清除：make clean



## 文件清单

spider.cpp -- 主控文件

confparser -- 加载配置文件

DNSManager -- DNS解析，获得ip地址

url.cpp -- url类

urlManager -- url管理类

urlsocket -- http通信，url提取

wrap -- socket过程封装

savehtml -- 保存模块（暂时只做了html保存模块，后面会做个sava基类，保存各种文件



## 爬虫系统模块



### 控制模块



### 下载模块



### 解析模块



### 保存模块

