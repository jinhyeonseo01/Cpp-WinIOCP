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
	std::string originalDomainName; // www.pinterest.com
	std::string splitDomainName;

	std::cin >> originalDomainName;

	std::string targetKeyword = "www";
	size_t size = originalDomainName.find(targetKeyword);

	if (size != std::string::npos) {
		size += targetKeyword.size() + 1;
		splitDomainName = originalDomainName.substr(size, originalDomainName.size() - size);
	}
	else
		splitDomainName = originalDomainName;



	struct hostent* domainInfo;
	in_addr addr;
	std::array<char, 22> arr;
	int count = 0;

	domainInfo = gethostbyname(splitDomainName.c_str());

	if (domainInfo != nullptr)
	{
		for (count = 0; domainInfo->h_addr_list[count] != NULL; count++);
		std::cout << "IP 갯수 : " << count << "\n";
		for (int j = 0; domainInfo->h_addr_list[j] != NULL; j++)
		{
			inet_ntop(AF_INET, &addr, arr.data(), arr.size());
			memcpy(&addr, domainInfo->h_addr_list[j], domainInfo->h_length);
			std::cout << arr.data() << "\n";
		}

		std::cout << "\n\n";
	}


	domainInfo = gethostbyname(originalDomainName.c_str());
	if (domainInfo != nullptr)
	{
		for (count = 0; domainInfo->h_aliases[count] != NULL; count++);
			std::cout << "별명 갯수 : " << count << "\n";
		for (int j = 0; domainInfo->h_aliases[j] != NULL; j++)
		{
			std::array<char, 200> arr;
			memcpy(arr.data(), domainInfo->h_aliases[j], strlen(domainInfo->h_aliases[j]) + 1);
			std::cout << arr.data() << "\n";
		}
	}

	WSACleanup();
	return 0;
}

