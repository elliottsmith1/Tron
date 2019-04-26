#include "TronClient.h"

int main()
{
	//initialise client
	std::unique_ptr<TronClient> client = std::make_unique<TronClient>();

	//run
	client->init();

	return 0;
}

