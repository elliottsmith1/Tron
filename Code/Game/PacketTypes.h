#pragma once
#include <SFML/Network.hpp>

//different packet types
enum NetMsg : sf::Int32
{
	INVALID = 0,	
	PLAYER_DATA = 1,
	PING = 2,
	PONG = 3,
	CLIENT_COUNT = 4,
	ID = 5,
	PLAYER_INPUT = 6,
	CONNECTED_CLIENTS = 7,
	RUN_GAME = 8,
	TRIGGER_START = 9,
	PLAYER_DIED = 10,
	END_GAME = 11
};