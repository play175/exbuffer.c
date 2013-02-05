/*!
 * exbuffer.c
 * yoyo 2013 https://github.com/play175/exbuffer.c
 * new BSD Licensed
 */
 #include <cstdio>
#include <cstdlib>
//#include <cstring>
#include <cmath>

#ifdef WIN32
#include <winsock.h>
#endif

#ifdef _LINUX
#include <arpa/inet.h>
#endif

#ifndef EXBUFFER_H
#define EXBUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!< 每次扩展缓冲区的单位(字节) */
#define EXTEND_BYTES 512

/*!< 字节序定义 */
enum exbuffer_endian
{
	EXBUFFER_BIG_ENDIAN,
	EXBUFFER_LITTLE_ENDIAN
};

/*!< exbuffer_t数据结构 */
typedef struct exbuffer_value
{
	unsigned char headLen;/*!< 包头长度：2/4 */
    exbuffer_endian endian;/*!<  */
    size_t readOffset;/*!<  */
    size_t putOffset;/*!<  */
    size_t dlen;/*!< 本次数据包长度 */
	unsigned char* buffer;/*!< 缓冲区 */
	size_t bufferlen;/*!< 缓冲区长度 */
	void (*recvHandle)(unsigned char*, size_t);/*!< 接收到数据时的回调函数指针 */
} exbuffer_t;

/*!< 创建一个新的exbuffer_t数据结构 */
exbuffer_t* exbuffer_new(unsigned char headLen = 2,exbuffer_endian endian = EXBUFFER_BIG_ENDIAN,size_t bufferlen = 512)
{
	exbuffer_t* value;
	value =  (exbuffer_t *)malloc (sizeof (exbuffer_t));
	value->bufferlen = bufferlen;
	value->headLen = 2;
	value->endian = endian;
	value->readOffset = 0;
	value->putOffset = 0;
	value->dlen = 0;
	value->recvHandle = NULL;
	
	value->buffer = (unsigned char*)malloc(value->bufferlen);
	//memset(value->buffer,0,value->bufferlen);
	
	return value;
};

/*!< 释放exbuffer_t */
void exbuffer_free(exbuffer_t** value)
{
	free ((*value)->buffer);
	(*value)->buffer = NULL;
	(*value)->recvHandle = NULL;	
	free (*value);
	(*value) = NULL;
};

/*!<  */
void exbuffer_printHex(unsigned char* bytes,unsigned short len = 50)
{
	unsigned short iLoop;
	for(iLoop = 0;iLoop < len;iLoop++)
    {
        printf("%02x ",bytes[iLoop]);
    }
	printf("\n");
};

/*!< 打印exbuffer_t中的缓冲区内存(最多打印50个字节) */
void exbuffer_dump(exbuffer_t* value)
{
	unsigned short len = 50;
	if(value->bufferlen<len)len = value->bufferlen;
	exbuffer_printHex(value->buffer,len);
};

/*!< 获取缓冲区有效数据长度 */
size_t exbuffer_getLen(exbuffer_t* value)
{
   if(value->putOffset>= value->readOffset){ // ------******-------
		return value->putOffset - value->readOffset;
	}
	return value->bufferlen - value->readOffset + value->putOffset; //***-------*********
};

void exbuffer_proc(exbuffer_t* value)
{
	unsigned short count = 0;
	size_t i;
	while(1)
	{
		count++;
		if(count>1000)
		{
			fprintf(stderr,"count>1000\n");
			break;//1000次还没读完??
		}
		if(value->dlen == 0)
		{
			//printf("value->dlen == 0\n");
			//printf("有效长度:%d\n",exbuffer_getLen(value));
			if(exbuffer_getLen(value) < value->headLen)
			{
				//printf("连包头都读不了:%d\n",value->dlen);
				break;//连包头都读不了
			}
			unsigned char *bytes = (unsigned char *)malloc(value->headLen);
			if(value->bufferlen - value->readOffset >= value->headLen)//***********[**]
			{
				//printf("***********[**]\n");
				for(i=0;i<value->headLen;i++)
				{
					bytes[i] = value->buffer[value->readOffset+i];
				}
				value->readOffset += value->headLen;
				//printf("value->readOffset=%d\n",value->readOffset);
			}
			else //*]**----------********[*
			{
				//printf("*]**----------********[*\n");
				unsigned char rlen = 0;
				for(i = 0;i<(value->bufferlen - value->readOffset);i++)
				{
					bytes[i] = value->buffer[value->readOffset+i];
					rlen++;
				}
				value->readOffset = 0;
				for(i = 0;i<(value->headLen - rlen);i++)
				{
					bytes[rlen+i] = value->buffer[value->readOffset+i];
				}
				value->readOffset += (value->headLen - rlen);
			}
			//解析包体长度
			if(value->headLen==2)
			{
				union HeadBytesS
				{
					unsigned char bytes[2];
					unsigned short val;
				} headS;
				headS.bytes[0] = bytes[0];
				headS.bytes[1] = bytes[1];
				if(value->endian == EXBUFFER_BIG_ENDIAN)
				{
					value->dlen = ntohs(headS.val);//把网络字节序换成主机字节序
				}
				else
				{
					value->dlen = headS.val;
				}
			}
			else
			{
				union HeadBytesL
				{
					unsigned char bytes[4];
					unsigned long val;
				} headL;
				headL.bytes[0] = bytes[0];
				headL.bytes[1] = bytes[1];
				headL.bytes[2] = bytes[2];
				headL.bytes[3] = bytes[3];
				if(value->endian == EXBUFFER_BIG_ENDIAN)
				{
					value->dlen = ntohl(headL.val);//把网络字节序换成主机字节序
				}
				else
				{
					value->dlen = headL.val;
				}
			}
			//exbuffer_printHex(bytes,2);
			free(bytes);
			bytes = NULL;
			//printf("value->dlen=%d\n",value->dlen);
			//printf("value->readOffset=%d\n",value->readOffset);
		}

		//printf("value->dlen:%d\n",value->dlen);
		//读包体
		if(exbuffer_getLen(value) >= value->dlen)
		{
			unsigned char *dbuff = (unsigned char *)malloc(value->dlen);
			if(value->readOffset + value->dlen > value->bufferlen)//***---*****
			{
				size_t len1 = value->bufferlen - value->readOffset;
				if (len1 > 0) 
				{
					memcpy(dbuff,value->buffer + value->readOffset,len1);
				}
				value->readOffset = 0;

				size_t len2 = value->dlen - len1;
				memcpy(dbuff + len1,value->buffer + value->readOffset,len2);
				value->readOffset += len2;
			}
			else
			{
				memcpy(dbuff,value->buffer + value->readOffset,value->dlen);
				value->readOffset += value->dlen;
			}
			size_t dlen = value->dlen;
			value->dlen = 0;
			
			if(value->recvHandle == NULL)
			{
				printf("receive packet:%d\n",dlen);
				//exbuffer_printHex(dbuff,dlen);
			}
			{
				value->recvHandle(dbuff,dlen);
			}
			free(dbuff);
			dbuff = NULL;

			if (value->readOffset == value->putOffset) 
			{
				break;
			}			
		}
		else 
		{
			break;
		}
	}
}

/*!< 推送一段数据进去 */
void exbuffer_put(exbuffer_t* value, unsigned char* buffer,size_t offset,size_t len)
{
	//exbuffer_dump(value);
	//printf(">>receive bytes:%d\n",len);
	//当前缓冲区已经不能满足次数数据了
	if(len + exbuffer_getLen(value) > value->bufferlen){
		size_t rn1 = (len + exbuffer_getLen(value))/EXTEND_BYTES;
		if((len + exbuffer_getLen(value))%EXTEND_BYTES>0)rn1+=1;
		size_t ex = rn1 * EXTEND_BYTES;//每次扩展EXTEND_BYTES的倍数
		size_t exlen = ex - value->bufferlen;//增加的长度
		unsigned char* tmp = (unsigned char*)malloc(ex);
		//memset(tmp,0,ex);
		memcpy(tmp,value->buffer,value->bufferlen);
		//释放原内存区
		free(value->buffer);
		value->buffer = NULL;
		//重新指向新内存
		value->buffer = tmp;
		value->bufferlen = ex;
		
		//整理内存
		if (value->putOffset < value->readOffset) //***** ---********------- 
		{
			size_t cpylen;
			size_t cpystctstart;
			size_t cpydeststart;
			if (value->putOffset <= exlen) //***** ---********------- 
			{
				cpystctstart = 0;
				cpydeststart = ex - exlen;
				cpylen = value->putOffset;
			}
			else //********* ---********-----
			{
				cpystctstart = 0;
				cpydeststart = ex - exlen;
				cpylen = exlen;
			}
			memcpy(value->buffer + cpystctstart,value->buffer + cpydeststart,cpylen);
		}
	}
	
	if(exbuffer_getLen(value) == 0){
		value->putOffset = value->readOffset = 0;
	}
	//判断是否会冲破_buffer尾部
	if((value->putOffset + len) > value->bufferlen){
		//分两次存 一部分存在数据后面 一部分存在数据前面
		size_t len1 = value->bufferlen - value->putOffset;
		memcpy(value->buffer + value->putOffset,buffer + offset,len1);
		offset += len1;		
		size_t len2 = len - len1;
		memcpy(value->buffer,buffer + offset,len2);
		value->putOffset = len2;
	}else{
		memcpy(value->buffer + value->putOffset,buffer + offset,len);
		value->putOffset += len;
	}
	//exbuffer_dump(value);
	//printf("value->putOffset:%d\n",value->putOffset);
	//printf("value->readOffset:%d\n",value->readOffset);
	exbuffer_proc(value);
};

#ifdef __cplusplus
}
#endif

#endif