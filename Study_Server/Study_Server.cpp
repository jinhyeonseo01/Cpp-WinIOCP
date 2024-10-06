#include "Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <list>
#include <memory>
#include <ranges>
#include <functional>
#include <numbers>
#include <numeric>
#include <string>
#include <string_view>

int main(int argc, char* argv[])
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 0;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	//inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_addr.s_addr = 0;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int a = bind(sock, (sockaddr*)&addr, sizeof(addr));
	printf("%d\n", a);
	int b = listen(sock, 10);
	printf("%d\n", b);
	//int c = accept(sock, (sockaddr*)&addr, sizeof(addr));

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	addrlen = sizeof(clientaddr);
	client_sock = accept(sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
	}
	printf("Connect!\n");
	char buf[1000];
	while (1)
	{
		int retval = recv(client_sock, buf, 1000, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		// 데이터 보내기
		retval = send(client_sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	scanf("%d");
	//accept()

	WSACleanup();
	return 0;
}

