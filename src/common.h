#ifndef _COMMON_H_
#define _COMMON_H_


// 标准库
#include <fcntl.h> //文件控制
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/types.h> /* See NOTES */
#include <sys/stat.h>
#include <unistd.h>

// 网络
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

// 三方库
#include <pthread.h>

//通用宏定义
#define ARG_SIZE    128
#define MSG_SIZE    256
#define PORT        51024

#define THREAD_MAX_NUM 100 /* max number of thread. */
#define THREAD_DEF_NUM 20  /* by default ,number of thread. */
#define THREAD_MIN_NUM 5   /* min number of thread pool. */


#endif