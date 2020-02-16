#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

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
struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader {
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


int main() {
	//����Windows��Sock���绷��
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1�����������˵�socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("���󣬽���Socketʧ��...\n");
	}else {
		printf("����Socket�ɹ�...\n");
	}

	//����socket��ַ
	sockaddr_in _sin = {};
	memset(&_sin, 0, sizeof(_sin));
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //inet_addr("127.0.0.1");s
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(_sin));
	//2���ӷ�����
	if (SOCKET_ERROR == ret) {
		printf("�������ӷ�����ʧ��...\n");
	}
	else {
		printf("���ӷ������ɹ�...\n");
	}

	while (TRUE) {
		//��������
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
		//��������
		if (0 == strcmp(cmdBuf,"exit")) {
			printf("�յ�exit����������...\n");
			break;
		}else if(0 == strcmp(cmdBuf, "login")){
			Login login;
			strcpy(login.userName,"xl");
			strcpy(login.passWord, "xlmm");
			//���������������
			send(_sock, (const char*)&login, sizeof(login), 0);
			//���շ��������ص�����
			LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult��%d \n", loginRet.result);
		}else if (0 == strcmp(cmdBuf, "logout")) {
			Loginout logout;
			strcpy(logout.userName, "xl");
			//���������������
			send(_sock, (char*)&logout, sizeof(Loginout), 0);
			//���շ��������ص�����
			LoginoutResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(LoginoutResult), 0);
			printf("LoginoutResult��%d \n", logoutRet.result);
		}else {
			printf("��֧�ֵ��������������...\n");
		}
	}

	//4�ر�socket
	closesocket(_sock);
	WSACleanup();
	printf("���˳����������\n");
	getchar();
	return 0;
}