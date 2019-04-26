#pragma once

//SFML
#include <SFML/Graphics.hpp>

//base game object class
class GameObject
{
public:
	GameObject() = default;
	GameObject(sf::Texture& _texture);
	~GameObject() = default;

	//base tick (currently not used)
	virtual void tick(float _dt);

	//return position of sprite
	const sf::Vector2f getPosition();

	//return sprite
	const sf::Sprite getSprite();

	//return texture
	const sf::Texture getTexture();

	//set texture to sprite
	void setSprite(sf::Texture& _texture);

	//manually set position on server
	void setServerPosition(sf::Vector2f _server_position);

	//return server position 
	const sf::Vector2f getServerPosition();

	//sprite 
	sf::Sprite m_sprite;

protected:

	//texture for sprite
	sf::Texture m_texture;

	//position held by server if needed
	sf::Vector2f server_position = sf::Vector2f(0.0f, 0.0f);

};

