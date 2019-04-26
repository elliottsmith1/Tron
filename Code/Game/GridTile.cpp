#include "GridTile.h"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

GridTile::GridTile(sf::Texture& _texture)
{
	tile = sf::RectangleShape(sf::Vector2f(25, 25));
	tile.setTexture(&_texture);
	tile.setOrigin((tile.getSize().x * 0.5f), (tile.getSize().y * 0.5f));
}



void GridTile::tick(float _dt)
{

}



void GridTile::setPosition(float _x, float _y)
{
	position = sf::Vector2f(sf::Vector2f(_x, _y));
	tile.setPosition(sf::Vector2f(_x, _y));
}



const sf::Vector2f GridTile::getPosition()
{
	return position;
}



void GridTile::setActive(bool _active)
{
	active = _active;
}



const bool GridTile::getActive()
{
	return active;
}



sf::RectangleShape GridTile::getTile()
{
	return tile;
}



void GridTile::turnSquare(bool _turn)
{
	bool turn = _turn;

	if (turn)
	{
		tile.setRotation(90.0f);
	}

	else
	{
		tile.setRotation(0.0f);
	}
}

