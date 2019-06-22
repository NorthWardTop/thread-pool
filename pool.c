/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 21:08:52
 * @LastEditors: northward
 * @LastEditTime: 2019-06-23 00:21:58
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
 * @description: 每个线程的工作函数
 * 完成各个线程工作的运行和释放
 * 完成两个线程队列间线程的迁移
 * @param {type} 
 * @return: 
 */
void *do_work(void *arg)
{
	//保存线程指针
	thread_node_t *self = (thread_node_t*)arg;
	pthread_mutex_lock(&self->mutex); //进入本线程上锁
	self->tid = syscall(SYS_gettid); //明确自己的tid
	pthread_mutex_unlock(&self->mutex); //设置好tid就解锁

	//死循环去执行或等待任务
	while (1) {
		pthread_mutex_lock(&self->mutex); //上锁自己,代码保护区
		if (self->task == NULL) //当前线程是空任务就等待
			pthread_cond_wait(&self->cond, &self->mutex);
		//所有线程,如果没有任务,都阻塞在上述代码,等待
		//否则就是有任务,继续执行,去锁定任务->执行任务
		pthread_mutex_lock(&self->task->mutex);
		self->task->func(self->task->arg);

		//执行完成后从小到大清理整个任务
		//1. 设置和释放任务和任务参数等属性
		self->task->func = NULL; //下一轮睡觉
		self->task->flag = 0;
		self->task->tid = 0;
		self->task->next = NULL;
		free(self->task->arg); //释放任务参数和任务锁
		pthread_mutex_unlock(&self->task->mutex);
		pthread_mutex_destroy(&self->task->mutex);
		free(self->task); //最后释放任务内存
		//2. 复原线程节点属性
		self->task = NULL;
		self->flag = 0;

		//从任务队列中取新任务
		pthread_mutex_lock(&task_queue_head);
		if (task_queue_head != NULL) {
			//如果队列有任务,保存任务节点并设置到当前线程上
			//取任务,头指针后移
			task_node_t *task = task_queue_head->head;
			task_queue_head->head = task_queue_head->head->next; 

			//设置线程对象,让他下一轮运行起来
			self->flag = 1;
			self->task = task;
			//设置任务对象
			task->tid = self->tid;
			task->next = NULL; //一个任务如果有多个任务节点
			task->flag = 1;

			task_queue_head->number--;

			//解锁任务队列,解锁线程,退出保护区
			pthread_mutex_unlock(&task_queue_head->mutex);
			pthread_mutex_unlock(&self->mutex);
			continue; //返回下轮直接执行
		} else {
			//如果任务队列空了
			...
		}

	}

	return NULL;
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

