/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 21:08:52
 * @LastEditors: northward
 * @LastEditTime: 2019-06-21 23:46:56
 * @Description: 线程池函数实现
 */


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
	thread_queue_busy->number = 0;
	pthread_mutex_init(&thread_queue_busy->mutex, NULL);
	pthread_cond_init(&thread_queue_busy->cond, NULL);

	//线程空闲队列
	thread_queue_idle = (thread_queue_t*)malloc(sizeof(thread_queue_t)); 
	thread_queue_idle->head = NULL;
	thread_queue_idle->rear = NULL;
	thread_queue_idle->number = 0;
	pthread_mutex_init(&thread_queue_idle->mutex, NULL);
	pthread_cond_init(&thread_queue_idle->cond, NULL);
	//主线程到此任务完成
	//创建空闲线程池
	pool_create();
}

/**
 * @description: 分配和设置空闲线程池队列线程池
 * @param {type} 
 * @return: 
 */
void pool_create()
{
	int i=0;
	//分配一个动态一维数组的内存, 数组个数为默认池子大小
	struct _thread_node *tmp = (struct _thread_node*)
		malloc(sizeof(struct _thread_node)*THREAD_DEF_NUM);
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
	//将空闲线程指针head指向数组第一个
	//也可以不加锁
	pthread_mutex_lock(&thread_queue_idle->mutex);
	thread_queue_idle->head = &tmp[0]; //头结点thread_queue_t*为
	thread_queue_idle->rear = &tmp[THREAD_DEF_NUM-1];
	thread_queue_idle->number = THREAD_DEF_NUM;
	//线程设置完成解锁
	pthread_mutex_unlock(&thread_queue_idle->mutex);
}



/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void *do_work(void *arg)
{
	
}


/**
 * @description: 销毁池
 * 
 * @param {
 * thread_queue_t *tp 将要销毁的线程池地址
 * } 
 * @return: 
 */
void pool_destroy(thread_queue_t *tp)
{

}

/**
 * @description: 退出前清理内存, 函数从后往前写
 * 主要完成三个全局指针的清理及其子节点如函数参数等的清理
 * @param {type} 
 * @return: 
 */
void exit_pool()
{
	//清理工作,工作参数和任务
//1.清理三个队列每个节点
	//清理任务队列节点(相当于清理head指向的动态一维数组)
	// pthread_mutex_lock(&task_queue_head->mutex);
	// while(task_queue_head->head)
	// ...
	// pthread_mutex_unlock(&task_queue_head->mutex);
	// pthread_mutex_destroy(&task_queue_head->mutex);
	// if (task_queue_head)
	// 	free(task_queue_head);

	// //清理忙线程队列节点
	// pthread_mutex_lock(&thread_queue_busy->mutex);
	// ...
	// pthread_mutex_unlock(&thread_queue_busy->mutex);
	// pthread_mutex_destroy(&thread_queue_busy->mutex);
	// if (thread_queue_busy) 
	// 	free(thread_queue_busy);

	// //清理空闲线程队列节点
	// pthread_mutex_lock(&thread_queue_idle->mutex);
	// if (thread_queue_idle->head)
	// 	free(thread_queue_idle->head); //释放动态一维数组
	// pthread_mutex_unlock(&thread_queue_idle->mutex);
	// pthread_mutex_destroy(&thread_queue_idle->mutex);
	// if (thread_queue_idle)
	// 	free(thread_queue_idle);
	printf("clean pool\n");

}

void *thread_manager(void *ptr)
{
	while (1) {
		
	} 
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

