#include "player.h"
#include "iostream"

using namespace Ogre;

namespace sb
{
	void Player::addBall(Ball* ball)
	{
		playerBalls[ball->getNumber()] = ball;
	}

	int Player::getBallsLeft() const
	{
		return playerBalls.size();
	}

	void Player::ballPotted(int b)
	{
		std::cout << "Ball: " + b << std::endl;
		playerBalls.erase(b);
	}

	std::string Player::getName() const
	{
		return playerName;
	}

	std::string Player::getType() const
	{
		return ballType;
	}

	int Player::getID() const
	{
		return ID;
	}
}