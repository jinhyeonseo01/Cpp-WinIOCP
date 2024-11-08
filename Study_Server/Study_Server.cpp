#include <mutex>

#include "Common.h"


std::mutex mutex;
void WriteProgress(double per, COORD coord)
{
	std::lock_guard<std::mutex> lock(mutex);
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	std::cout << "수신률 : " << (per) * 100 << "     \n";
}

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
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	COORD coord = csbi.dwCursorPosition;
	std::cout << "\n";
	std::array<char, 1024*32> tempBuffer;
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
		while(packet.Offset() < bodySize)
		{
			receiveSize = min(bodySize - packet.Offset(), tempBuffer.size());
			receiveSize = recv(clientSocket, tempBuffer.data(), receiveSize, MSG_WAITALL);
			if (receiveSize < 0)
				return 0;
			packet.PushData(tempBuffer.begin(), tempBuffer.begin() + receiveSize);
			WriteProgress(packet.Offset() / (double)bodySize, coord);
		}
		packet.Marking();
		int fileNameSize = static_cast<int>(packet.PopData<unsigned long long int>());
		std::string fileName;
		fileName.resize(fileNameSize);
		packet.PopData((char*)fileName.data(), fileNameSize);
		int fileSize = static_cast<int>(packet.PopData<long long int>());
		std::deque<char> fileData;
		for(auto it = fileName.rbegin(); it != fileName.rend();++it)
		{
			if(*it == '\\')
			{
				auto base_it = it.base();
				fileName = std::string(base_it, fileName.end());
				break;
			}
		}
		std::cout << "파일 저장" << fileName << "\n";
		std::ofstream os{ fileName , std::ios::binary };
		for (int i = 0; i < fileSize; i++) {
			char data = packet.PopData<char>();
			os.write(&data, 1);
		}
		packet.Clear();
		std::cout << "완료\n";
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
