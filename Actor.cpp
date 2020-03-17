#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include "GameWorld.h"
#include <cmath>
#include <algorithm>
using namespace std;

//Actor class implementation

Actor::Actor(int imageID, double startX, double startY, Direction startDirection, int depth, StudentWorld* studWorld)
: GraphObject(imageID, startX, startY, startDirection, depth)
{
	m_alive = true;
	m_studWorld = studWorld;
}

bool Actor::damage(int hp)
{
	if (!alive() && hp <= 0)
		return false;
	setDead();
	return true;
}

//ActorWithHP class implementation

ActorWithHP::ActorWithHP(int imageID, double startX, double startY, Direction startDirection, StudentWorld* studWorld, int starthp)
: Actor(imageID, startX, startY, startDirection, 0, studWorld)
{
	m_health = starthp;
}

bool ActorWithHP::setHealth(int hp)
{
	if (hp <= 0)
		return false;
	m_health = hp;
	return true;
}

bool ActorWithHP::damage(int hp)
{
	if (!alive() || hp <= 0)
		return false;
	m_health -= hp;
	if (m_health <= 0)
	{
		setDead();	//dies if hp reaches 0
		myStudWorld()->playSound(soundWhenDie());
	}
	else
	{
		myStudWorld()->playSound(soundWhenHurt());
	}
	return true;
}

//Socrates class implementation

Socrates::Socrates(double startX, double startY, StudentWorld* studWorld)
: ActorWithHP(IID_PLAYER, startX, startY, 0, studWorld, 100)
{
	m_numSpray = 20;
	m_numFlame = 5;
}

bool Socrates::addFlame(int num)
{
	if (num > 0)
	{
		m_numFlame += num;
		return true;
	}
	return false;
}

void Socrates::doSomething()
{
	if (!alive())
		return;
	int keyPressed;
	if (myStudWorld()->getKey(keyPressed))
	{
		switch (keyPressed)
		{
		case KEY_PRESS_LEFT:
		case KEY_PRESS_RIGHT:
			moveAlongRim(keyPressed);
			break;
		case KEY_PRESS_SPACE:
			if (m_numSpray > 0)
			{
				double newX, newY;
				projectileXY(0, newX, newY);
				myStudWorld()->addActor(new Spray(newX, newY, getDirection(), myStudWorld()));
				myStudWorld()->playSound(SOUND_PLAYER_SPRAY);
				m_numSpray--;
			}
			break;
		case KEY_PRESS_ENTER:
			if (m_numFlame > 0)
			{
				double newX, newY;
				for (int i = 0; i < 16; i++)
				{
					projectileXY(22 * i, newX, newY);
					myStudWorld()->addActor(new Flame(newX, newY, getDirection() + 22 * i, myStudWorld()));
				}
				myStudWorld()->playSound(SOUND_PLAYER_FIRE);
				m_numFlame--;
			}
			break;
		}
	}
	else if (m_numSpray < 20)
		m_numSpray++;
}

bool Socrates::moveAlongRim(const int keyPressed)
{
	const double PI = 4 * atan(1);
	double currTheta = atan2(getY() - VIEW_RADIUS, getX() - VIEW_RADIUS) * 180 / PI;
	switch (keyPressed)
	{
	case KEY_PRESS_LEFT:
		currTheta += 5;	//move counter-clockwise
		break;
	case KEY_PRESS_RIGHT:
		currTheta -= 5;	//move clockwise
		break;
	default:
		return false;
	}
	moveTo(VIEW_RADIUS * cos(currTheta * PI / 180) + VIEW_RADIUS, VIEW_RADIUS * sin(currTheta * PI / 180) + VIEW_RADIUS);
	//move Socrates to new position
	setDirection(static_cast<int>(atan2(getY() - VIEW_RADIUS, getX() - VIEW_RADIUS) * 180 / PI) + 180);
	//Socrates faces towards the center of the petri dish
	return true;
}

void Socrates::projectileXY(Direction angle, double& x, double& y)
{
	//angle = 0 means same direction as Socrates
	const double PI = 4 * atan(1);
	double theta = PI * (static_cast<double>(getDirection()) + static_cast<double>(angle)) / 180;
	x = getX() + SPRITE_WIDTH * cos(theta);
	y = getY() + SPRITE_WIDTH * sin(theta);
}

//Bacteria class implementation

Bacteria::Bacteria(int imageID, double startX, double startY, StudentWorld* studWorld, int starthp, int toxicity)
: ActorWithHP(imageID, startX, startY, 90, studWorld, starthp)
{
	m_foodEaten = 0;
	m_movePlan = 0;
	m_toxicity = toxicity;
	myStudWorld()->playSound(SOUND_BACTERIUM_BORN);
	myStudWorld()->incBacteria();
}

bool Bacteria::resetFood()
{
	if (m_foodEaten >= 3)
	{
		m_foodEaten = 0;
		return true;
	}
	return false;
}

bool Bacteria::eat(Actor* food)
{
	if (food != nullptr)
	{
		m_foodEaten++;
		food->setDead();
		return true;
	}
	return false;
}

void Bacteria::setRandDirection()
{
	setDirection(randInt(0, 359));
	m_movePlan = 10;
}

bool Bacteria::decMove()
{
	if (m_movePlan >= 0)
	{
		m_movePlan--;
		return true;
	}
	return false;
}

bool Bacteria::setMove(int step)
{
	if (step >= 0)
	{
		m_movePlan = step;
		return true;
	}
	return false;
}

void Bacteria::doSomething()
{
	if (!alive())
		return;
	bool isAggro = aggressiveBehavior();
	if (myStudWorld()->overlapWithPlayer(this))
		myStudWorld()->damagePlayer(m_toxicity);
	else if (m_foodEaten >= 3)
		divide();
	else if (myStudWorld()->eatFood(this))
		return;
	if (isAggro)
		return;
	specificBehavior();
}

void Bacteria::divisionXY(double& x, double& y) const
{
	x = (getX() < VIEW_WIDTH / 2 ? getX() + SPRITE_WIDTH / 2 : getX() - SPRITE_WIDTH / 2);
	y = (getY() < VIEW_HEIGHT / 2 ? getY() + SPRITE_WIDTH / 2 : getY() - SPRITE_WIDTH / 2);
}

bool Bacteria::turnIntoFood() const
{
	int probFood = randInt(0, 1);	//50% chance to turn into food
	if (probFood == 1)
	{
		myStudWorld()->addActor(new Food(getX(), getY(), myStudWorld()));
		return true;
	}
	return false;
}

bool Bacteria::damage(int hp)
{
	if (ActorWithHP::damage(hp))
	{
		if (!alive())
		{
			myStudWorld()->increaseScore(100);
			turnIntoFood();
		}
		return true;
	}
	return false;
}

Bacteria::~Bacteria()
{
	myStudWorld()->decBacteria();
}

//Salmonella class implementation

Salmonella::Salmonella(double startX, double startY, StudentWorld* studWorld, int starthp, int toxicity)
: Bacteria(IID_SALMONELLA, startX, startY, studWorld, starthp, toxicity)
{
}

void Salmonella::divide()
{
	if (resetFood())	//checks if foodEaten >= 3
	{
		double newX, newY;
		divisionXY(newX, newY);
		myStudWorld()->addActor(new Salmonella(newX, newY, myStudWorld()));
	}
}

bool Salmonella::move()
{
	if (checkMove() > 0)
	{
		decMove();
		if (!myStudWorld()->moveOverlap(this, 3))	//moveOverlap also checks whether new xy are within petri dish
			moveForward(3);
		else
			setRandDirection();
		return true;
	}
	return false;
}

void Salmonella::specificBehavior()
{
	if (checkMove() > 0)
		move();
	else if (!myStudWorld()->findClosestFood(this))	
		setRandDirection();
}

bool Salmonella::moveTowardsFood(Actor* food)
{
	const double PI = 4 * atan(1);
	if (food != nullptr)
	{
		setDirection(180 * atan2(food->getY() - getY(), food->getX() - getX()) / PI);
		//Salmonella faces the food
		setMove(1);
		move();
		return true;
	}
	return false;
}

//AggressiveSalmonella class implementation

AggressiveSalmonella::AggressiveSalmonella(double startX, double startY, StudentWorld* studWorld)
: Salmonella(startX, startY, studWorld, 10, 2)
{
}

bool AggressiveSalmonella::aggressiveBehavior()
{
	if (myStudWorld()->distToPlayer(this) <= 72)
	{
		setDirection(myStudWorld()->getPlayerDirection(this));
		if (!myStudWorld()->moveOverlap(this, 3))	//moveOverlap also checks whether new xy are within petri dish
			moveForward(3);
		return true;
	}
	return false;
}

void AggressiveSalmonella::divide()
{
	if (resetFood())	//checks if foodEaten >= 3
	{
		double newX, newY;
		divisionXY(newX, newY);
		myStudWorld()->addActor(new AggressiveSalmonella(newX, newY, myStudWorld()));
	}
}

//Ecoli class implementation

Ecoli::Ecoli(double startX, double startY, StudentWorld* studWorld)
: Bacteria(IID_ECOLI, startX, startY, studWorld, 5, 4)
{
}

void Ecoli::divide()
{
	if (resetFood())	//checks if foodEaten >= 3
	{
		double newX, newY;
		divisionXY(newX, newY);
		myStudWorld()->addActor(new Ecoli(newX, newY, myStudWorld()));
	}
}

bool Ecoli::move()
{
	for (int i = 0; i < 10; i++)
	{
		setDirection(myStudWorld()->getPlayerDirection(this) + 10 * i);
		if (!myStudWorld()->moveOverlap(this, 2))
		{
			moveForward(2);
			return true;
		}
	}
	return false;
}

void Ecoli::specificBehavior()
{
	if (myStudWorld()->distToPlayer(this) <= VIEW_HEIGHT)
		move();
}

//Dirt class implementation

Dirt::Dirt(double startX, double startY, StudentWorld* studWorld)
: Actor(IID_DIRT, startX, startY, 0, 1, studWorld)
{
}

//Food class implementation

Food::Food(double startX, double startY, StudentWorld* studWorld)
: Actor(IID_FOOD, startX, startY, 90, 1, studWorld)
{
}

//Projectile class implementation

Projectile::Projectile(int imageID, double startX, double startY, Direction startDirection, StudentWorld* studWorld, int range, int damage)
	: Actor(imageID, startX, startY, startDirection, 1, studWorld)
{
	m_distTraveled = 0;
	m_range = range;
	m_damage = damage;
}

void Projectile::doSomething()
{
	if (!alive())
		return;
	if (myStudWorld()->dealDamage(this))
		return;
	moveForward(SPRITE_RADIUS * 2);
	m_distTraveled += SPRITE_RADIUS * 2;
	if (rangeReached())
		setDead();
}

bool Projectile::damageTarget(Actor* target)
{
	if (target != nullptr)
	{
		target->damage(m_damage);
		setDead();
		return true;
	}
	return false;
}

//Flame class implementation

Flame::Flame(double startX, double startY, Direction startDirection, StudentWorld* studWorld)
: Projectile(IID_FLAME, startX, startY, startDirection, studWorld, 32, 5)
{
}

//Spray class implementation

Spray::Spray(double startX, double startY, Direction startDirection, StudentWorld* studWorld)
: Projectile(IID_SPRAY, startX, startY, startDirection, studWorld, 112, 2)
{
}

//Goodie class implementation

Goodie::Goodie(int imageID, double startX, double startY, StudentWorld* studWorld, int lifetime)
: Actor(imageID, startX, startY, 0, 1, studWorld)
{
	m_age = 0;
	m_lifetime = lifetime;
}

void Goodie::doSomething()
{
	if (!alive())
		return;
	if (myStudWorld()->overlapWithPlayer(this))	//check if it overlaps with Socrates
	{
		myStudWorld()->applyEffect(this);
		setDead();
		return;
	}
	m_age++;
	if (LifetimeReached())
		setDead();
}

//RestoreHealthGoodie class implementation

RestoreHealthGoodie::RestoreHealthGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime)
: Goodie(IID_RESTORE_HEALTH_GOODIE, startX, startY, studWorld, lifetime)
{
}

void RestoreHealthGoodie::pickUp(Socrates* player)
{
	if (player != nullptr)
	{
		myStudWorld()->increaseScore(250);
		myStudWorld()->playSound(SOUND_GOT_GOODIE);
		player->completeHeal();
	}
}

//FlameThrowerGoodie class implementation

FlameThrowerGoodie::FlameThrowerGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime)
: Goodie(IID_FLAME_THROWER_GOODIE, startX, startY, studWorld, lifetime)
{
}

void FlameThrowerGoodie::pickUp(Socrates* player)
{
	if (player != nullptr)
	{
		myStudWorld()->increaseScore(300);
		myStudWorld()->playSound(SOUND_GOT_GOODIE);
		player->addFlame(5);
	}
}

//ExtraLifeGoodie class implementation

ExtraLifeGoodie::ExtraLifeGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime)
: Goodie(IID_EXTRA_LIFE_GOODIE, startX, startY, studWorld, lifetime)
{
}

void ExtraLifeGoodie::pickUp(Socrates* player)
{
	if (player != nullptr)
	{
		myStudWorld()->increaseScore(500);
		myStudWorld()->playSound(SOUND_GOT_GOODIE);
		myStudWorld()->incLives();
	}
}

//Fungus class implementation

Fungus::Fungus(double startX, double startY, StudentWorld* studWorld, int lifetime)
: Goodie(IID_FUNGUS, startX, startY, studWorld, lifetime)
{
}

void Fungus::pickUp(Socrates* player)
{
	if (player != nullptr)
	{
		myStudWorld()->increaseScore(-50);
		player->damage(20);
	}
}

//Pit class implementation

Pit::Pit(double startX, double startY, StudentWorld* studWorld)
: Actor(IID_PIT, startX, startY, 0, 1, studWorld)
{
	numSalmon = 5;
	numAggroSalmon = 3;
	numEcoli = 2;
}

void Pit::doSomething()
{
	if (empty())
	{
		setDead();
		return;
	}
	int prob = randInt(0, 49);	//1 in 50 chance to release a bacterium
	if (prob == 0)
	{
		bool emitted = false;	//has not emitted a bacterium
		while (!emitted)
		{
			int choice = randInt(0, 2);
			switch (choice)
			{
			case 0:
				if (numSalmon > 0)
				{
					emitted = true;
					myStudWorld()->addActor(new Salmonella(getX(), getY(), myStudWorld()));
					numSalmon--;
				}
				break;
			case 1:
				if (numAggroSalmon > 0)
				{
					emitted = true;
					myStudWorld()->addActor(new AggressiveSalmonella(getX(), getY(), myStudWorld()));
					numAggroSalmon--;
				}
				break;
			case 2:
				if (numEcoli > 0)
				{
					emitted = true;
					myStudWorld()->addActor(new Ecoli(getX(), getY(), myStudWorld()));
					numEcoli--;
				}
				break;
			}
		}
	}
}

Pit::~Pit()
{
	myStudWorld()->decPits();
}