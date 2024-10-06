#include "Common.h"

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <deque>
#include <fstream>
#include <list>
#include <memory>
#include <ranges>
#include <functional>
#include <numbers>
#include <numeric>
#include <string>
#include <string_view>


void clearConsole() {
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}


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
	addr.sin_port = htons(9000);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int a = bind(sock, (sockaddr*)&addr, sizeof(addr));
	int b = listen(sock, 10);

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	addrlen = sizeof(clientaddr);
	client_sock = accept(sock, (struct sockaddr*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET) {
		err_display("accept()");
	}
	printf("Connect!\n");

	unsigned long long int fileSize;
	int retval = recv(client_sock, (char*)(&fileSize), 8, MSG_WAITALL);
	fileSize = ntohll(fileSize);

	unsigned short fileNameSize;
	retval = recv(client_sock, (char*)(&fileNameSize), 2, MSG_WAITALL);
	fileNameSize = ntohs(fileNameSize);

	printf("fileSize : %lld\n", static_cast<long long int>(fileSize));
	std::vector<char> nameData;
	nameData.resize((fileNameSize+1));
	retval = recv(client_sock, (char*)nameData.data(), static_cast<short>(fileNameSize), MSG_WAITALL);
	nameData[fileNameSize] = '\0';
	std::string fileName = nameData.data();
	std::cout << "fileName : " << fileName<<"\n";
	clearConsole();
	COORD coord;
	coord.X = 0;
	coord.Y = 0;

	std::deque<char> fileData;
	std::array<char, 1024> buf;
	int offset = 0;
	while (1) {
		int readSize = min(buf.size(), fileSize - offset);
		if (readSize <= 0)
			break;
		int retval = recv(client_sock, buf.data(), readSize, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		for (int i = 0; i < retval; i++)
			fileData.push_back(buf[i]);
		offset += retval;

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		printf("수신률 : %lf%%                \n", (offset / (float)fileSize)*100);
	}
	printf("수신완료\n");
	printf("저장중\n");
	
	std::string originalName = fileName.substr(0,std::find(fileName.begin(), fileName.end(), '.') - fileName.begin());
	std::string ext = fileName.substr(std::find(fileName.begin(), fileName.end(), '.') - fileName.begin() + 1);
	std::ofstream os(originalName+"(download)." + ext, std::ios::binary);

	for (int i = 0; i < fileData.size(); i++)
		os.write((char*)&fileData[i], 1);
	os.close();
	printf("저장완료\n");

	scanf("%d");
	//accept()

	WSACleanup();
	return 0;
}

