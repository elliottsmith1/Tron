#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

//SFML
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

//Game headers
#include <Game/PacketTypes.h>
#include <Game/Player.h>
#include <Game/GameObject.h>

//Headers
#include "TronClient.h"
#include "stdafx.h"

TronClient::~TronClient()
{
	//clear vectors
	lobby_sprites.clear();
	connection_dots.clear();
}

void TronClient::init()
{
	initSprites();

	if (!t_texture.loadFromFile("../../Resources/Textures/Trail.png"))
	{
		// error
	}

	//if (!cat_music.openFromFile("..\..\Resources\Audio\Nyan.wav"))
	//{
	//	// error
	//}

	//cat_music.setLoop(true);
	//cat_music.setVolume(100);

	//cat_music.play();

	grid.reserve(grid_height*grid_width);

	initGrid();

	m_socket = std::make_unique<sf::TcpSocket>();

	m_players.reserve(4);

	for (int i = 0; i < 4; i++)
	{
		std::unique_ptr<Player> player = std::make_unique<Player>(c_texture, i);

		m_players.push_back(std::move(player));		
	}

	resetPlayers();

	//thread client networking
	std::thread first(&TronClient::client, this);

	//main loop
	runClient();

	//join thread when done
	first.join();
}



void TronClient::runClient()
{
	sf::RenderWindow window(sf::VideoMode(1024, 768), "TRON");

	//if this clients player isn't active then set to active
	if (!m_players[id]->getActive())
	{
		m_players[id]->setActive(true);
	}

	//toggleMusic();

	//run until exit
	while (m_GS != GameState::EXIT)
	{
		auto start = std::chrono::steady_clock::now();
		window.clear();

		switch (m_GS)
		{
		case GameState::MAIN_MENU:
			//menu state
			checkMenuState();

			window.draw(start_sprite);
			window.draw(exit_sprite);

			break;

		case GameState::LOBBY:
			//lobby state
			checkMenuState();

			checkConnections(connected_clients);

			//draw all relavent sprites and shapes
			for (auto& sprite : lobby_sprites)
			{
				window.draw(*sprite);
			}

			for (auto& dot : connection_dots)
			{
				window.draw(dot);
			}

			for (auto& player : m_players)
			{
				window.draw(player->getSprite());
			}

			break;

		case GameState::PLAYING:
			//playing state
			players_playing = 0;

			//draw all game relavent assets
			for (int i = 0; i < m_players.size(); i++)
			{
				if (m_players[i]->getActive())
				{
					players_playing++;
				}
			}

			for (auto& tile : grid)
			{
				if (tile.getActive())
				{
					window.draw(tile.getTile());
				}
			}

			for (int i = 0; i < m_players.size(); i++)
			{
				if (m_players[i]->getActive())
				{
					window.draw(m_players[i]->getSprite());
				}
			}

			//check for winner
			checkWin();

			break;

		case GameState::GAME_OVER:
			//game over state

			//draw all game over assets
			for (auto& tile : grid)
			{
				if (tile.getActive())
				{
					window.draw(tile.getTile());
				}
			}

			for (int i = 0; i < m_players.size(); i++)
			{
				if (m_players[i]->getActive())
				{
					window.draw(m_players[i]->getSprite());
				}
			}

			checkMenuState();

			for (auto& sprite : game_over_sprites)
			{
				window.draw(*sprite);
			}

			//depending on who wins, displayer their player number
			switch (winner)
			{
			case 0:
				window.draw(player1_sprite);
				break;
			case 1:
				window.draw(player2_sprite);
				break;
			case 2:
				window.draw(player3_sprite);
				break;
			case 3:
				window.draw(player4_sprite);
				break;
			default:
				window.draw(player1_sprite);
				break;
			}

			break;

		case GameState::EXIT:
			//exit state closes window
			window.close();

			break;

		default:
			break;
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			//poll for window close
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			//only allow input if one of the 4 players
			if (id < 4)
			{
				if (event.type == sf::Event::KeyPressed)
				{
					//only poll for input when key pressed
					input();
				}
			}
		}

		window.display();

		//DT timer
		auto end = std::chrono::steady_clock::now();
		auto difference = std::chrono::duration<float>(end - start);
		time_difference = difference.count();
	}
}



void TronClient::client()
{
	//exit if not connected
	if (!connect())
	{
		m_GS = GameState::EXIT;
		return;
	}

	auto handle = std::async(std::launch::async, [&]
	{
		// keep track of the socket status
		sf::Socket::Status status;

		do
		{
			//receive packets here
			sf::Packet packet;
			status = m_socket->receive(packet);
			if (status == sf::Socket::Done)
			{
				//check packet type using header
				int header = 0;
				packet >> header;

				NetMsg msg = static_cast<NetMsg>(header);

				//ping check
				if (msg == NetMsg::PING)
				{
					sf::Packet pong;
					pong << NetMsg::PONG;
					m_socket->send(pong);
				}

				//receive id
				else if (msg == NetMsg::ID)
				{
					sf::Int32 _id = 0;
					packet >> _id;

					id = _id;
				}

				//receive which clients are active
				else if (msg == NetMsg::CONNECTED_CLIENTS)
				{
					sf::Int32 clients = 0;
					bool client1 = false;
					bool client2 = false;
					bool client3 = false;
					bool client4 = false;

					packet >> clients >> client1 >> client2 >> client3 >> client4;

					if (clients < 5)
					{
						connected_clients = clients;
						m_players[0]->setActive(client1);
						m_players[1]->setActive(client2);
						m_players[2]->setActive(client3);
						m_players[3]->setActive(client4);

						checkConnections(connected_clients);
					}
				}

				//another client stated the game
				else if (msg == NetMsg::TRIGGER_START)
				{
					resetPlayers();

					bool start = false;
					packet >> start;

					if (start)
					{
						m_GS = GameState::PLAYING;
					}
				}

				//receive player details from server game
				else if (msg == NetMsg::PLAYER_DATA)
				{
					sf::Int32 playerRot;
					sf::Int32 playerID;
					bool player_active;
					sf::Int32 p_pos;
					bool player_alive;

					packet >> playerRot >> playerID >> player_active >> p_pos >> player_alive;

					//if receiving data about a locally non active player, set to active
					if (player_active != m_players[playerID]->getActive())
					{
						if (player_active)
						{
							m_players[playerID]->setActive(true);
						}

						else
						{
							m_players[playerID]->setActive(false);
						}

					}

					//if a player on server is active relay information to local player
					if (player_active)
					{
						//position on grid
						m_players[playerID]->setGridPos(p_pos);

						//soon to be previous position on grid
						sf::Int32 behind_player = p_pos;

						float posX = grid[p_pos].getPosition().x - m_players[playerID]->getPosition().x;
						float posY = grid[p_pos].getPosition().y - m_players[playerID]->getPosition().y;

						//move sprite
						m_players[playerID]->moveBike(posX, posY);

						//change direction
						m_players[playerID]->setDirection(playerRot);

						//set alive state
						m_players[playerID]->setAlive(player_alive);

						//set grid position begind player
						if (!grid[behind_player].getActive())
						{
							//if moving on y axis then turn grid tile
							switch (playerRot)
								{
								case 1:
								case 3:
									grid[behind_player].turnSquare(true);
									break;
								case 2:
								case 0:
									grid[behind_player].turnSquare(false);
									break;
								default:
									break;
								}

							//set grid to active
							grid[behind_player].setActive(true);
						}

						//check if players have hit bounds
						checkPlayers(playerID);		
					}
				}
			}
		} while (m_GS != GameState::EXIT);
	});
}



const void TronClient::checkPlayers(sf::Int32 _id)
{
	//check players on bounds
	if (m_players[_id]->getAlive())
	{
		m_players[_id]->boundingBox();

		//if hit bounds then send death to other clients
		if (!m_players[_id]->getAlive())
		{
			playerDied(_id);
		}
	}	

	else
	{
		//if dead then set to red
		m_players[_id]->deathColour();
	}
}



void TronClient::checkWin()
{
	//check if someones won

	//when not single player
	if (players_playing > 1)
	{
		int players_dead = 0;

		for (int i = 0; i < m_players.size(); i++)
		{
			if (m_players[i]->getActive())
			{
				if (!m_players[i]->getAlive())
				{
					//if not still alive then increment
					players_dead++;
				}
			}
		}

		//check if one player left
		if (players_dead == (players_playing - 1))
		{
			for (int i = 0; i < m_players.size(); i++)
			{
				if (m_players[i]->getActive())
				{
					if (m_players[i]->getAlive())
					{
						//set winner id
						winner = i;
					}
				}
			}

			//end the game
			endGame();
			m_GS = GameState::GAME_OVER;
			on_start = false;			
		}
	}

	//single player
	else
	{
		for (int i = 0; i < m_players.size(); i++)
		{
			if (m_players[i]->getActive())
			{
				if (!m_players[i]->getAlive())
				{
					endGame();
					m_GS = GameState::GAME_OVER;
					on_start = false;						
				}
			}
		}
	}
}



void TronClient::resetPlayers()
{
	//reset players
	for (int i = 0; i < m_players.size(); i++)
	{
		float x_pos = grid[m_players[i]->getSpawn()].getPosition().x;
		float y_pos = grid[m_players[i]->getSpawn()].getPosition().y;

		m_players[i]->setPosition(x_pos, y_pos);
		m_players[i]->setAlive(true);

		m_players[i]->createBike();
	}

	//reset grid
	for (auto& tile : grid)
	{
		tile.setActive(false);
	}
}

void TronClient::toggleMusic()
{
	/*if (cat_music.getStatus())
	{
		cat_music.stop();
	}

	else
	{
		cat_music.play();
	}*/

	/*cat_music.play();*/
}



bool TronClient::connect()
{
	// attempt connection to server
	auto status = m_socket->connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return false;
	}

	// all good
	m_socket->setBlocking(false);
	return true;
}



void TronClient::input()
{
	if (status == sf::Socket::Done)
	{
		//different input depending on state
		switch (m_GS)
		{
		case GameState::MAIN_MENU:

			//on start button
			if (on_start)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					on_start = false;
				}

				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
				{
					m_GS = GameState::LOBBY;
				}
			}

			//on exit button
			else
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					on_start = true;
				}

				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
				{
					m_GS = GameState::EXIT;
				}
			}

			break;

		case GameState::LOBBY:

			//on start button
			if (on_start)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					on_start = false;
				}

				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
				{
					winner = 0;

					m_GS = GameState::PLAYING;
					startGame();
				}
			}

			//on exit button
			else
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					on_start = true;
				}

				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
				{
					m_GS = GameState::MAIN_MENU;
					on_start = true;
				}
			}

			break;

		case GameState::PLAYING:

			//show lobby during game
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			{
				m_GS = GameState::LOBBY;
			}

			//player input to change direction
			//sends to server
			//only allow left and right turning (can't double back)
			if (m_players[id]->getAlive())
			{
				if ((m_players[id]->getDirection() == 1) || (m_players[id]->getDirection() == 3))
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
					{
						m_players[id]->setDirection(0);
						sendPlayer();
					}

					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
					{
						m_players[id]->setDirection(2);
						sendPlayer();
					}
				}

				else if ((m_players[id]->getDirection() == 0) || (m_players[id]->getDirection() == 2))
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
					{
						m_players[id]->setDirection(1);
						sendPlayer();
					}

					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
					{
						m_players[id]->setDirection(3);
						sendPlayer();
					}
				}
			}

			break;

		case GameState::GAME_OVER:

			//return to lobby
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
			{				
				resetPlayers();
				m_GS = GameState::LOBBY;
				on_start = true;
			}

			break;

		case GameState::EXIT:
			//this will auto close the window
			break;

		default:
			break;

		}
	}
}



void TronClient::sendPlayer()
{
	//send player rotation to server
	sf::Packet packet;

	sf::Int32 playerID = id;

	sf::Int32 playerRot = m_players[id]->getDirection();	

	packet << NetMsg::PLAYER_INPUT << playerRot << playerID;

	m_socket->send(packet);
}



void TronClient::playerDied(int _id)
{
	//send player died to server
	sf::Packet packet;

	sf::Int32 player_id = _id;

	packet << NetMsg::PLAYER_DIED << player_id;

	m_socket->send(packet);
}



void TronClient::startGame()
{
	//start game on server
	sf::Packet packet;

	packet << NetMsg::RUN_GAME;

	m_socket->send(packet);
}



void TronClient::endGame()
{
	//end game on server
	sf::Packet packet;

	packet << NetMsg::END_GAME;

	m_socket->send(packet);
}



void TronClient::checkMenuState()
{
	//set sprites depending on state
	float scale_size = 1.3f;

	//lobby sprite positions 
	if (m_GS == GameState::LOBBY)
	{
		start_sprite.setPosition(650, 400);
		exit_sprite.setPosition(650, 500);

		player1_sprite.setPosition(100, 200);
		player2_sprite.setPosition(100, 300);
		player3_sprite.setPosition(100, 400);
		player4_sprite.setPosition(100, 500);

		for (int i = 0; i < 4; i++)
		{
			float x_pos = grid[416 + (i * 176)].getPosition().x;
			float y_pos = grid[416 + (i * 176)].getPosition().y;

			m_players[i]->setPosition(x_pos, y_pos);

			switch (i)
			{
			case 1:
			case 3:
				m_players[i]->setDirection(2);
				break;
			case 0:
			case 2:
				m_players[i]->setDirection(0);
				break;
			}
		}

		float x_pos = connection_dots[id].getPosition().x + 120;
		float y_pos = connection_dots[id].getPosition().y;

		you_sprite.setPosition(x_pos, y_pos);
	}

	//game over sprite positions
	else if (m_GS == GameState::GAME_OVER)
	{
		player1_sprite.setPosition(200, 200);
		player2_sprite.setPosition(200, 200);
		player3_sprite.setPosition(200, 200);
		player4_sprite.setPosition(200, 200);
	}

	else
	{
		start_sprite.setPosition(400, 200);
		exit_sprite.setPosition(400, 400);
	}

	//change size and colour of button highlighted 
	if (on_start)
	{
		start_sprite.setColor(sf::Color::Cyan);
		start_sprite.setScale(sf::Vector2f(scale_size, scale_size));

		exit_sprite.setColor(sf::Color::White);
		exit_sprite.setScale(sf::Vector2f(1, 1));
	}

	else
	{
		start_sprite.setColor(sf::Color::White);
		start_sprite.setScale(sf::Vector2f(1, 1));

		exit_sprite.setColor(sf::Color::Cyan);
		exit_sprite.setScale(sf::Vector2f(scale_size, scale_size));
	}
}



void TronClient::initSprites()
{
	//initialise sprites

	//reserve spots to gold sprites
	connection_dots.reserve(4);
	lobby_sprites.reserve(8);
	game_over_sprites.reserve(3);

	//load textures from file
	if (!c_texture.loadFromFile("../../Resources/Textures/Nyancat.png"))
	{
		// error
	}

	if (!start_texture.loadFromFile("../../Resources/Textures/START.PNG"))
	{
		// error
	}

	if (!exit_texture.loadFromFile("../../Resources/Textures/exit.PNG"))
	{
		// error
	}

	if (!lobby_texture.loadFromFile("../../Resources/Textures/LOBBY.PNG"))
	{
		// error
	}

	if (!game_over_texture.loadFromFile("../../Resources/Textures/GAME_OVER.PNG"))
	{
		// error
	}

	if (!player1_texture.loadFromFile("../../Resources/Textures/PLAYER_1.PNG"))
	{
		// error
	}

	if (!player2_texture.loadFromFile("../../Resources/Textures/PLAYER_2.PNG"))
	{
		// error
	}

	if (!player3_texture.loadFromFile("../../Resources/Textures/PLAYER_3.PNG"))
	{
		// error
	}

	if (!player4_texture.loadFromFile("../../Resources/Textures/PLAYER_4.PNG"))
	{
		// error
	}

	if (!win_texture.loadFromFile("../../Resources/Textures/WINS.PNG"))
	{
		// error
	}

	if (!you_texture.loadFromFile("../../Resources/Textures/YOU.PNG"))
	{
		// error
	}

	//set textures to sprites
	//set sprite positions 
	start_sprite.setTexture(start_texture);
	start_sprite.setPosition(400, 200);
	lobby_sprites.push_back(&start_sprite);

	exit_sprite.setTexture(exit_texture);
	exit_sprite.setPosition(400, 400);
	lobby_sprites.push_back(&exit_sprite);
	game_over_sprites.push_back(&exit_sprite);

	lobby_sprite.setTexture(lobby_texture);
	lobby_sprite.setPosition(400, 100);
	lobby_sprites.push_back(&lobby_sprite);

	player1_sprite.setTexture(player1_texture);
	player1_sprite.setPosition(100, 200);
	player1_sprite.setColor(sf::Color::Yellow);
	lobby_sprites.push_back(&player1_sprite);

	player2_sprite.setTexture(player2_texture);
	player2_sprite.setPosition(100, 300);
	player2_sprite.setColor(sf::Color(0, 140, 255));
	lobby_sprites.push_back(&player2_sprite);

	player3_sprite.setTexture(player3_texture);
	player3_sprite.setPosition(100, 400);
	player3_sprite.setColor(sf::Color::Magenta);
	lobby_sprites.push_back(&player3_sprite);

	player4_sprite.setTexture(player4_texture);
	player4_sprite.setPosition(100, 500);
	player4_sprite.setColor(sf::Color::Green);
	lobby_sprites.push_back(&player4_sprite);

	game_over_sprite.setTexture(game_over_texture);
	game_over_sprite.setPosition(400, 50);
	game_over_sprites.push_back(&game_over_sprite);

	win_sprite.setTexture(win_texture);
	win_sprite.setPosition(500, 200);
	win_sprite.setScale(0.7f, 0.7f);
	game_over_sprites.push_back(&win_sprite);	

	//set connection dots
	for (int i = 0; i < 4; i++)
	{
		connection_dot = sf::CircleShape(15);
		connection_dot.setPosition(410, ((i * 100) + 210));
		connection_dot.setFillColor(sf::Color(255, 165, 0));
		connection_dot.setOutlineThickness(5);
		connection_dot.setOutlineColor(sf::Color::White);
		connection_dots.push_back(connection_dot);
	}

	you_sprite.setTexture(you_texture);

	float x_pos = connection_dots[id].getPosition().x + 120;
	float y_pos = connection_dots[id].getPosition().y;

	you_sprite.setPosition(x_pos, y_pos);

	you_sprite.setScale(sf::Vector2f(0.4f, 0.4f));
	lobby_sprites.push_back(&you_sprite);
}



void TronClient::initGrid()
{
	//initialise grid tiles
	float x = 0;
	float y = 0;

	for (int i = 0; i < grid_height; i++)
	{
		for (int j = 0; j < grid_width; j++)
		{
			//new tiles and their positions
			tile = GridTile(t_texture);

			tile.setActive(false);

			tile.setPosition(x, y);

			tile.getTile().setTexture(&t_texture);
			x += 25.0f;

			grid.push_back(tile);
		}
		x = 0.0f;
		y += 25.0f;
	}
}



void TronClient::checkConnections(int _connections)
{
	//check who is connected
	int num = _connections;

	if (num > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			//set connection dot colours
			if (m_players[i]->getActive())
			{
				connection_dots[i].setFillColor(sf::Color::Green);
			}

			if (!m_players[i]->getActive())
			{
				connection_dots[i].setFillColor(sf::Color(255, 165, 0));
			}
		}
	}
}







