/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 21:08:52
 * @LastEditors: northward
 * @LastEditTime: 2019-07-20 15:27:44
 * @Description: 线程池函数实现
 */


#include "common.h"
#include "pool.h"

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
	thread_node_t *tmp = (thread_node_t*)
		malloc(sizeof(thread_node_t)*THREAD_DEF_NUM);
	if (tmp == NULL) {
		perror("create_pool: malloc failed!");
		exit_pool();
	}

	for (i = 0; i<THREAD_DEF_NUM; ++i) {
		tmp[i].tid = i+1;//没创建线程先初始化为0
		tmp[i].flag = 0;//空闲
		tmp[i].task = NULL; //没有任务

		//分别设置这个节点的前后 BUG?
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
		pthread_create(&tmp[i].tid, NULL, do_work, (void*)&tmp[i]);
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

		//从任务队列中取新任务 BUG?
		pthread_mutex_lock(&task_queue_head->mutex);
		if (task_queue_head->head != NULL) {
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
			pthread_mutex_unlock(&task_queue_head->mutex); //没有任务执行, 解锁任务队列
			pthread_mutex_lock(&thread_queue_busy->mutex); //上锁忙队列准备摘除已经运行的
			//如果任务队列空,没有任务去执行,将当前结点从忙队列移动到空闲队列
			//1. 从忙队列摘除,有四种情况:只有这一个节点,头上,尾巴,中间
			if (thread_queue_busy->head == self && thread_queue_busy->rear == self) {
				//只有这一个节点
				thread_queue_busy->head = NULL;
				thread_queue_busy->rear = NULL;
				self->prev = NULL; self->next = NULL;
			} else if (thread_queue_busy->head == self && thread_queue_busy->rear != self) {
				//头上
				thread_queue_busy->head = thread_queue_busy->head->next;
				thread_queue_busy->head->prev = NULL;
				self->prev = NULL; self->next = NULL;
			} else if (thread_queue_busy->head != self && thread_queue_busy->rear == self) {
				//尾巴
				thread_queue_busy->rear = thread_queue_busy->rear->prev;
				thread_queue_busy->rear->next = NULL;
				self->prev = NULL; self->next = NULL;
			} else {
				//中间
				//SF
				self->prev->next = self->next;
				self->next->prev = self->prev;
				self->prev = NULL; self->next = NULL;
			}
			//从忙队列摘除完成,解锁忙队列
			pthread_mutex_unlock(&thread_queue_busy->mutex);

			//2. 将self线程加到空闲队列,两种情况:队列空,不空就放在头上
			pthread_mutex_lock(&thread_queue_idle->mutex);
			//BUG?
			if (thread_queue_idle->head == NULL && thread_queue_idle->rear == NULL) {
				thread_queue_idle->head = self;
				thread_queue_idle->rear = self;
				self->prev = NULL; self->next = NULL;
				//thread_queue_idle->number++;//BUG?
			} else {
				thread_queue_idle->head->prev = self;
				self->prev = NULL;
				self->next = thread_queue_idle->head;
				thread_queue_idle->head = self;
				thread_queue_idle->number++;
			}
			pthread_mutex_unlock(&thread_queue_idle->mutex);
			pthread_mutex_unlock(&self->mutex);
			//有新空闲节点加入,激活条件变量
			pthread_cond_signal(&thread_queue_idle->cond);
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

/**
 * @description: 从任务队列取任务,从空闲队列取线程,
 * 修改任务,修改线程,加入忙队列
 * @param {type} 
 * @return: 
 */
void *thread_manager(void *ptr)
{
	while (1) {
		task_node_t *tmp_task;
		thread_node_t *tmp_thread;

		//取任务
		pthread_mutex_lock(&task_queue_head->mutex);
		if (task_queue_head->number == 0) 
			pthread_cond_wait(&task_queue_head->cond, &task_queue_head->mutex);
		tmp_task = task_queue_head->head;
		task_queue_head->head = task_queue_head->head->next;
		tmp_task->next = NULL; //切断和队列关联
		task_queue_head->number--;
		pthread_mutex_unlock(&task_queue_head->mutex);

		//取线程
		pthread_mutex_lock(&thread_queue_idle->mutex);
		if (thread_queue_idle->number == 0) 
			pthread_cond_wait(&thread_queue_idle->cond, &thread_queue_idle->mutex);
		tmp_thread = thread_queue_idle->head;
		if (thread_queue_idle->head == thread_queue_idle->rear) {
			thread_queue_idle->head = NULL;
			thread_queue_idle->rear = NULL;
		} else {
			thread_queue_idle->head = thread_queue_idle->head->next;
			thread_queue_idle->head->prev = NULL;
		}
		thread_queue_idle->number--; 
		pthread_mutex_unlock(&thread_queue_idle->mutex);

		//设置任务
		pthread_mutex_lock(&tmp_task->mutex);
		// tmp_task->arg = NULL; 任务参数和函数在创建时候已经赋值
		tmp_task->flag = 1; 
		tmp_task->next = NULL;
		tmp_task->tid = tmp_thread->tid;
		pthread_mutex_unlock(&tmp_task->mutex);
		
		//设置线程
		pthread_mutex_lock(&tmp_thread->mutex);
		tmp_thread->flag = 1;
		tmp_thread->next = NULL;
		tmp_thread->prev = NULL;
		tmp_thread->task = tmp_task;
		pthread_mutex_unlock(&tmp_thread->mutex);

		//添加到忙队列
		pthread_mutex_lock(&thread_queue_busy->mutex);
		if (thread_queue_busy->head == thread_queue_busy->rear) {
			thread_queue_busy->head = tmp_thread;
			thread_queue_busy->rear = tmp_thread;
		} else {
			thread_queue_busy->head->prev = tmp_thread;
			tmp_thread->next = thread_queue_busy->head;
			thread_queue_busy->head = tmp_thread;
		}
		thread_queue_busy->number++;
		pthread_mutex_unlock(&thread_queue_busy->mutex);

		pthread_cond_signal(&tmp_thread->cond);
		//没有人等待忙队列
		// pthread_cond_signal(&thread_queue_busy->cond);

	}
	return NULL;
}

/**
 * @description: socket服务端,连接客户端请求并将连接句柄传递给任务节点参数
 * 具体实现:
 * 		1. 网络编程五步走(ip地址信息直接从网卡获取)
 * 			socket->sockaddr_in->bind->listen->accept
 * 		2. 循环accept阻塞等待新连接句柄
 * 		3. 有新句柄则创建任务节点, 并将句柄作为任务参数传递
 * 		4. 放入任务队列
 * @param {type} 
 * @return: 
 */
void *task_manager(void *ptr)
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	int ret = -1;
	if (listen_fd < 0) {
		perror("socket failed!\n");
		goto socket_err;
	}
	//interface request获取地址
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "lo");
	ret = ioctl(listen_fd, SIOCGIFADDR, &ifr); //获取PA addr
	if (ret < 0) {
		perror("ioctl get PA addr failed!\n");
		goto getaddr_err;
	}

	//设置sockaddr_in对象
	struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(PORT);
	srv_addr.sin_addr.s_addr =  /* 直接从ifr.ifr_addr接口中取地址(ifconfig) */
		((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr.s_addr;


	//bind
	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if (ret < 0) {
		perror("server socket bind failed!\n");
		goto bind_err;
	}
	//listen
	ret = listen(listen_fd, 5);
	if(ret < 0) {
		perror("server listen failed!\n");
		goto listen_err;
	}

	//server initialize complete, start accept new task
	for (int i = 0; 1; ++i) {
		int cli_fd;
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		//如果没有新连接请求, 该线程将阻塞在这
		cli_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &len);
		if (cli_fd < 0) {
			perror("server accept a client failed!\n");
			goto accept_err;
		}
		printf("accepted\n");

		//新连接接受完成, 开始new task
		task_node_t* tmp;
		task_node_t* newtask = (task_node_t*)malloc(sizeof(task_node_t));
		newtask->arg = malloc(ARG_SIZE);
		if (newtask == NULL || newtask->arg == NULL) {
			perror("new task malloc failed!\n");
			goto malloc_err;
		}

		//填充新的task任务节点
		memset(newtask->arg, '\0', ARG_SIZE);
		sprintf(newtask->arg, "%d", cli_fd); //将参数设置进新任务
		newtask->task_id = i; //task_id是循环变量
		newtask->flag = 0;
		newtask->func = proc_client;
		newtask->next = NULL;
		newtask->tid = 0;
		//初始化这个节点的互斥锁
		pthread_mutex_init(&newtask->mutex, NULL);
		pthread_mutex_lock(&task_queue_head->mutex); //锁定队列,开始加入
		if (task_queue_head->head == NULL) {
			task_queue_head->head = newtask;
		} else {
			tmp = task_queue_head->head;
			while (tmp) 
				tmp = tmp->next;//SF
			tmp->next = newtask;
		}
		task_queue_head->number++;
		//任务添加完成,解锁并激活队列
		pthread_mutex_unlock(&task_queue_head->mutex);
		pthread_cond_signal(&task_queue_head->cond);
	}

malloc_err:
accept_err:
listen_err:
bind_err:
getaddr_err:
	close(listen_fd);
socket_err:
	pool_destroy(thread_queue_idle);
	pool_destroy(thread_queue_busy);
	return NULL;
}

/**
 * @description: 每10秒遍历并打印忙队列所有的线程id和任务id 
 * 待扩展: 当空闲线程数在50秒内都处于空, 创建新线程
 * 		  当空闲线程数在50秒内都处于4/5以上, 销毁2/3的线程
 * @param {NULL} 
 * @return: NULL
 */
void *monitor(void *ptr)
{
	thread_node_t *thread = NULL;
	//线程主循环不终止
	while (1)
	{
		//上锁
		pthread_mutex_lock(&thread_queue_busy->mutex);
		printf("----------------begin------------------\n");
		thread = thread_queue_busy->head;
		//遍历队列
		while (thread)
		{
			printf("thread id: %ld, task id: %d\n", thread->tid, thread->task->task_id);
			thread = thread->next;
		}
		printf("----------------end------------------\n");
		//解锁队列
		pthread_mutex_unlock(&thread_queue_busy->mutex);
		//等待几秒再遍历
		sleep(5);

		int new_number;
		if (thread_queue_idle->number < THREAD_DEF_NUM*(1/5)) {
			//创建线程达到THREAD_DEF_NUM
			new_number = THREAD_DEF_NUM;
		} 
		if (thread_queue_busy->number < THREAD_DEF_NUM*(1/5)) {
			new_number = THREAD_DEF_NUM*(1/2);
		}
	}

	return NULL;
}

