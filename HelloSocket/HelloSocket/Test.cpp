#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")

int main() {
	//启动Windows的Sock网络环境
	WORD ver = MAKEWORD(2,2);
	WSADATA dat;
	WSAStartup(ver, &dat);



	WSACleanup();
	return 0;
}
