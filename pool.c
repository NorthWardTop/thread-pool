/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 21:08:52
 * @LastEditors: northward
 * @LastEditTime: 2019-06-10 00:42:55
 * @Description: 线程池函数实现
 */

#include <pthread.h>
#include "ds.h"

//全局变量声明
extern task_queue_t *task_queue_head;	 //任务队列
extern thread_queue_t *thread_queue_busy; //线程忙队列
extern thread_queue_t *thread_queue_idle; //线程空闲队列

void *thread_manager(void *ptr)
{
	return NULL;
}

void *task_manager(void *ptr)
{
	return NULL;
}

void *monitor(void *ptr)
{
	return NULL;
}

