#include "Common.h"

int main(int argc, char* argv[])
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 0;

	struct sockaddr_in sockAddress;
	std::memset(&sockAddress, 0, sizeof(sockAddress));

	inet_pton(AF_INET, "127.0.0.1", &sockAddress.sin_addr);
	sockAddress.sin_port = htons(56671);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock != 0)
		err_quit("ERRPR : ");

	int retval = connect(sock, (struct sockaddr*)&sockAddress, sizeof(sockAddress));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	closesocket(sock);

	WSACleanup();
	return 0;
}
