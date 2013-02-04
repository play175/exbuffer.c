exbuffer.c
==========

设计目标是一个纯C的网络协议缓冲器，该协议简单介绍：
协议包分为包头和包体：包长采用2个字节或者4个字节，用来表示本次数据包中包体的长度
接受到数据就存储在缓冲区，缓冲区动态扩展以保证可以足够存储。
当接收到一个以上完整的数据包就调用回调函数recvHandle。

nodejs版本的exbuffer：https://github.com/play175/ExBuffer

我的博客：http://yoyo.play175.com/
