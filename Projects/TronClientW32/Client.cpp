#include "Client.h"
#include "stdafx.h"
#include "Player.h"

#include <SFML/Graphics.hpp>
#include <future>
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <vector>

void runClient()
{
	sf::RenderWindow window(sf::VideoMode(1000, 800), "TRON");

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(player_one->getBike());
		window.display();

		player_one->playerTick();
	}
}

void client()
{
	TcpClient socket;
	if (!connect(socket))
	{
		return;
	}

	//auto handle = std::async(std::launch::async, [&]
	//{
	//	// keep track of the socket status		
	//	do
	//	{
	//		sf::Packet packet;
	//		status = socket.receive(packet);

	//		if (status == sf::Socket::Done)
	//		{
	//			std::string string;

	//			packet >> string;

	//			std::cout << string << std::endl;
	//		}

	//	} while (status != sf::Socket::Disconnected);
	//});

	//return input(socket);
}

bool connect(TcpClient &socket)
{
	// attempt connection to server
	auto status = socket.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return false;
	}
	// all good
	std::cout << "Connected to server: " << SERVER_IP;
	std::cout << std::endl;
	return true;
}

void input(TcpClient &socket)
{
	do
	{
		if (status == sf::Socket::Done)
		{
			sf::Packet packet;
			std::string string;

			std::cin >> string;

			packet << string;

			socket.send(packet);
		}
	} while (status != sf::Socket::Disconnected);
}