#include <algorithm>
#include <iostream>

//SFML
#include <SFML/Network.hpp>

//Game headers
#include <Game/PacketTypes.h>
#include <Game/Player.h>
#include <Game/GridTile.h>

//Headers
#include "stdafx.h"
#include "ServerUser.h"
#include "Server.h"

void Server::initServerGame()
{
	if (!blank_texture.loadFromFile("../../Resources/Textures/Nyancat.png"))
	{
		// error
	}

	//set up server players and grid
	m_players.reserve(4);
	grid.reserve(grid_height*grid_width);

	for (int i = 0; i < 4; i++)
	{
		std::unique_ptr<Player> player = std::make_unique<Player>(blank_texture, i);

		m_players.push_back(std::move(player));
	}

	for (int i = 0; i < (grid_height * grid_width); i++)
	{
		std::unique_ptr<GridTile> tile = std::make_unique<GridTile>();

		grid.push_back(*tile);
	}

	//run
	runServer();
}



void Server::tickGame(TcpClients & tcp_clients)
{
	//tick server game
	auto start = std::chrono::steady_clock::now();

	//if set to run
	if (run_game)
	{
		for (int i = 0; i < m_players.size(); i++)
		{
			sf::Int32 next_tile = 0;

			if (m_players[i]->getActive())
			{
				if (m_players[i]->getAlive())
				{
					//move player on grid depending on direction
					sf::Int32 p_pos = m_players[i]->getGridPos();

					grid[p_pos].setActive(true);

					switch (m_players[i]->getDirection())
					{
					case 0:
						next_tile = p_pos + 1;

						m_players[i]->setGridPos(p_pos + 1);

						break;

					case 1:
						next_tile = p_pos + 44;

						m_players[i]->setGridPos(p_pos + 44);	

						break;

					case 2:
						next_tile = p_pos - 1;

						m_players[i]->setGridPos(p_pos - 1);

						break;

					case 3:
						next_tile = p_pos - 44;

						m_players[i]->setGridPos(p_pos - 44);	

						break;

					default:
						next_tile = p_pos + 1;

						m_players[i]->setGridPos(p_pos + 1);	

						break;

					}

					//if about to hit an active tile then die
					if (grid[next_tile].getActive())
					{
						m_players[i]->setAlive(false);
					}					
				}
			}
		}

		//relay new player information to clients
		sendAllPlayers(tcp_clients);
	}

	//DT (not used)
	auto end = std::chrono::steady_clock::now();
	auto difference = std::chrono::duration<float>(end - start);
	time_difference = 0.1 + difference.count();
}



void Server::sendPlayerToAll(TcpClients & tcp_clients, int player_id)
{	
	//send player details to clients
	//id
	sf::Int32 playerID = player_id;

	//direction
	sf::Int32 playerRot = m_players[playerID]->getDirection();

	//active state
	bool playerActive = m_players[playerID]->getActive();

	//grid position
	sf::Int32 p_pos = m_players[playerID]->getGridPos();

	//alive state
	bool player_alive = m_players[playerID]->getAlive();

	/*std::cout << "Latency: " << sender.getLatency().count()	<< "us" << std::endl;*/
	
	sf::Packet packet;

	packet << NetMsg::PLAYER_DATA << playerRot << playerID << playerActive << p_pos << player_alive;

	//send the packet to other clients
		
	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}



void Server::sendConnections(TcpClients & tcp_clients)
{
	//send whici clients are active to all clients
	sf::Packet packet;
	bool client_1_connect = m_players[0]->getActive();
	bool client_2_connect = m_players[1]->getActive();
	bool client_3_connect = m_players[2]->getActive();
	bool client_4_connect = m_players[3]->getActive();

	packet << NetMsg::CONNECTED_CLIENTS << connected_clients;
	packet << client_1_connect;
	packet << client_2_connect;
	packet << client_3_connect;
	packet << client_4_connect;

	//if at least 1
	if (connected_clients > 0)
	{
		for (auto& client : tcp_clients)
		{
			client.getSocket().send(packet);
		}
	}
}



void Server::startGame(TcpClients & tcp_clients, bool _start)
{
	//start running server game and tell all clients
	bool start = _start;

	sf::Packet packet;

	packet << NetMsg::TRIGGER_START << start;

	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}



void Server::endGame()
{
	//reset server game
	for (int i = 0; i < m_players.size(); i++)
	{
		m_players[i]->setSpawn();
		m_players[i]->setAlive(true);
	}

	for (auto& tile : grid)
	{
		tile.setActive(false);
		tile.turnSquare(false);
	}
}



void Server::sendAllPlayers(TcpClients & tcp_clients)
{
	//send all player's details
	for (int i = 0; i < 4; i++)
	{
		if (m_players[i]->getActive())
		{
			sendPlayerToAll(tcp_clients, i);
		}
	}
}



bool Server::bindServerPort(sf::TcpListener &listener)
{
	//try to bind to port
	if (listener.listen(SERVER_TCP_PORT) != sf::Socket::Done)
	{
		std::cout << "Could not bind server port";
		std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
		std::cout << std::endl;
		return false;
	}

	else
	{
		std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
		std::cout << std::endl;
		return true;
	}
}



void Server::listen(sf::TcpListener &tcp_listener, sf::SocketSelector &selector, TcpClients &tcp_clients)
{
	//listen for messages
	while (true)
	{
		const sf::Time timeout = sf::Time(sf::milliseconds(250));
		if (selector.wait(timeout))
		{
			if (selector.isReady(tcp_listener))
			{
				connect(tcp_listener, selector, tcp_clients);
			}
			else
			{
				receiveMsg(tcp_clients, selector);
				clearStaleCli(tcp_clients);
			}
		}

		else
		{
			ping(tcp_clients);
		}

		//if set to then run server game on loop here
		if (run_game)
		{
			tickGame(tcp_clients);
		}
	}
}



void Server::connect(sf::TcpListener &tcp_listener, sf::SocketSelector &selector, TcpClients &tcp_clients)
{
	//connect new clients
	auto  client_ptr = new sf::TcpSocket;
	auto& client_ref = *client_ptr;

	if (tcp_listener.accept(client_ref) == sf::Socket::Done)
	{
		selector.add(client_ref);

		ServerUser client(client_ptr);

		connected_clients++;

		int clientID = 0;

		//set up connected player
		for (int i = 0; i < 4; i++)
		{
			if (!m_players[i]->getActive())
			{
				m_players[i]->setActive(true);
				m_players[i]->setServerPosition(m_players[i]->getPosition());
				clientID = i;
				client.setID(i);
				break;
			}
		}

		//send their id to client
		sf::Packet packet;
		packet << NetMsg::ID << clientID;
		client.getSocket().send(packet);

		tcp_clients.push_back(std::move(client));
		std::cout << "client connected" << std::endl;		

		std::string client_count = std::to_string(tcp_clients.size());
		std::cout << "There are " + client_count + " connected clients" << std::endl;

		sendConnections(tcp_clients);
	}
}



void Server::receiveMsg(TcpClients &tcp_clients, sf::SocketSelector &selector)
{
	//receive packets
	for (auto& sender : tcp_clients)
	{
		auto& sender_socket = sender.getSocket();

		if (selector.isReady(sender_socket))
		{
			sf::Packet packet;

			//disconnect client
			if (sender_socket.receive(packet) == sf::Socket::Disconnected)
			{
				selector.remove(sender_socket);
				sender_socket.disconnect();
				std::cout << "Client (" << sender.getClientID() << ") Disconnected" << std::endl;

				m_players[sender.getClientID()]->setActive(false);
				connected_clients--;
				sendConnections(tcp_clients);

				break;
			}

			int header = 0;
			packet >> header;

			NetMsg msg = static_cast<NetMsg>(header);

			//pong message
			if (msg == NetMsg::PONG)
			{
				sender.pong();
			}

			//set player to dead
			else if (msg == NetMsg::PLAYER_DIED)
			{
				sf::Int32 player_id;

				packet >> player_id;

				if (m_players[player_id]->getAlive())
				{
					m_players[player_id]->setAlive(false);
				}
			}

			//change player direction
			else if (msg == NetMsg::PLAYER_INPUT)
			{				
				sf::Int32 playerRot;
				sf::Int32 playerID;				
				bool player_alive;

				packet >> playerRot >> playerID >> player_alive;

				m_players[playerID]->setDirection(playerRot);

				if (!m_players[playerID]->getAlive())
				{
					m_players[playerID]->setAlive(player_alive);
				}
			}		

			//start game
			else if (msg == NetMsg::RUN_GAME)
			{
				run_game = true;

				startGame(tcp_clients, true);
			}

			//end game
			else if (msg == NetMsg::END_GAME)
			{
				run_game = false;

				endGame();
			}
		}
	}
}



void Server::runServer()
{
	//setup and call loop
	sf::TcpListener tcp_listener;

	if (!bindServerPort(tcp_listener))
	{
		return;
	}

	sf::SocketSelector selector;
	selector.add(tcp_listener);

	TcpClients tcp_clients;

	return listen(tcp_listener, selector, tcp_clients);
}



void Server::clearStaleCli(TcpClients & tcp_clients)
{
	//clear non active clients
	tcp_clients.erase(std::remove_if(tcp_clients.begin(), tcp_clients.end(), [](const ServerUser& client)
	{
		return(!client.isConnected());
	}), tcp_clients.end());
}



void Server::ping(TcpClients& tcp_clients)
{
	//ping message
	constexpr auto timeout = 10s;
	for (auto& client : tcp_clients)
	{
		const auto& timestamp = client.getPingTime();
		const auto  now = std::chrono::steady_clock::now();
		auto delta = now - timestamp;

		if (delta > timeout)
		{
			client.ping();
		}
	}
}

