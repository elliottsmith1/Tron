#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector> 
#include <chrono>

//SFML
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

//Game includes
#include <Game/PacketTypes.h>
#include <Game/Player.h>
#include <Game/GridTile.h>

//Headers
#include "stdafx.h"
#include "ServerUser.h"

//ports
constexpr int SERVER_TCP_PORT(27000);
constexpr int SERVER_UDP_PORT(27001);

//sockets and clients
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<ServerUser>;

class Server
{
public:
	Server() = default;
	~Server() = default;

	//run server-side version of game
	void tickGame(TcpClients & tcp_clients);

	//send a player's server details to all clients
	void sendPlayerToAll(TcpClients & tcp_clients, int player_id);

	//send all server player details to all clients
	void sendAllPlayers(TcpClients & tcp_clients);

	//relay which players are active
	void sendConnections(TcpClients & tcp_clients);

	//start server game
	void startGame(TcpClients & tcp_clients, bool _start);

	//end server game
	void endGame();

	//bind port
	bool bindServerPort(sf::TcpListener&);

	//remove unused clients
	void clearStaleCli(TcpClients & tcp_clients);

	//connect clients
	void connect(sf::TcpListener& tcp_listener, sf::SocketSelector& selector, TcpClients& tcp_clients);

	//listen for messages
	void listen(sf::TcpListener&, sf::SocketSelector&, TcpClients&);

	//check ping
	void ping(TcpClients& tcp_clients);

	//receive a packet 
	void receiveMsg(TcpClients& tcp_clients, sf::SocketSelector& selector);

	//set up server
	void runServer();

	//initialise server side game
	void initServerGame();

private:
	//server's players
	std::vector <std::unique_ptr<Player>> m_players;

	//texture to initialise players
	sf::Texture blank_texture;

	//server version of grid
	sf::Int32             grid_height = 33;
	sf::Int32             grid_width = 44;
	std::vector<GridTile> grid;

	//game playing tick
	float time_difference = 0;

	//number of clients connected
	sf::Int32 connected_clients = 0;

	//if server game should tick
	bool run_game = false;
};

