#pragma once
#include <chrono>
#include <memory>

//SFML
#include <SFML/Network.hpp>

using namespace std::chrono_literals;
class ServerUser
{
public:

	ServerUser() = default;
	ServerUser(sf::TcpSocket*);
	ServerUser(ServerUser&&);

	//return socket
	sf::TcpSocket& getSocket();

	//check connection
	bool isConnected() const;

	//set latency
	void setLatency(std::chrono::microseconds);

	//ping checks
	void ping();
	void pong();

	//send client id
	void sendID(int _id);

	//getters
	const auto& getPingTime() const { return timestamp; }
	const auto& getLatency()  const { return latency; }
	int getClientID()         const { return id; }

	//set id
	void setID(int _id) { id = _id; }

	ServerUser& operator=(ServerUser&&);
	bool operator==(const ServerUser& rhs) { return id == rhs.id; }

private:

	static unsigned int next_id;

	//socket
	std::unique_ptr<sf::TcpSocket> socket = nullptr;

	//latency
	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	std::chrono::microseconds			  latency = 100us;

	//id
	int id = 0;
};
