#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

//SFML
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>

//Game classes
#include <Game/Player.h>
#include <Game/GridTile.h>

//Headers
#include "stdafx.h"
#include "GameState.h"

using namespace std::chrono_literals;

const sf::IpAddress SERVER_IP("127.0.0.1");
constexpr int SERVER_TCP_PORT(27000);
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClientPtr>;

class TronClient
{
public:	
	TronClient() = default;
	~TronClient();

	//initialise client
	void init();

	//client netowrking
	void client();

	//main game loop
	void runClient();

	//connect to server
	bool connect();

	//user input
	void input();

	//send player's details eg. direction
	void sendPlayer();

	//set sprites depending on game state
	void checkMenuState();

	//initialise sprites
	void initSprites();

	//initialise grid
	void initGrid();
	
	//check how many are connected in lobby
	void checkConnections(int _connections);

	//start
	void startGame();

	//end
	void endGame();

	//send local player death to server
	void playerDied(int _id);

	//check players for bounding box/death
	const void checkPlayers(sf::Int32 _id);

	//check to see if someone has won
	void checkWin();

	//reset assets
	void resetPlayers();

	//toggle music
	void toggleMusic();


private:

	//client socket status
	sf::Socket::Status status;

	//vector for client's players
	std::vector <std::unique_ptr<Player>> m_players;

	//client socket
	std::unique_ptr<sf::TcpSocket> m_socket;

	//cat texture
	sf::Texture c_texture;

	//trail texture
	sf::Texture t_texture;

	//start of game text sprites & textures
	sf::Texture start_texture;
	sf::Sprite  start_sprite;

	sf::Texture exit_texture;	
	sf::Sprite  exit_sprite;

	sf::Texture game_over_texture;
	sf::Sprite  game_over_sprite;

	sf::Texture lobby_texture;
	sf::Sprite  lobby_sprite;

	sf::Texture player1_texture;
	sf::Sprite  player1_sprite;

	sf::Texture player2_texture;
	sf::Sprite  player2_sprite;

	sf::Texture player3_texture;
	sf::Sprite  player3_sprite;

	sf::Texture player4_texture;
	sf::Sprite  player4_sprite;

	sf::Texture win_texture;
	sf::Sprite  win_sprite;

	sf::Texture you_texture;
	sf::Sprite  you_sprite;

	std::vector<sf::Sprite*> lobby_sprites;
	std::vector<sf::Sprite*> game_over_sprites;
	//end of sprites
	
	//circle shapes for lobby connections
	sf::CircleShape              connection_dot;
	std::vector<sf::CircleShape> connection_dots;

	//size of grid in tiles
	sf::Int32 grid_height = 33;
	sf::Int32 grid_width = 44;

	//grid 
	std::vector<GridTile> grid;
	GridTile              tile;

	//client id passed from server
	sf::Int32 id = 0;

	//checks and sets id of winner
	sf::Int32 winner = 0;

	//how many connected clients are playing
	sf::Int32 players_playing = 0;

	//how many clients are connected
	sf::Int32 connected_clients = 0;

	//game playing tick
	float time_difference = 0;	

	//game state - starts on menu
	GameState m_GS = GameState::MAIN_MENU;

	//checks if start button is highlighted 
	bool on_start = true;

	//music
	sf::Music cat_music;
};

