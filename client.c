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
		printf("Usage: %s port remote_file save_as\n", argv[0]);
		return 0;
	}

	struct ifreq ifr;
	struct sockaddr_in srvaddr;
	int cli_fd;
	int ret;

	info_t sendmsg;
	char recvmsg[MSG_SIZE] = "\0";

	//创建客户端socket
	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd < 0) {
		perror("client socket failed!");
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
		perror("get PA address failed!");
		close(cli_fd);
		return 0;
	}
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(atoi(argv[1]));
	/* 先将ifr.ifr_name强转为sockaddr_in,再->sin_addr.s_addr */
	srvaddr.sin_addr.s_addr = 
		((struct sockaddr_in*)&(ifr.ifr_name))->sin_addr.s_addr;
	//地址设置完成, 开始连接
	ret = connect(cli_fd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
	if (ret < 0) {
		perror("client connect failed!");
		close(cli_fd);
		return 0;
	}

	/* 测试一 获取文件状态 */
	sendmsg.flag = FILE_STAT;
	strcpy(sendmsg.path, argv[2]);
	ret = send(cli_fd, &sendmsg, sizeof(sendmsg), 0); //发送包
	if (ret < 0) {
		perror("client request file status failed to send!");
		close(cli_fd);
		return 0;
	}
	ret = recv(cli_fd, recvmsg, strlen(recvmsg)+1, 0);
	if (ret < 0) {
		perror("client response file status failed to recv!");
		close(cli_fd);
		return 0;
	}
	printf("file status: %s\n", recvmsg);

	/* 测试二 获取文件/下载文件 */
	//服务端逻辑仅仅服务一次, 重新服务必须重新连接
	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (cli_fd < 0) {
		perror("client socket failed!");
		return 0;
	}
	ret = connect(cli_fd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
	if (ret < 0) {
		perror("client connect failed!");
		close(cli_fd);
		return 0;
	}
	//发送文件下载请求
	sendmsg.flag = FILE;
	sendmsg.file_begin = 0;
	sprintf(sendmsg.path, "%s", argv[2]);
	ret = send(cli_fd, &sendmsg, sizeof(sendmsg), 0);
	if (ret < 0) {
		perror("send failed for get file download request package");
		close(cli_fd);
		return 0;
	}
	//本地创建文件, 开始下载
	int file_fd = open(FILE_NAME, O_WRONLY | O_CREAT, 0644);
	if (file_fd < 0) {
		perror("create/open local file failed!");
		close(cli_fd);
		return -1;
	}
	lseek(file_fd, sendmsg.file_begin, SEEK_SET);
	while (ret > 0) {
		ret = recv(cli_fd, recvmsg, MSG_SIZE, 0);
		write(file_fd, recvmsg, MSG_SIZE);
	}

	if (ret < 0) {
		perror("receive failed for file download package");
		close(cli_fd);
		close(file_fd);
		return -1;
	}

	perror("end");
	close(cli_fd);
	close(file_fd);
	return 0;

}