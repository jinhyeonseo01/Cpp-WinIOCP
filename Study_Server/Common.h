#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����

#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...

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
#include <thread>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
	void* lpMsgBuf;
	FormatMessageA( //A = Ascii
		FORMAT_MESSAGE_ALLOCATE_BUFFER // ���� �ڵ带 �ʰ� �о��
		| FORMAT_MESSAGE_FROM_SYSTEM, // �׸��� �װ� �ý��ۿ� �־�
		nullptr/* ��� ã���� �����ʿ� X*/, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		// MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) ������ �ü�� �⺻ ���� �������� ������.
		(char*)&lpMsgBuf/*������ ���� �޸�*/, 0/*ũ�⵵ �ڵ� ����*/, nullptr/*�ƱԸ�Ʈ�� �ڵ� ����*/);
	//lpMsgBuf�޸� �ѱ��
	MessageBoxA(nullptr, static_cast<const char*>(lpMsgBuf), msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1); // ������ �ɰ��ϸ� ������
}


// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, nullptr);
	printf("[%s] %s\n", msg, static_cast<char*>(lpMsgBuf));
	LocalFree(lpMsgBuf);
}

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, nullptr);
	printf("[����] %s\n", static_cast<char*>(lpMsgBuf));
	LocalFree(lpMsgBuf);
}


void clearConsole()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

inline bool SocketCheck(int code)
{
	if (code < 0)
	{
		std::cout << "Socket Error\n";
		exit(-1);
	}
	return code >= 0;
}


class Packet

{
public:
	static int HEADER_SIZE;

	int bodySize = 0;

	std::deque<uint8_t> data;

	int offset = 0;

	int marker = 0;


	void PushData(const unsigned long long int& data)

	{
		unsigned long long int netData = htonll(data);

		this->data.resize(this->data.size() + 8);

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[0];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[1];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[2];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[3];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[4];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[5];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[6];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[7];
	}

	void PushData(const unsigned int& data)

	{
		unsigned int netData = htonl(data);

		this->data.resize(this->data.size() + 4);

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[0];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[1];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[2];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[3];
	}

	void PushData(const unsigned short& data)

	{
		unsigned short netData = htons(data);

		this->data.resize(this->data.size() + 2);

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[0];

		this->data[offset++] = reinterpret_cast<uint8_t*>(&netData)[1];
	}

	void PushData(const unsigned char& data)

	{
		this->data.resize(this->data.size() + 1);

		this->data[offset++] = data;
	}

	void PushData(const long long int& data)

	{
		PushData(*reinterpret_cast<const long long unsigned int*>(&data));
	}

	void PushData(const int& data)

	{
		PushData(*reinterpret_cast<const unsigned int*>(&data));
	}

	void PushData(const short& data)

	{
		PushData(*reinterpret_cast<const unsigned short*>(&data));
	}

	void PushData(const char& data)

	{
		PushData(*reinterpret_cast<const unsigned char*>(&data));
	}

	void PushData(const float& data)

	{
		PushData(*reinterpret_cast<const unsigned int*>(&data));
	}

	void PushData(const double& data)

	{
		PushData(*reinterpret_cast<const unsigned long long int*>(&data));
	}

	template <class T, class = std::enable_if_t<std::is_base_of_v<
		std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>>>

	void PushData(T begin, T end)

	{
		for (; begin != end; ++begin)

			PushData(*begin);
	}

	void PushSize()

	{
		unsigned int netData = htonl(static_cast<unsigned int>(Size()));


		for (int i = HEADER_SIZE - 1; i >= 0; --i)

			this->data.push_front(reinterpret_cast<uint8_t*>(&netData)[i]);

		marker += HEADER_SIZE;
	}

	void PopData(unsigned long long int& data)

	{
		unsigned long long netData = 0;

		reinterpret_cast<uint8_t*>(&netData)[0] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[1] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[2] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[3] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[4] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[5] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[6] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[7] = this->data[offset++];

		data = ntohll(netData);
	}

	void PopData(unsigned int& data)

	{
		unsigned int netData = 0;

		reinterpret_cast<uint8_t*>(&netData)[0] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[1] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[2] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[3] = this->data[offset++];


		data = ntohl(netData);
	}

	void PopData(unsigned short& data)

	{
		unsigned short netData = 0;

		reinterpret_cast<uint8_t*>(&netData)[0] = this->data[offset++];

		reinterpret_cast<uint8_t*>(&netData)[1] = this->data[offset++];


		data = ntohs(netData);
	}

	void PopData(unsigned char& data)

	{
		data = this->data[offset++];
	}

	void PopData(long long int& data)

	{
		unsigned long long int temp;

		PopData(temp);

		data = *reinterpret_cast<long long int*>(&temp);
	}

	void PopData(int& data)

	{
		unsigned int temp;

		PopData(temp);

		data = *reinterpret_cast<int*>(&temp);
	}

	void PopData(short& data)

	{
		unsigned short temp;

		PopData(temp);

		data = *reinterpret_cast<short*>(&temp);
	}

	void PopData(char& data)

	{
		unsigned char temp;

		PopData(temp);

		data = *reinterpret_cast<char*>(&temp);
	}

	void PopData(float& data)

	{
		unsigned int temp;

		PopData(temp);

		data = *reinterpret_cast<float*>(&temp);
	}

	void PopData(double& data)

	{
		unsigned long long int temp;

		PopData(temp);

		data = *reinterpret_cast<double*>(&temp);
	}

	template <class T>

	T PopData()

	{
		T temp;

		PopData(temp);

		return temp;
	}

	template <class T>

	T PeekData()

	{
		T temp;

		int prevOffset = this->offset;

		PopData(temp);

		this->offset = prevOffset;

		return temp;
	}

	template <class T>

	void PeekData(T& t)

	{
		int prevOffset = this->offset;

		PopData(t);

		this->offset = prevOffset;
	}

	void PeekData(char* data, int& size)

	{
		Copy(data, size);
	}

	void PopData(char* data, int& size)

	{
		Copy(data, size);

		offset += size;
	}

	template <class T, class = std::enable_if_t<std::is_base_of_v<
		std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>>>

	void PopData(T iter, int& size)

	{
		for (int i = 0; i < size; i++)

		{
			*iter = PopData<T>();

			offset += size;

			++iter;
		}
	}


	void Marking()

	{
		marker = offset;

		offset = 0;
	}

	int Size()

	{
		return marker;
	}

	int Offset()

	{
		return offset;
	}

	void Copy(char* data, int& size)

	{
		size = min(max(Size() - offset, 0), size);

		for (int i = 0; i < size; i++)

			memcpy(&data[i], &this->data[offset + i], 1);
	}


	void Clear()
	{
		offset = 0;

		marker = 0;
	}

	void OffsetClear()
	{
		offset = 0;
	}

	void BackOffset(int trySize, int realSize)
	{
		offset -= trySize - realSize;
	}
};


int Packet::HEADER_SIZE = 4;

// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(const char* fmt, ...);

HINSTANCE hInst; // �ν��Ͻ� �ڵ�
HWND hEdit; // ����Ʈ ��Ʈ��

#pragma comment(lib, "comctl32.lib")

inline std::string to_string(const wchar_t* _first, UINT codePage = CP_THREAD_ACP) noexcept
{
	int size = std::wcslen(_first);
	if (size == 0)
		return {};
	std::string str2;
	str2.resize(WideCharToMultiByte(codePage, 0, _first, size, nullptr, 0, nullptr, nullptr));
	WideCharToMultiByte(codePage, 0, _first, size, const_cast<LPSTR>(str2.data()), static_cast<int>(str2.capacity()), nullptr, nullptr);
	return std::move(str2);
}