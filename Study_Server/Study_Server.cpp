#include "Common.h"


// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET clientSocket = (SOCKET)(arg);
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(clientSocket, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

	std::array<char, 1024> tempBuffer;
	Packet packet;
	while (true)
	{
		// 데이터 받기
		int receiveSize = recv(clientSocket, tempBuffer.data(), Packet::HEADER_SIZE, MSG_WAITALL);
		if (receiveSize < 0)
			break;
		packet.PushData(tempBuffer.begin(), tempBuffer.begin() + receiveSize);
		packet.Marking();
		int bodySize = packet.PeekData<int>();
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		COORD coord = csbi.dwCursorPosition;
		while(packet.Offset() < bodySize)
		{
			receiveSize = min(bodySize - packet.Offset(), tempBuffer.size());
			receiveSize = recv(clientSocket, tempBuffer.data(), receiveSize, MSG_WAITALL);
			packet.PushData(tempBuffer.begin(), tempBuffer.begin() + receiveSize);
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
			std::cout << "수신률 : " << (packet.Offset() / (double)bodySize) * 100 << "     \n";
		}
		packet.Marking();
		int fileNameSize = static_cast<int>(packet.PopData<unsigned long long int>());
		std::string fileName;
		fileName.resize(fileNameSize);
		packet.PopData((char*)fileName.data(), fileNameSize);
		int fileSize = static_cast<int>(packet.PopData<long long int>());
		std::deque<char> fileData;
		std::ofstream os{ fileName };
		for (int i = 0; i < fileSize; i++) {
			char data = packet.PopData<char>();
			os.write(&data, 1);
		}
	}
	closesocket(clientSocket);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));
	return 0;
}



int main(int argc, char* argv[])
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 0;


	sockaddr_in acceptAddr;
	sockaddr_in clientAddr;

	memset(&acceptAddr, 0, sizeof(acceptAddr));
	acceptAddr.sin_addr.s_addr = 0;
	acceptAddr.sin_family = AF_INET;
	acceptAddr.sin_port = htons(56672);


	SOCKET acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET clientSocket;
	int addrlen;
	addrlen = sizeof(clientAddr);

	SocketCheck(bind(acceptSocket, (sockaddr*)&acceptAddr, sizeof(acceptAddr)));
	SocketCheck(listen(acceptSocket, SOMAXCONN));
	std::cout << "Ready\n";
	while (true)
	{
		SocketCheck(clientSocket = accept(acceptSocket, (struct sockaddr*)&clientAddr, &addrlen));
		HANDLE threadHandle = CreateThread(0, 0, ProcessClient, (void*)clientSocket, CREATE_SUSPENDED, 0);
		ResumeThread(threadHandle);
	}

	closesocket(acceptSocket);
	WSACleanup();
	return 0;
}
