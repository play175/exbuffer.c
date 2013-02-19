exbuffer.c
==========

设计目标是一个纯C的网络协议缓冲器，该协议简单介绍：

协议包分为包头和包体：包长采用2个字节或者4个字节，用来表示本次数据包中包体的长度

接受到数据就存储在缓冲区，缓冲区动态扩展以保证可以足够存储。

当接收到一个以上完整的数据包就调用回调函数recvHandle。

```c
#include "exbuffer.h"

void recvHandle(unsigned char *rbuf,size_t len)
{
	printf("收到数据:%d\n",len);
	exbuffer_printHex(rbuf,len);
}


int main(int argc, char **argv)
{
	exbuffer_t* value;
	value = exbuffer_new();
	//value->headLen = 2;//设置包头长度，默认是2（2、4分别对应short和int，可表示65535/以及2~32次方长度的数据）
	value->recvHandle = recvHandle;

	//test ntohl
	//printf("%d\n",ntohl(1,EXBUFFER_LITTLE_ENDIAN));

	unsigned char buf[] = {0,2,3,4,0,1,5,0};
	exbuffer_put(value,(unsigned char*)buf,0,8);

	unsigned char buf2[] = {3};
	exbuffer_put(value,(unsigned char*)buf2,0,1);

	unsigned char buf3[] = {6,6,6};
	exbuffer_put(value,(unsigned char*)buf3,0,3);

	//printf("有效数据长度:%d\n",exbuffer_getLen(value));
	//printf("缓冲区长度:%d\n",value->bufferlen);
	printf("缓冲区:\n");
	exbuffer_dump(value,value->bufferlen);
	exbuffer_free(&value);
	//system("pause");
	//getchar();
	return EXIT_SUCCESS;
}

```

nodejs版本的exbuffer：https://github.com/play175/ExBuffer

对C不熟悉，第一次写C代码，可能很多不合理的地方，欢迎批评指正！

我的博客：http://yoyo.play175.com/
