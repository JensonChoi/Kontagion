#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
#include <stack>

class Actor;
class Socrates;
class Bacteria;
class Salmonella;
class Projectile;
class Goodie;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);

    virtual int init();

    virtual int move();

    virtual void cleanUp();

    double dist(double x1, double y1, double x2, double y2) const;

    bool overlap(Actor* first, Actor* second) const;
    //return whether the first actor overlap with the second actor

    double distToPlayer(Actor* actor);
    //return the distance between Socrates and the actor

    bool overlapWithPlayer(Actor* actor) const;
    //return whether the first actor overlap with player

    bool moveOverlap(Bacteria* bacteria, int step);
    //return whether the bacteria will be impeded by a dirt or go outside of the petri dish

    bool dealDamage(Projectile* projectile);
    //return whether projectile has successfully dealt damage

    bool eatFood(Bacteria* bacteria);
    //return whether bacteria has sucessfully eaten food

    bool findClosestFood(Salmonella* salmon);

    void applyEffect(Goodie* goodie);
    //applies the effects of the goodie 

    bool damagePlayer(int hp);
    //damages player

    int getPlayerDirection(Actor* actor);
    //in degrees

    bool addActor(Actor* actor);
    //return whether actor is successfully added

    void incBacteria();
    //increment numBacteria by 1

    bool decBacteria();
    //decrement numBacteria by 1

    bool decPits();
    //decrement numPit by 1

    virtual ~StudentWorld();

private:
    int m_numPits;
    int m_numBacteria;
    Socrates* m_player;
    std::list<Actor* > m_actors;
    std::stack<Actor* > m_actorsToAdd;

    void initXY(double& x, double& y) const;    //for init purposes

    void goodieXY(double& x, double& y, int angle) const;  //for generating goodies
};

//inline functions

inline void StudentWorld::incBacteria()
{
    m_numBacteria++;
}

#endif // STUDENTWORLD_H_
