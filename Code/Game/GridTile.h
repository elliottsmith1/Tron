#pragma once

//SFML
#include <SFML/Graphics.hpp>

class GridTile 
{
public:
	GridTile() = default;
	GridTile(sf::Texture& _texture);
	~GridTile() = default;

	//base tick
	void tick(float _dt);

	//move sprite
	void setPosition(float _x, float _y);

	//return position 
	const sf::Vector2f getPosition();

	//set and get active state
	void setActive(bool _active);
	const bool getActive();

	//return tile
	sf::RectangleShape getTile();

	//rotate tile
	void turnSquare(bool _turn);

private:

	//position coords
	sf::Vector2f position = sf::Vector2f(0.0f, 0.0f);

	//tile and texture to use
	sf::RectangleShape tile;
	sf::Texture        m_texture;

	//if active
	bool active = false;
};

