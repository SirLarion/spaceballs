#include "ball.h"

using namespace Ogre;

namespace sb
{
    // Getterit
    const Vector3 Ball::getVelocity() const { return velocity; }
    const Vector3 Ball::getPos() const { return node->getPosition(); }
    const float Ball::getMass() const { return mass; }
    const int Ball::getNumber() const { return ballNumber; }
    SceneNode* Ball::getNode() { return node; }

    void Ball::setVelocity(Vector3 newVelocity) { velocity = newVelocity; }
    void Ball::move(Vector3 velocity) { node->translate(velocity); }
    void Ball::attach(MovableObject* ent) { node->attachObject(ent); }
}
