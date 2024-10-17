#include "Common.h"


int main(int argc, char* argv[])
{
	char** parameter;
#ifdef _DEBUG
	parameter = new char*[argc = 3];
	parameter[0] = (char*)"Client";
	parameter[1] = (char*)"127.0.0.1";
	parameter[2] = (char*)"test.txt";
#else
	parameter = argv;
#endif

	std::string serverIP;
	std::string fileName;
	long long int fileSize;

	if (argc > 2) {
		serverIP = parameter[1];
		fileName = std::string(parameter[2], parameter[2] + strlen(parameter[2]));
		std::cout <<"파일 읽는 중 : "<< fileName <<"\n";
	}

	std::ifstream is{ fileName, std::ios::binary };
	std::deque<char> fileData;
	{
		if (!is) {
			printf("파일 읽기 실패\n");
			return -1;
		}

		is.seekg(0, std::ios::end);
		fileSize = is.tellg();
		is.seekg(0, std::ios::beg);
	}

	Packet packet;
	char readData;

	packet.Clear();
	packet.PushData(fileName.size());
	packet.PushData(fileName.begin(), fileName.end());
	packet.PushData(fileSize);
	for (int i = 0; i < fileSize; i++) {
		is.read(&readData, 1);
		packet.PushData(readData);
	}

	packet.Marking();
	packet.PushSize();
	int fullSize;
	packet.PeekData(fullSize);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bool isDomain = false;
	for (const char& c : serverIP)
		if (isalpha(c))
			isDomain = true;
	if(isDomain)
		memcpy(&serveraddr.sin_addr, gethostbyname(serverIP.c_str())->h_addr, 4);
	else
		inet_pton(AF_INET, serverIP.c_str(), &serveraddr.sin_addr);
	serveraddr.sin_port = htons(56672);

	SocketCheck(connect(serverSocket, (sockaddr*)&serveraddr, sizeof(serveraddr)));
	printf("Connect!\n");


	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	COORD coord = csbi.dwCursorPosition;

	std::array<char, 1024> packetData;

	int offset = 0;
	int size = packetData.size();
	while (true)
	{
		size = packetData.size();
		packet.PopData(packetData.data(), size);
		int sendSize = send(serverSocket, packetData.data(), size, 0);
		packet.BackOffset(size, sendSize);
		if (sendSize == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		if (sendSize == 0)
			break;

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
		printf("전송중 : %lf%%    \n", packet.Offset() / (double)packet.GetBodySize() * 100);

	}

	printf("전송완료\n");

	int b;
	scanf("%d", &b);

	closesocket(serverSocket);

	WSACleanup();
	return 0;
}