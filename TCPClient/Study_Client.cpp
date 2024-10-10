#include "../Win_IOCP/Common.h"


#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <deque>
#include <fstream>
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <iostream>
#include <list>
#include <vector>

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

char *SERVERIP = (char *)"127.0.0.1";
std::string fileName = "";
#define SERVERPORT 9000
#define BUFSIZE    512



void clearConsole() {
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}


int Test(void* arg)
{

	return 0;
}


int main(int argc, char *argv[])
{
	int retval;

	CreateThread(0,0, Test,0,0, 0);



	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 2)
	{
		SERVERIP = argv[1];
		fileName = std::string(argv[2], argv[2]+strlen(argv[2]));
	}
	printf("%d\n", fileName.size());
	printf("파일 읽는 중 %s\n", fileName.c_str());
	std::ifstream is{ fileName, std::ios::binary};
	std::deque<char> fileData;
	if (!is)
	{
		printf("파일 읽기 실패\n");
		return 0;
	}

	int headerSize = 8;

	is.seekg(0, std::ios::end);
	long long fileSize = is.tellg();
	is.seekg(0, std::ios::beg);
	unsigned long long int networkFileSize = htonll(static_cast<unsigned long long int>(fileSize));//
	unsigned short networkFileNameSize = htons(static_cast<short>(fileName.size()));//
	for(int i=0;i< headerSize;i++)
		fileData.emplace_back(((char*)&networkFileSize)[i]);

	for (int i = 0; i < 2; i++)
		fileData.emplace_back(((char*)&networkFileNameSize)[i]);

	for (int i = 0; i < fileName.size(); i++)
		fileData.emplace_back(fileName.data()[i]);


	char buffer;
	while (is.read(&buffer, sizeof(buffer))) {
		fileData.push_back(buffer); // deque에 추가
	}

	printf("파일 읽기 성공 : 파일 크기(%d)\n", fileSize);


	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);


	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	printf("Connect!\n");
	clearConsole();
	COORD coord;
	coord.X = 0;
	coord.Y = 0;

	std::array<char, 1024> packetData;
	int offset = 0;
	while (1) {
		// 데이터 입력
		int size = min(packetData.size(), fileData.size() - offset);
		if(size <= 0)
			break;
		std::copy(fileData.begin() + offset, fileData.begin() + offset + size, packetData.data());
		// 데이터 보내기
		retval = send(sock, packetData.data(), size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		if(retval == 0)
		{
			err_display("result = 0");
		}
		offset += retval;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		printf("전송중 : %lf%%                \n", offset/(float)(fileSize+headerSize) * 100);
		//clearConsole();
	}
	printf("전송완료\n");

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

