#include "Common.h"

int main(int argc, char* argv[])
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 0;

	struct sockaddr_in sockAddress;
	std::memset(&sockAddress, 0, sizeof(sockAddress));

	struct hostent* domainIP = gethostbyname("clrain.ggm.kr");
	printf("%d\n", domainIP->h_addrtype);
	in_addr addr;
	memcpy(&addr, domainIP->h_addr, domainIP->h_length);
	std::array<char, 22> arr;
	inet_ntop(AF_INET, &addr, arr.data(), arr.size());
	auto intAddr = htonl(*(unsigned int*)(domainIP->h_addr));
	for(int i=0;i<4;i++)
		printf("%d\n", ((unsigned char*)&intAddr)[i]);
	printf("%s\n", arr.data());

	//inet_pton(AF_INET, "127.0.0.1", &sockAddress.sin_addr);
	sockAddress.sin_addr = addr;
	sockAddress.sin_port = htons(56672);
	
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR)
		err_quit("ERRPR : ");
	/*
	int retval = connect(sock, (struct sockaddr*)&sockAddress, sizeof(sockAddress));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	printf("connect");
	*/

	closesocket(sock);

	WSACleanup();
	return 0;
}

