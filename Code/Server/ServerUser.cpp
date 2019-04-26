#include "ServerUser.h"
#include <Game/PacketTypes.h>

ServerUser::ServerUser(sf::TcpSocket* sock)
: socket(sock)
{

}



ServerUser::ServerUser(ServerUser&& rhs)
{
	this->id = rhs.id;
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
}



ServerUser& ServerUser::operator=(ServerUser&& rhs)
{
	this->id = rhs.id;
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
	return *this;
}



bool ServerUser::isConnected() const
{
	return socket->getLocalPort() != 0;
}



sf::TcpSocket& ServerUser::getSocket()
{
	return *socket;
}



void ServerUser::setLatency(std::chrono::microseconds duration)
{
	latency = duration;
}



void ServerUser::ping()
{
	//ping message
	sf::Packet packet;
	packet << NetMsg::PING;
	getSocket().send(packet);

	timestamp = std::chrono::steady_clock::now();
}



void ServerUser::pong()
{
	//pong message
	auto end = std::chrono::steady_clock::now();
	latency = std::chrono::duration_cast
		<std::chrono::microseconds>
		(end - timestamp);
	latency /= 2;
}



void ServerUser::sendID(int _id)
{
	//send id to client
	sf::Packet packet;
	packet << NetMsg::ID << _id;
	getSocket().send(packet);
}

unsigned int ServerUser::next_id = 0;