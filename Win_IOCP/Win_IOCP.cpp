#include "Common.h"

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		err_quit("Error");
	std::cout << "열림\n";
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		err_quit("Error");
	std::cout << "닫음\n";
	closesocket(sock);
	WSACleanup();
	return 0;
}