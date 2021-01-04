#pragma once
#include "Ogre.h"
#include "ball.h"

using namespace Ogre;

namespace sb
{
	class Player
	{
	public:
		Player(std::string name, std::string type, int id) : playerName(name), ballType(type), ID(id) {}
		void addBall(Ball* ball);
		int getBallsLeft() const;
		void ballPotted(int b);
		std::string getName() const;
		std::string getType() const;
		int getID() const;

	private:
		std::map<int, Ball*> playerBalls;
		std::string playerName;
		std::string ballType;
		int ID;
	};
}
