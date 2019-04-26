#pragma once

//inherits from game object
#include "GameObject.h"

class Player : public GameObject
{
public:
	Player() = default;
	Player(sf::Texture& _texture, int _id);
	~Player() = default;

	//set scale and colour
	void createBike();

	//move sprite
	void moveBike(float _x, float _y);

	//change direction
	void setDirection(sf::Int32 _direction);

	//return direction
	const sf::Int32 getDirection();

	//return ID
	const sf::Int32 getID();

	//set sprite position
	void setPosition(float _x, float _y);

	//set ID
	void setID(sf::Int32 _id);

	//set spawn point
	void setSpawn();

	//set active state
	void setActive(bool _active);

	//get active state
	const bool getActive();

	//get position on grid
	const sf::Int32 getGridPos();

	//set position on grid
	void setGridPos(sf::Int32 _grid_pos);

	//get alive state
	const bool getAlive();

	//set alive state
	void setAlive(bool _alive);

	//set colour to red when dead
	void deathColour();

	//check bounding box
	void boundingBox();

	//get spawn point
	const sf::Int32 getSpawn();

private:

	//which way moving (0, 1, 2 or 3)
	sf::Int32 move_direction = 0;

	//player id
	sf::Int32 id = 0;

	//position on grid
	sf::Int32 grid_position = 0;

	//spawn position
	sf::Int32 spawn_position = 0;

	//active state
	bool active = false;

	//alive state
	bool alive = true;
};

