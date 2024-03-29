
#include <pthread.h>

#include "common.h"
#include "pool.h"

#define TEST_THREAD_NUM	2


#define REMOTE_FILE		"test.txt"
#define SAVE_AS			"mk"
#define SERVER_ADDR		"127.0.0.1"


size_t get_file_size(int cli_fd, const char *remote_file)
{
	//创建客户端socket
	int ret;
	size_t fileSize = 0;
	info_t sendinfo;
	char recvmsg[MSG_SIZE];

	

	/* 测试一 获取文件状态 */
	sendinfo.flag = FILE_STAT;
	strcpy(sendinfo.path, remote_file);
	ret = send(cli_fd, &sendinfo, sizeof(sendinfo), 0); //发送包
	if (ret < 0) {
		perror("client request file status failed to send!");
		close(cli_fd);
		return 0;
	}

	ret = recv(cli_fd, recvmsg, MSG_SIZE, 0);
	fileSize = atoi(recvmsg);
	fileSize = ntohl(fileSize);
	if (ret < 0) {
		perror("client response file status failed to recv!");
		close(cli_fd);
		return 0;
	}
	printf("file status: %ld\n", fileSize);
	close(cli_fd);
	
	return fileSize;
}


int download_file(int cli_fd, const char *remote_file, const char *save_as, size_t fileSize)
{
	// size_t fileSize;
	int ret;
	info_t sendinfo;
	char recvmsg[MSG_SIZE];

	/* 测试二 获取文件/下载文件 */
	//服务端逻辑仅仅服务一次, 重新服务必须重新连接
	
	//发送文件下载请求
	sendinfo.flag = FILE;
	sendinfo.file_begin = 0;
	sprintf(sendinfo.path, "%s", remote_file);
	ret = send(cli_fd, &sendinfo, sizeof(sendinfo), 0);
	if (ret < 0) {
		perror("send failed for get file download request package");
		close(cli_fd);
		return 0;
	}
	//本地创建文件, 开始下载
	int file_fd = open(save_as, O_WRONLY | O_CREAT, 0644);
	if (file_fd < 0) {
		perror("create/open local file failed!");
		close(cli_fd);
		return -1;
	}
	lseek(file_fd, sendinfo.file_begin, SEEK_SET);
	int nleft = fileSize;
	do {
		memset(recvmsg, 0, strlen(recvmsg));
		ret = recv(cli_fd, recvmsg, MSG_SIZE, 0);
		write(file_fd, recvmsg, ret);
		nleft-=ret;
	} while (nleft > 0);

	if (ret < 0) {
		perror("receive failed for file download package");
		close(cli_fd);
		close(file_fd);
		return -1;
	}

	close(file_fd);
	close(cli_fd);
	return 0;
}


void *thread_func(void *arg)
{
	struct sockaddr_in srvaddr;
	int cli_fd, ret;

	bzero(&srvaddr, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(PORT);
	srvaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd < 0) {
		perror("client socket failed!");
		return 0;
	}

	ret = connect(cli_fd, (struct sockaddr*)&srvaddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("client connect failed!");
		close(cli_fd);
		return 0;
	}

	//获取文件大小
	size_t fileSize = get_file_size(cli_fd, REMOTE_FILE);
	close(cli_fd);
	cli_fd = 0;

	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd < 0) {
		perror("client socket failed!");
		return 0;
	}
	ret = connect(cli_fd, (struct sockaddr*)&srvaddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("client connect failed!");
		close(cli_fd);
		return 0;
	}


	download_file(cli_fd, REMOTE_FILE, SAVE_AS, fileSize);
	return NULL;

}

/**
 * @description: 网络客户端,用于测试线程池
 * @param {
 * 4个参数
 * argv[1]=端口
 * argv[2]=要访问的远程文件
 * argv[3]=下载的文件名} 
 * @return: 
 */
int main(int argc, const char *argv[])
{
	// if (argc != 4) {
	// 	printf("Usage: %s port remote_file save_as\n", argv[0]);
	// 	return 0;
	// }

	
	// int cli_fd, ret;
	pthread_t tids[TEST_THREAD_NUM];

	//设置地址信息
	//方法一:直接输入地址...
	
	
	for (size_t i = 0; i < TEST_THREAD_NUM; i++)
		pthread_create(&tids[i], NULL, thread_func, NULL);

	for (size_t i = 0; i < TEST_THREAD_NUM; i++)
		pthread_join(tids[i], NULL);
	
	

	perror("end");
	return 0;

}