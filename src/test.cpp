#include "bitmex.h"
#include <iostream>
#include <string>
#include <future>


/*
	Compiling Instructions 

		> g++ test.cpp bitmex.cpp compute.cpp encrypt++/encoder.cpp -l:libcryptopp.a -lcurl -lpthread -lcpprest

*/

int main()
{
	std::string key = "";
	std::string secret = "";


	bitmex client(key, secret, 20);
	
	std::cout << client.account("XBt") << std::endl;

	return 0;
}



