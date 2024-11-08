#include "Common.h"

#include <concurrent_queue.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// 에디트 컨트롤 출력 함수

SOCKET serverSocket;

HINSTANCE hInst; // 인스턴스 핸들
HWND hEdit; // 에디트 컨트롤
HWND hEdit1, hEdit2, hButtonConnect, hButtonFile, hProgressBar, hStaticText, hStaticText2;

TCHAR szFile[MAX_PATH] = { 0 }; TCHAR text1[256], text2[256];

Concurrency::concurrent_queue<double> queue;
std::shared_ptr<std::thread> th;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;

	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T("MyWndClass");
	if (!RegisterClass(&wndclass)) return 1;

	// 윈도우 생성
	HWND hWnd = CreateWindow(_T("MyWndClass"), _T("WinApp"),
		WS_OVERLAPPEDWINDOW, 0, 0, 500, 220,
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		hEdit = CreateWindow(_T("edit"), NULL,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL |
			WS_VSCROLL | ES_AUTOHSCROLL |
			ES_AUTOVSCROLL | ES_MULTILINE,
			0, 0, 0, 0, hwnd, (HMENU)100, hInst, NULL);

		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
			WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
			10, 10, 130, 25, hwnd, NULL, hInst, NULL);

		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
			WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
			150, 10, 70, 25, hwnd, NULL, hInst, NULL);

		hButtonConnect = CreateWindow(TEXT("Button"), TEXT("Connect"),
			WS_CHILD | WS_VISIBLE,
			230, 10, 70, 25, hwnd, (HMENU)1, hInst, NULL);
		hStaticText = CreateWindow(TEXT("Static"), TEXT("Disconnect"),
			WS_CHILD | WS_VISIBLE,
			320, 10, 80, 25, hwnd, (HMENU)4, hInst, NULL);

		hButtonFile = CreateWindow(TEXT("Button"), TEXT("파일 선택"),
			WS_CHILD | WS_VISIBLE,
			10, 50, 100, 25, hwnd, (HMENU)2, hInst, NULL);

		hButtonFile = CreateWindow(TEXT("Button"), TEXT("파일 전송"),
			WS_CHILD | WS_VISIBLE,
			120, 50, 100, 25, hwnd, (HMENU)3, hInst, NULL);

		hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL,
			WS_CHILD | WS_VISIBLE,
			10, 90, 300, 20, hwnd, NULL, hInst, NULL);

		hStaticText2 = CreateWindow(TEXT("Static"), TEXT("수정불가 텍스트"),
			WS_CHILD | WS_VISIBLE,
			10, 120, 300, 20, hwnd, NULL, hInst, NULL);

		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (serverSocket == INVALID_SOCKET) err_quit("socket()");

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 1:
		{
			GetWindowText(hEdit1, text1, 256);
			GetWindowText(hEdit2, text2, 256);
			std::string text1s = to_string(text1);
			std::string text2s = to_string(text2);

			std::string serverIP = text1s;
			int port = std::stoi(text2s);
			struct sockaddr_in serveraddr;
			memset(&serveraddr, 0, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			bool isDomain = false;
			for (const char& c : serverIP)
				if (isalpha(c))
					isDomain = true;
			if (isDomain)
				memcpy(&serveraddr.sin_addr, gethostbyname(serverIP.c_str())->h_addr, 4);
			else
				inet_pton(AF_INET, serverIP.c_str(), &serveraddr.sin_addr);
			serveraddr.sin_port = htons(port);

			SocketCheck(connect(serverSocket, (sockaddr*)&serveraddr, sizeof(serveraddr)));
			printf("Connect!\n");
			SetWindowText(hStaticText, L"connect");
		break;
		}
		case 2:
		{
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = szFile;
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = TEXT("모든 파일(*.*)\0*.*\0");
			ofn.nFilterIndex = 1;
			if (GetOpenFileName(&ofn))
			{

			}
		}
		break;
		case 3:
		{
			th = std::make_shared<std::thread>([&]() {
				std::string textPath = to_string(szFile);
				if (true)
				{
					std::string fileName = textPath;
					long long int fileSize;

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

					// 수신률 업데이트 함수

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


					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
					COORD coord = csbi.dwCursorPosition;

					std::array<char, 1024> packetData;

					SendMessage(hProgressBar, PBM_SETRANGE32, 0, (double)packet.Size());
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
						SendMessage(hProgressBar, PBM_SETPOS, packet.Offset(), 0);
						SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
						printf("전송중 : %lf%%    \n", packet.Offset() / (double)packet.Size() * 100);

					}

					printf("전송완료\n");
				}

			});
		}
		break;


		}

		break;
	case WM_SIZE:
		//MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hEdit);
		return 0;
	case WM_DESTROY:

		closesocket(serverSocket);
		WSACleanup();

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

