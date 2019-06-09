/*
 * @Author: northward
 * @Github: https://github.com/northwardtop
 * @Date: 2019-06-09 18:49:12
 * @LastEditors: northward
 * @LastEditTime: 2019-06-09 19:28:10
 * @Description: 用于描述线程池用到的各种结构的定义,函数的声明,和全局变量的声明
 * 
 */
#ifndef __DS_H__
#define __DS_H__

#include <pthread.h>

//通用宏定义
#define PORT 51024


//任务节点
typedef struct _task_node {
	void *arg; //任务执行函数参数
	void *(*func) (void *); //任务执行函数指针
	int task_id; //任务自身的ID
	pthread_t
}


//全局变量
