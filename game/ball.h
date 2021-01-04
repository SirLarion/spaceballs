#include "Ogre.h"

using namespace Ogre;

namespace sb
{
    // Biljardipalloja kuvastava luokka
    class Ball
    {
    public:
        Ball(SceneNode* _node, int _ballNumber) : 
            ballNumber(_ballNumber), node(_node), velocity(Vector3::ZERO), mass(1) {}

        const Vector3 getPos() const;
        const Vector3 getVelocity() const;
        SceneNode* getNode();
        const float getMass() const;
        const int getNumber() const;

        void move(Vector3 velocity);
        void setVelocity(Vector3 newVelocity);
        void attach(MovableObject* ent);

    private:
        int ballNumber; //Numero m‰‰ritt‰‰ mist‰ pelipallosta on kyse
        SceneNode* node;
        Vector3 velocity;
        float mass;
    };
}
