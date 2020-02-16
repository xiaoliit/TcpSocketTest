#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <vector>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_ERROR
};

//DateHeader
struct DataHeader {
	short dataLength;
	short cmd;
};

//DataPackage
struct Login: public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult: public DataHeader {
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Loginout : public DataHeader {
	Loginout() {
		dataLength = sizeof(Loginout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LoginoutResult : public DataHeader {
	LoginoutResult() {
		dataLength = sizeof(LoginoutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};

vector<SOCKET> g_clients;

int processor(SOCKET _cSock) {
	//接收客户端的请求数据
	char szRecv[4096] = {};
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("客户端已退出,任务结束\n");
		return -1;
	}
	//处理请求
	switch (header->cmd) {
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("收到命令：CMD_LOGIN 数据长度：%d,userName=%s,userPassword=%s\n", login->dataLength, login->userName, login->passWord);
		//忽略判断用户密码是否正确的过程
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(ret), 0);
	}
	break;
	case CMD_LOGINOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout* logout = (Loginout*)szRecv;
		printf("收到命令：CMD_LOGINOUT 数据长度：%d,userName=%s\n", logout->dataLength, logout->userName);
		LoginoutResult ret;
		send(_cSock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	default:
		DataHeader header = { 0,CMD_ERROR };
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		break;
	}
}

int main() {
	
	//启动Windows的Sock网络环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//1建立服务器端的socket
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);

	//创建socket地址
	sockaddr_in _sin = {};
	memset(&_sin,0,sizeof(_sin));
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");

	//2绑定用于接受客户端连接的网络端口
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("错误,绑定网络端口失败...\n");
		return -1;
	}else {
		printf("绑定网络端口成功...\n");
	}

	//3监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("监听网络端口失败...\n");
	}
	else {
		printf("监听网络端口成功...\n");
	}

	while (TRUE) {
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			FD_SET(g_clients[n], &fdRead);
		}

		timeval t = {0,0};
		//nfds 是一个整数值，是指fd_set集合中所有的socket的范围
		int ret = select(_sock + 1,&fdRead,&fdWrite,&fdExp,&t);
		if (ret < 0) {
			printf("select任务结束\n");
			break;
		}
		if (FD_ISSET(_sock,&fdRead)) {
			FD_CLR(_sock,&fdRead);
			//4接收客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(clientAddr);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("错误，接受到无效的客户端SOCKET...\n");
				return -1;
			}
			g_clients.push_back(_cSock);
			printf("新客户端加入: socket = %d,IP = %s \n", _cSock, inet_ntoa(clientAddr.sin_addr));
			
		}

		for (size_t n = 0; n < fdRead.fd_count -1; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}
	}

	for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
		closesocket(g_clients[n]);
	}
	//6关闭socket
	closesocket(_sock);
	WSACleanup();
	printf("已退出，任务结束\n");
	getchar();
	return 0;
}