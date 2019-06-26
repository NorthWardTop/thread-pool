/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 18:49:12
 * @LastEditors: northward
 * @LastEditTime: 2019-06-27 00:47:20
 * @Description: 用于描述线程池用到的各种结构的定义,函数的声明,和全局变量的声明
 * ds是data segment缩写
 */
#ifndef __DS_H__
#define __DS_H__

// 标准库
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/syscall.h> 
#include <sys/ioctl.h>
#include <fcntl.h> //文件控制

// 网络
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

// 三方库
#include <pthread.h>

//通用宏定义
#define PORT 51024

#define THREAD_MAX_NUM  100     /* max number of thread. */ 
#define THREAD_DEF_NUM  20      /* by default ,number of thread. */
#define THREAD_MIN_NUM  5       /* min number of thread pool. */


//任务节点
typedef struct _task_node {
	void *arg;             //任务执行函数参数
	void *(*func)(void *); //任务执行函数指针
	int task_id;           //任务自身的ID
	pthread_t tid;         //执行任务的tid
	char flag;             //任务是否被派发

	struct _task_node *next;      //链表的下一个结点
	pthread_mutex_t mutex; //用于互斥访问本节点
} task_node_t;

//任务队列
typedef struct _task_queue {
	task_node_t *head; //队列头节点
	int number;      //任务数,包含未分配的和已经分但未完成

	pthread_mutex_t mutex; //线程互斥访问本队列
	pthread_cond_t cond; //没有任务将阻塞管理线程,来了新任务将唤醒
} task_queue_t;

//线程节点
typedef struct _thread_node {
	pthread_t tid; //线程tid
	char flag; //是否忙
	task_node_t *task; //分配的任务

	struct _thread_node *prev; //前面的线程
	struct _thread_node *next; //后面的线程
	pthread_mutex_t mutex; //线程互斥访问当前结点
	//当管理线程没有投递任务时,休眠,管理线程分配任务时候signal
	pthread_cond_t cond; 
} thread_node_t;

//线程队列
typedef struct _thread_queue {
	int number; //队列中的线程数

	struct _thread_node *head;
	struct _thread_node *rear; 
	pthread_mutex_t mutex; //线程互斥访问这个队列
	//没有空闲线程则阻塞,有空闲线程或有完成任务的线程则唤醒
	pthread_cond_t cond; 
} thread_queue_t;

//客户端服务端发送消息的结构
struct info {
	char flag; //0:文件状态stat, 1:文件内容
	char path; //请求的文件名/路径
	int file_begin; //获取状态时不使用
	int len; //获取状态时不使用
};


//函数声明
void init_pool();
void pool_create();
void pool_destroy(thread_queue_t *tp);
void exit_pool();

void *thread_manager(void *ptr);
void *task_manager(void *ptr);
void *monitor(void *ptr);

void *do_work(void *arg);


#endif

