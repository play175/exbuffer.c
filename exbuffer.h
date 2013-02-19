/*!
 * exbuffer.c
 * yoyo 2013 https://github.com/play175/exbuffer.c
 * new BSD Licensed
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef EXBUFFER_H_INCLUDED
#define EXBUFFER_H_INCLUDED


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
    enum exbuffer_endian endian;/*!<  */
    size_t readOffset;/*!<  */
    size_t putOffset;/*!<  */
    size_t dlen;/*!< 本次数据包长度 */
	unsigned char* buffer;/*!< 缓冲区 */
	size_t bufferlen;/*!< 缓冲区长度 */

    /**< 读包长临时用的数据 */
    unsigned char *headBytes;
	union HeadBytesS
    {
        unsigned char bytes[2];
        unsigned short val;
    } headS;

    union HeadBytesL
    {
        unsigned char bytes[4];
        unsigned long val;
    } headL;

	void (*recvHandle)(unsigned char*, size_t);/*!< 接收到数据时的回调函数指针 */
} exbuffer_t;


/*!< 创建一个新的exbuffer_t数据结构 */
exbuffer_t* exbuffer_new();

/*!< 释放exbuffer_t */
void exbuffer_free(exbuffer_t** value);

/*!< 打印内存数据 */
void exbuffer_printHex(unsigned char* bytes,unsigned short len);

/*!< 打印exbuffer_t中的缓冲区内存(最多打印50个字节) */
void exbuffer_dump(exbuffer_t* value,unsigned short len);

/*!< 获取缓冲区有效数据长度 */
size_t exbuffer_getLen(exbuffer_t* value);

/*!< 推送一段数据进去 */
void exbuffer_put(exbuffer_t* value, unsigned char* buffer,size_t offset,size_t len);

#endif // EXBUFFER_H_INCLUDED
