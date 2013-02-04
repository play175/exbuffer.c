#include "exbuffer.c"

void recvHandle(unsigned char *rbuf,size_t len)
{
	printf("收到数据:%d\n",len);
	exbuffer_printHex(rbuf,len);
}

int main(int argc, char **argv)
{
	exbuffer_t* value;
	value = exbuffer_new();
	value->recvHandle = recvHandle;

	unsigned char buf[] = {0,2,3,4,0,1,5,0};
	exbuffer_put(value,(unsigned char*)buf,0,8);

	unsigned char buf2[] = {3};
	exbuffer_put(value,(unsigned char*)buf2,0,1);

	unsigned char buf3[] = {6,6,6};
	exbuffer_put(value,(unsigned char*)buf3,0,3);

	//printf("有效数据长度:%d\n",exbuffer_getLen(value));
	//printf("缓冲区长度:%d\n",value->bufferlen);
	printf("缓冲区:\n");
	exbuffer_dump(value);
	exbuffer_free(&value);

	//getchar();
	return EXIT_SUCCESS;
}
