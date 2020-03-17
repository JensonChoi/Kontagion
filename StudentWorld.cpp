#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    m_numPits = 0;
    m_numBacteria = 0;
}

int StudentWorld::init()
{
    //add socrates
    m_player = new Socrates(0, VIEW_HEIGHT/2, this);

    //add pit objects
    double startX, startY;
    m_numPits = getLevel();
    for (int i = 0; i < m_numPits; i++)
    {
        bool valid;
        do {
            valid = true;
            initXY(startX, startY);
            for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
            {
                if ((dist((*it)->getX(), (*it)->getY(), startX, startY) <= 2.0 * SPRITE_RADIUS))
                {
                    valid = false;
                    break;
                }
            }
        } while (!valid);
        m_actors.push_back(new Pit(startX, startY, this));
    }

    //add food objects
    int numFood = min(5 * getLevel(), 25);
    for (int i = 0; i < numFood; i++)
    {
        bool valid;
        do {
            valid = true;
            initXY(startX, startY);
            for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
            {
                if ((dist((*it)->getX(), (*it)->getY(), startX, startY) <= 2.0 * SPRITE_RADIUS))
                {
                    valid = false;
                    break;
                }
            }
        } while (!valid);
        m_actors.push_back(new Food(startX, startY, this));
    }

    //add dirt objects
    int numDirt = max(180 - 20 * getLevel(), 20);
    for (int i = 0; i < numDirt; i++)
    {
        bool valid;
        do {
            valid = true;
            initXY(startX, startY);
            for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
            {
                if (!(*it)->canOverlap() && (dist((*it)->getX(), (*it)->getY(), startX, startY) <= 2.0 * SPRITE_RADIUS))
                {
                    valid = false;
                    break;
                }
            }
        } while (!valid);        
        m_actors.push_back(new Dirt(startX, startY, this));
        
    }   
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    m_player->doSomething();
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        (*it)->doSomething();

        //check if Socrates is still alive
        if (!m_player->alive())
        {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }

        //check if level is completed
        if (m_numPits == 0 && m_numBacteria == 0)
        {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }

    //delete actors that are no longer alive at the end of the round
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end();)
    {
        if (!(*it)->alive())
        {
            delete (*it);
            it = m_actors.erase(it);
        }
        else
            it++;
    }

    //add fungus
    int chanceFungus = max(510 - getLevel() * 10, 200);
    int probFungus = randInt(0, chanceFungus - 1);
    if (probFungus == 0)
    {
        int angle = randInt(0, 359);
        double startX, startY;
        goodieXY(startX, startY, angle);
        int lifetime = max(randInt(0, 300 - 10 * getLevel() - 1), 50);
        m_actors.push_back(new Fungus(startX, startY, this, lifetime));
    }

    //add goodie
    int chanceGoodie = max(510 - getLevel() * 10, 250);
    int probGoodie = randInt(0, chanceGoodie - 1);
    if (probGoodie == 0)
    {
        int choice = randInt(0, 9);
        int angle = randInt(0, 359);
        double startX, startY;
        goodieXY(startX, startY, angle);
        int lifetime = max(randInt(0, 300 - 10 * getLevel() - 1), 50);
        switch (choice)
        {
        case 0:
            m_actors.push_back(new ExtraLifeGoodie(startX, startY, this, lifetime));
            break;
        case 1:
        case 2:
        case 3:
            m_actors.push_back(new FlameThrowerGoodie(startX, startY, this, lifetime));
            break;
        default:
            m_actors.push_back(new RestoreHealthGoodie(startX, startY, this, lifetime));
        }
    }

    //add actors that are on the stack
    while (!m_actorsToAdd.empty())
    {
        m_actors.push_back(m_actorsToAdd.top());
        m_actorsToAdd.pop();
    }

    //update GameStatText
    ostringstream gameText;
    gameText << "Score: ";
    gameText.fill('0');
    if (getScore() < 0)
        gameText << '-' << setw(5) << abs(getScore());
    else
        gameText << setw(6) << getScore();
    gameText.fill(' ');
    gameText << "  Level: ";
    gameText << setw(2) << getLevel();
    gameText << "  Lives: ";
    gameText << setw(1) << getLives();
    gameText << "  Health: ";
    gameText << setw(3) << m_player->health();
    gameText << "  Sprays: ";
    gameText << setw(2) << m_player->numSpray();
    gameText << "  Flames: ";
    gameText << setw(2) << m_player->numFlame();
    setGameStatText(gameText.str());
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete m_player;
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end();)
    {
        delete *it;
        it = m_actors.erase(it);
    }
}

double StudentWorld::dist(double x1, double y1, double x2, double y2) const
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool StudentWorld::overlap(Actor* first, Actor* second) const
{
    if (first != second)
    {
        return dist(first->getX(), first->getY(), second->getX(), second->getY()) <= SPRITE_RADIUS * 2.0;
    }
    return false;   //the same actor can't overlap with itself
}

bool StudentWorld::overlapWithPlayer(Actor* actor) const
{
    return overlap(actor, m_player);
}

bool StudentWorld::moveOverlap(Bacteria* bacteria, int step)
{
    const double PI = 4 * atan(1);
    //compute the bacteria's xy coordinates after moving
    double newX = bacteria->getX() + step * cos(PI * bacteria->getDirection() / 180);
    double newY = bacteria->getY() + step * sin(PI * bacteria->getDirection() / 180);

    //check if the new xy is outside of the petri dish
    if (dist(newX, newY, VIEW_WIDTH / 2, VIEW_HEIGHT / 2) >= VIEW_RADIUS)
        return true;

    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {   
        //check if *it is a dirt
        if((*it)->alive() && (*it)->canBlock() && (dist(newX, newY, (*it)->getX(), (*it)->getY()) <= SPRITE_RADIUS))
            return true;
    }
    return false;
}

bool StudentWorld::dealDamage(Projectile* projectile)
{   
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->alive() && (*it)->damageable() && overlap(projectile, (*it)->getMe()))
        {
            return projectile->damageTarget((*it)->getMe());
        }
    }
    return false;
}

double StudentWorld::distToPlayer(Actor* actor)
{
    return dist(m_player->getX(), m_player->getY(), actor->getX(), actor->getY());
}

bool StudentWorld::eatFood(Bacteria* bacteria)
{   
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->alive() && (*it)->edible() && overlap(bacteria, (*it)->getMe()))
        {
            return bacteria->eat((*it)->getMe());
        }
    }
    return false;
}

bool StudentWorld::damagePlayer(int hp)
{
    if (hp > 0)
    {
        return m_player->damage(hp);
    }
    return false;
}

int StudentWorld::getPlayerDirection(Actor* actor)
{
    const double PI = 4 * atan(1);
    double theta = atan2(m_player->getY() - actor->getY(), m_player->getX() - actor->getX());
    return static_cast<int>(180 * theta / PI);
}

bool StudentWorld::findClosestFood(Salmonella* salmon)
{
    const double PI = 4 * atan(1);
    Actor* food = nullptr;
    double minDist = VIEW_RADIUS;   //any further the Salmonella can't detect the food
    for (list<Actor* >::iterator it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->alive() && (*it)->edible() && dist(salmon->getX(), salmon->getY(), (*it)->getX(), (*it)->getY()) <= minDist)
        {
            food = (*it)->getMe();
        }
    }
    return salmon->moveTowardsFood(food);
}

void StudentWorld::applyEffect(Goodie* goodie)
{
    if(goodie != nullptr)
        goodie->pickUp(m_player);
}

bool StudentWorld::addActor(Actor* actor)
{
    if (actor != nullptr)
    {
        m_actorsToAdd.push(actor); 
        return true;
    }
    return false;
}

bool StudentWorld::decBacteria()
{
    if (m_numBacteria > 0)
    {
        m_numBacteria--;
        return true;
    }
    return false;
}

bool StudentWorld::decPits()
{
    if (m_numPits > 0)
    {
        m_numPits--;
        return true;
    }
    return false;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

void StudentWorld::initXY(double& x, double& y) const
{
    bool valid = false;
    double tempX, tempY;
    while (!valid)
    {
        tempX = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
        tempY = randInt(VIEW_RADIUS - 120, VIEW_RADIUS + 120);
        if (dist(tempX, tempY, VIEW_RADIUS, VIEW_RADIUS) <= 120)    //must be no more than 120 pixels away from the center
            valid = true;
    }
    x = tempX;
    y = tempY;
}

void StudentWorld::goodieXY(double& x, double& y, int angle) const
{
    x = VIEW_RADIUS + VIEW_RADIUS * cos(angle);
    y = VIEW_RADIUS + VIEW_RADIUS * sin(angle);
}