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
	//���տͻ��˵���������
	char szRecv[4096] = {};
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)szRecv;
	if (nLen <= 0) {
		printf("�ͻ������˳�,�������\n");
		return -1;
	}
	//��������
	switch (header->cmd) {
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ����CMD_LOGIN ���ݳ��ȣ�%d,userName=%s,userPassword=%s\n", login->dataLength, login->userName, login->passWord);
		//�����ж��û������Ƿ���ȷ�Ĺ���
		LoginResult ret;
		send(_cSock, (char*)&ret, sizeof(ret), 0);
	}
	break;
	case CMD_LOGINOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout* logout = (Loginout*)szRecv;
		printf("�յ����CMD_LOGINOUT ���ݳ��ȣ�%d,userName=%s\n", logout->dataLength, logout->userName);
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
	
	//����Windows��Sock���绷��
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//1�����������˵�socket
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);

	//����socket��ַ
	sockaddr_in _sin = {};
	memset(&_sin,0,sizeof(_sin));
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");

	//2�����ڽ��ܿͻ������ӵ�����˿�
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("����,������˿�ʧ��...\n");
		return -1;
	}else {
		printf("������˿ڳɹ�...\n");
	}

	//3��������˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("��������˿�ʧ��...\n");
	}
	else {
		printf("��������˿ڳɹ�...\n");
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
		//nfds ��һ������ֵ����ָfd_set���������е�socket�ķ�Χ
		int ret = select(_sock + 1,&fdRead,&fdWrite,&fdExp,&t);
		if (ret < 0) {
			printf("select�������\n");
			break;
		}
		if (FD_ISSET(_sock,&fdRead)) {
			FD_CLR(_sock,&fdRead);
			//4���տͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(clientAddr);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("���󣬽��ܵ���Ч�Ŀͻ���SOCKET...\n");
				return -1;
			}
			g_clients.push_back(_cSock);
			printf("�¿ͻ��˼���: socket = %d,IP = %s \n", _cSock, inet_ntoa(clientAddr.sin_addr));
			
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
	//6�ر�socket
	closesocket(_sock);
	WSACleanup();
	printf("���˳����������\n");
	getchar();
	return 0;
}