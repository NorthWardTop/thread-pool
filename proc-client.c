#include "ds.h"


/**
 * @description: 每个线程要处理的任务, 属于应用逻辑
 * @param {客户端连接的文件描述符} 
 * @return: 状态
 */
void *proc_client(void* ptr)
{
	printf("thread success\n");
	int cli_fd = atoi((char*)ptr); //调用他前是sprintf转化的fd
	int ret;
	info_t msg;

	struct stat mystat;
	char sendmsg[MSG_SIZE] = '\0';
	memset(&msg, '\0', sizeof(msg));
	memset(&sendmsg, '\0', strlen(sendmsg));

	//从客户端接收fd
	ret = recv(cli_fd, &msg, sizeof(msg), 0);
	if (ret < 0) {
		perror("proc_client recv failed!\n");
		close(cli_fd);
		return NULL;
	}

	//分析msg命令
	switch (msg.flag)
	{
	case FILE_STAT: /* 发送文件状态(大小) */
		ret = stat(msg.path, &mystat); //获取文件状态
		if (ret < 0) {
			perror("client get file status failed!\n");
			close(cli_fd);
			return NULL;
		}
		//打包发送
		sprintf(sendmsg, "%d", htol(mystat.st_size));
		ret = send(cli_fd, sendmsg, strlen(sendmsg) + 1, 0);
		if (ret < 0) {
			perror("send file stat failed in proc_client");
			close(cli_fd);
			return NULL;
		}
		//发送完成,关闭fd
		close(cli_fd);
		break;
	case FILE: /* 发送文件 */
		break;
	default:
		break;
	}

	return NULL;
}
