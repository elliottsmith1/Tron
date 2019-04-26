#include "Player.h"
#include "GameObject.h"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <future>
#include <iostream>
#include <string>
#include <vector>

Player::Player(sf::Texture& _texture, int _id) : GameObject (_texture)
{
	id = _id;
	m_sprite.setOrigin((m_texture.getSize().x * 0.5), (m_texture.getSize().y * 0.5));

	createBike();
	setSpawn();
}



void Player::createBike()
{	
	//set scale and colour depending on id
	m_sprite.setScale(sf::Vector2f(0.05, 0.05));

	switch (id)
	{
	case 0:
		m_sprite.setColor(sf::Color(250, 250, 250));
		break;
	case 1:
		m_sprite.setColor(sf::Color(0, 250, 250));
		break;
	case 2:
		m_sprite.setColor(sf::Color(250, 0, 250));
		break;
	case 3:
		m_sprite.setColor(sf::Color(0, 250, 0));
		break;
	default:
		m_sprite.setColor(sf::Color(250, 250, 250));
		break;
	}	
}



void Player::moveBike(float _x, float _y)
{
	m_sprite.move(_x, _y);
}



void Player::setDirection(sf::Int32 _direction)
{
	//change direction
	move_direction = _direction;

	switch (move_direction)
	{
	case 0:
		m_sprite.setRotation(0);
		m_sprite.setScale(sf::Vector2f(0.05f, 0.05f));
		break;
	case 1:
		m_sprite.setRotation(90);
		m_sprite.setScale(sf::Vector2f(0.05f, 0.05f));
		break;
	case 2:
		m_sprite.setRotation(0);
		m_sprite.setScale(sf::Vector2f(-0.05f, 0.05f));
		break;
	case 3:
		m_sprite.setRotation(90);
		m_sprite.setScale(sf::Vector2f(-0.05f, -0.05f));
	default:
		break;
	}
}



const sf::Int32 Player::getDirection()
{
	return move_direction;
}



const sf::Int32 Player::getID()
{
	return id;
}



void Player::setPosition(float _x, float _y)
{
	m_sprite.setPosition(_x, _y);
}



void Player::setID(sf::Int32 _id)
{
	id = _id;
}



void Player::setSpawn()
{
	//set spawn locations
	switch (id)
	{
	case 0:
		grid_position = 416;
		spawn_position = grid_position;
		break;
	case 1:
		grid_position = 592;
		spawn_position = grid_position;
		setDirection(2);
		break;
	case 2:
		grid_position = 768;
		spawn_position = grid_position;
		break;
	case 3:
		grid_position = 944;
		spawn_position = grid_position;
		setDirection(2);
		break;
	default:
		grid_position = 416;
		spawn_position = grid_position;
		break;
	}
}



void Player::setActive(bool _active)
{
	active = _active;
}



const bool Player::getActive()
{
	return active;
}



const sf::Int32 Player::getGridPos()
{
	return grid_position;
}



void Player::setGridPos(sf::Int32 _grid_pos)
{
	grid_position = _grid_pos;
}



const bool Player::getAlive()
{
	return alive;
}



void Player::setAlive(bool _alive)
{
	alive = _alive;
}



void Player::deathColour()
{
	m_sprite.setColor(sf::Color::Red);
}



void Player::boundingBox()
{
	//if outside bounds then kill
	if ((m_sprite.getPosition().y < 20) || (m_sprite.getPosition().y > 760))
	{
		alive = false;
	}

	else if ((m_sprite.getPosition().x < 20) || (m_sprite.getPosition().x > 1010))
	{
		alive = false;
	}
}



const sf::Int32 Player::getSpawn()
{
	return spawn_position;
}

