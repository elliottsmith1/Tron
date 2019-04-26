#include "GameObject.h"

#include <SFML/Graphics.hpp>
#include <future>
#include <iostream>
#include <string>
#include <SFML/Network.hpp>
#include <vector>

GameObject::GameObject(sf::Texture& _texture)
{
	m_texture = _texture;
	setSprite(m_texture);
}



void GameObject::tick(float _dt)
{
	//currently not used
}



const sf::Vector2f GameObject::getPosition()
{
	return m_sprite.getPosition();
}



const sf::Sprite GameObject::getSprite()
{
	return m_sprite;
}



const sf::Texture GameObject::getTexture()
{
	return m_texture;
}



void GameObject::setSprite(sf::Texture& _texture)
{
	m_sprite.setTexture(_texture);
}



void GameObject::setServerPosition(sf::Vector2f _server_position)
{
	server_position = _server_position;
}



const sf::Vector2f GameObject::getServerPosition()
{
	return server_position;
}

