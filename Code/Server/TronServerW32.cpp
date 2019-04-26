// TronServerW32.cpp : Defines the entry point for the console application.
//
#include "Server.h"

#include <thread>

int main()
{
	//initialise server
	std::unique_ptr<Server> server = std::make_unique<Server>();

	//run
	server->initServerGame();

    return 0;
}

