#include "ds.h"

#define FILE_NAME		"test.txt"

/**
 * @description: 网络客户端,用于测试线程池
 * @param {
 * 4个参数
 * argv[1]=端口
 * argv[2]=要访问的远程文件
 * argv[3]=下载的文件名} 
 * @return: 
 */
int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("Usage: %s port remote_file save_as\n");
		return 0;
	}

	struct ifreq ifr;
	struct sockaddr_in srvaddr;
	int cli_fd;
	int ret;

	//创建客户端socket
	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd) {
		perror("client socket failed!\n");
		return 0;
	}
	//设置地址信息
	//方法一:直接输入地址...
	// 
	// srvaddr.sin_family = AF_INET;
	// srvaddr.sin_port = htons(atoi(argv[1]));
	// srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//方法二:从interface获取
	strcpy(ifr.ifr_name, "lo");
	ret = ioctl(cli_fd, SIOCGIFADDR, &ifr);
	if (ret < 0) {
		perror("get PA address failed!\n");
		close(cli_fd);
		return 0;
	}
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(atoi(argv[1]));
	/* 先将ifr.ifr_name强转为sockaddr_in,再->sin_addr.s_addr */
	srvaddr.sin_addr.s_addr = 
		((struct sockaddr_in*)&(ifr.ifr_name))->sin_addr.s_addr;
	ret = connect(cli_fd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
	if (ret < 0) {
		perror("client connect failed!\n");
		close(cli_fd);
		return 0;
	}
	/* 测试一 获取文件状态 */
	info_t sendmsg;
	char recvmsg[MSG_SIZE] = "\0";
	sendmsg.flag = FILE_STAT;
	strcpy(sendmsg.path, argv[2]);
	ret = send(cli_fd, sendmsg, sizeof(sendmsg), 0); //发送包
	if (ret < 0) {
		perror("client request file status failed to send!\n");
		close(cli_fd);
		return 0;
	}
	ret = recv(cli_fd, sendmsg, strlen(sendmsg)+1, 0);
	if (ret < 0) {
		perror("client response file status failed to recv!\n");
		close(cil_fd);
		return 0;
	}
	printf("file status: %s\n", recvmsg);

	/* 测试二 获取文件/下载文件 */
	//服务端逻辑仅仅服务一次, 重新服务必须重新连接

	ret = connect(cli_fd, (struct sockaddr*)srvaddr, sizeof(srvaddr));
	if (ret < 0) {
		perror("client connect failed!\n");
		close(cli_fd);
		return 0;
	}


	sendmsg.file_begin




}