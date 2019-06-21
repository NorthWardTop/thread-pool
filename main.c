/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 18:49:20
 * @LastEditors: northward
 * @LastEditTime: 2019-06-21 22:59:25
 * @Description: 线程池main函数,用于
 * 启动线程管理线程, 监控线程, 任务管理线程
 */

#include "ds.h"

//全局队列定义
task_queue_t *task_queue_head;	 //任务队列
thread_queue_t *thread_queue_busy; //线程忙队列
thread_queue_t *thread_queue_idle; //线程空闲队列

/**
 * @description: 主函数初始化线程池系统
 * 创建并等待线程管理线程,任务管理线程,监控线程
 * @param {type} 
 * @return: 
 */
int main(int argc, char *argv[])
{
	pthread_t thread_manager_tid, task_manager_tid, monitor_tid;
	//全局指针的初始化
	init_pool();

	pthread_create(&thread_manager_tid, NULL, thread_manager, NULL);
	pthread_create(&task_manager_tid, NULL, task_manager, NULL);
	pthread_create(&monitor_tid, NULL, monitor, NULL);
	
	pthread_join(thread_manager_tid, NULL);
	pthread_join(task_manager_tid, NULL);
	pthread_join(monitor_tid, NULL);

	exit_pool();

	return 0;
}