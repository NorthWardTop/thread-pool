/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 21:08:52
 * @LastEditors: northward
 * @LastEditTime: 2019-06-12 00:41:27
 * @Description: 线程池函数实现
 */

#include <pthread.h>
#include "ds.h"

//全局变量声明
extern task_queue_t *task_queue_head;	 //任务队列
extern thread_queue_t *thread_queue_busy; //线程忙队列
extern thread_queue_t *thread_queue_idle; //线程空闲队列

/**
 * @description: 初始化线程池的全局变量
 * @param {type} 
 * @return: 
 */
void init_pool()
{
	//初始化任务队列
	task_queue_head = (task_queue_t*)malloc(sizeof(task_queue_t));	 
	task_queue_head->head = NULL;
	task_queue_head->number = 0;
	pthread_mutex_init(&task_queue_head->mutex, NULL);
	pthread_cond_init(&task_queue_head->cond, NULL);

	//线程忙队列
	thread_queue_busy = (thread_queue_t*)malloc(sizeof(thread_queue_t)); 
	thread_queue_busy->head = NULL;
	thread_queue_busy->rear = NULL;
	thread_queue_busy->number = 0
	pthread_mutex_init(&thread_queue_busy->mutex, NULL);
	pthread_cond_init(&thread_queue_busy->cond, NULL);

	//线程空闲队列
	thread_queue_idle = (thread_queue_t*)malloc(sizeof(thread_queue_t)); 
	thread_queue_idle->head = NULL;
	thread_queue_idle->rear = NULL;
	thread_queue_idle->number = 0
	pthread_mutex_init(&thread_queue_idle->mutex, NULL);
	pthread_cond_init(&thread_queue_idle->cond, NULL);
	//主线程到此任务完成
	//创建空闲线程池
	create_pool();
}

/**
 * @description: 分配和设置线程池
 * @param {type} 
 * @return: 
 */
void create_pool()
{
	int i=0;
	//分配一个动态一维数组的内存, 数组个数为默认池子大小
	thread_node_t *tmp = (thread_node_t*)
		malloc(sizeof(thread_node_t)*THREAD_DEF_NUM);
	if (tmp == NULL) {
		perror("create_pool: malloc failed!");
		exit_pool();
	}

	for (; i<THREAD_DEF_NUM; ++i) {
		tmp[i].tid = 0;//没创建线程先初始化为0
		tmp[i].flag = 0;//空闲
		tmp[i].task = NULL; //没有任务
		//分别设置这个节点的前后
		if (i == 0) 
			tmp[i].prev = NULL;
		else
			tmp[i].prev = &tmp[i - 1];

		if (i == THREAD_DEF_NUM - 1)
			tmp[i].next = NULL;
		else
			tmp[i].next = &tmp[i+1];
		//初始化这个线程节点的条件锁和互斥锁
		pthread_mutex_init(&tmp[i].mutex, NULL);
		pthread_cond_init(&tmp[i].cond, NULL);
		//创建这个线程, 使他执行do_work(),
		//没有任务的将全部阻塞在这个函数上
		pthread_create(&tmp[i].tid, NULL, do_work, NULL);
	}
}

/**
 * @description: 退出前清理内存, 函数从后往前写
 * @param {type} 
 * @return: 
 */
void exit_pool()
{
	//清理工作,工作参数和任务
	...

	//0.清理三个全局队列指针
	pthread_mutex_lock(&thread_queue_idle->mutex);
	if (thread_queue_idle)
		free(thread_queue_idle);
	pthread_mutex_unlock(&thread_queue_idle->mutex);
	pthread_mutex_destroy(&thread_queue_idle->mutex); //销毁锁

	...


}

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

