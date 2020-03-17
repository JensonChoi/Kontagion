#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

class Actor: public GraphObject
{
public:
	Actor(int imageID, double startX, double startY, Direction startDirection, int depth, StudentWorld* studWorld);

	bool alive() const;

	void setDead();
	
	virtual bool damageable() const;
	//return whether the actor can be damaged by projectiles

	virtual bool damage(int hp);
	//return whether the actor has been successfully damaged

	virtual bool canOverlap() const;
	//return whether the actor can overlap with other actors when being created

	virtual bool canBlock() const;
	//return whether the actor can block the movement of other actors

	virtual bool edible() const;
	//return whether the actor can be eaten by bacteria

	virtual void doSomething() = 0;

	Actor* getMe();

	virtual ~Actor()
	{}
protected:
	StudentWorld* myStudWorld() const;

private:
	bool m_alive;
	StudentWorld* m_studWorld;
};

class ActorWithHP : public Actor
{
public:
	ActorWithHP(int imageID, double startX, double startY, Direction startDirection, StudentWorld* studWorld, int starthp);

	int health() const;
	//return the number of HP

	virtual bool damage(int hp);
	//return whether the actor has been successfully damaged

	virtual ~ActorWithHP()
	{}
protected:
	bool setHealth(int hp);
	//return whether health is successfully reset

private:
	int m_health;

	virtual int soundWhenHurt() const = 0;

	virtual int soundWhenDie() const = 0;
};

class Socrates : public ActorWithHP
{
public:
	Socrates(double startX, double startY, StudentWorld* studWorld);

	int numSpray() const;

	int numFlame() const;

	void completeHeal();
	//restores Socrates hp fully

	bool addFlame(int num);
	//return whether flame thower charges are successfully added

	virtual bool damageable() const;
	//returns false since it's not damagable by projectiles

	virtual void doSomething();

	virtual ~Socrates()
	{}
private:
	int m_numSpray;
	int m_numFlame;

	bool moveAlongRim(const int keyPressed);

	void projectileXY(Direction angle, double& x, double& y);
	//for generating projectiles

	virtual int soundWhenHurt() const;

	virtual int soundWhenDie() const;
};

class Bacteria : public ActorWithHP
{
public:
	Bacteria(int imageID, double startX, double startY, StudentWorld* studWorld, int starthp, int toxicity);

	bool decMove();
	//decrement movePlan by 1

	bool eat(Actor* food);

	virtual bool damage(int hp);

	virtual void doSomething();

	virtual ~Bacteria();
protected:
	int checkMove() const;

	bool setMove(int step);

	virtual bool move() = 0;
	//make a move

	bool resetFood();
	//return whether foodEaten has successfully been reset

	void divisionXY(double& x, double& y) const;	//for cell division purposes
	
	bool turnIntoFood() const;

	void setRandDirection();
private:
	int m_foodEaten;
	int m_movePlan;
	int m_toxicity;

	virtual bool aggressiveBehavior();

	virtual void divide() = 0;
	//creates another bacteria of the same kind through cell division

	virtual void specificBehavior() = 0;
	//do behavior specific to each kind of bacteria
};

class Salmonella : public Bacteria
{
public:
	Salmonella(double startX, double startY, StudentWorld* studWorld, int starthp = 4, int toxicity = 1);

	bool moveTowardsFood(Actor* food);

	virtual ~Salmonella()
	{}
protected:
	virtual bool move();

private:
	virtual void divide();

	virtual void specificBehavior();

	virtual int soundWhenHurt() const;

	virtual int soundWhenDie() const;
};

class AggressiveSalmonella : public Salmonella 
{
public:
	AggressiveSalmonella(double startX, double startY, StudentWorld* studWorld);

	virtual ~AggressiveSalmonella()
	{}
private:
	virtual bool aggressiveBehavior();

	virtual void divide();
};

class Ecoli : public Bacteria
{
public:
	Ecoli(double startX, double startY, StudentWorld* studWorld);

	virtual ~Ecoli()
	{}
protected:
	virtual bool move();

private:
	virtual void divide();

	virtual void specificBehavior();

	virtual int soundWhenHurt() const;

	virtual int soundWhenDie() const;
};

class Dirt : public Actor
{
public:
	Dirt(double startX, double startY, StudentWorld* studWorld);

	virtual bool canOverlap() const;

	virtual bool canBlock() const;

	virtual void doSomething()
	{}

	virtual ~Dirt()
	{}
};

class Food : public Actor
{
public:
	Food(double startX, double startY, StudentWorld* studWorld);

	virtual bool edible() const;

	virtual bool damageable() const;

	virtual void doSomething()
	{}

	virtual ~Food()
	{}
};

class Projectile : public Actor
{
public:
	Projectile(int imageID, double startX, double startY, Direction startDirection, StudentWorld* studWorld, int range, int damage);

	virtual bool damageable() const;
	//returns false

	virtual void doSomething();

	bool damageTarget(Actor* target);

	virtual ~Projectile()
	{}
private:
	int m_distTraveled;
	int m_range;
	int m_damage;

	bool rangeReached() const;
	//return true if distance traveled has reached range
};

class Flame : public Projectile
{
public:
	Flame(double startX, double startY, Direction startDirection, StudentWorld* studWorld);

	virtual ~Flame()
	{}
};

class Spray : public Projectile
{
public:
	Spray(double startX, double startY, Direction startDirection, StudentWorld* studWorld);

	virtual ~Spray()
	{}
};

class Goodie : public Actor
{
public:
	Goodie(int imageID, double startX, double startY, StudentWorld* studWorld, int lifetime);

	virtual void doSomething();

	virtual void pickUp(Socrates* player) = 0;
	//Goodie gets picked up by the player

	virtual ~Goodie()
	{}
private:
	int m_age;
	int m_lifetime;

	bool LifetimeReached() const;
	//return true if age has reached lifetime
};

class RestoreHealthGoodie : public Goodie
{
public:
	RestoreHealthGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime);

	virtual void pickUp(Socrates* player);

	virtual ~RestoreHealthGoodie()
	{}
};

class FlameThrowerGoodie : public Goodie
{
public:
	FlameThrowerGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime);

	virtual void pickUp(Socrates* player);

	virtual ~FlameThrowerGoodie()
	{}
};

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(double startX, double startY, StudentWorld* studWorld, int lifetime);

	virtual void pickUp(Socrates* player);

	virtual ~ExtraLifeGoodie()
	{}
};

class Fungus : public Goodie
{
public:
	Fungus(double startX, double startY, StudentWorld* studWorld, int lifetime);

	virtual void pickUp(Socrates* player);

	virtual ~Fungus()
	{}
};

class Pit : public Actor
{
public:
	Pit(double startX, double startY, StudentWorld* studWorld);

	virtual bool damageable() const;
	//return false

	virtual void doSomething();

	virtual ~Pit();
private:
	int numSalmon;
	int numAggroSalmon;
	int numEcoli;

	bool empty() const;
};

//inline functions

inline bool Actor::alive() const
{
	return m_alive;
}

inline void Actor::setDead()
{
	m_alive = false;
}

inline bool Actor::damageable() const
{
	return true;
}

inline bool Actor::canOverlap() const
{
	return false;
}

inline bool Actor::canBlock() const
{
	return false;
}

inline bool Actor::edible() const
{
	return false;
}

inline StudentWorld* Actor::myStudWorld() const
{
	return m_studWorld;
}

inline Actor* Actor::getMe()
{
	return this;
}

inline int ActorWithHP::health() const
{
	return m_health;
}

inline int Socrates::numSpray() const
{
	return m_numSpray;
}

inline int Socrates::numFlame() const
{
	return m_numFlame;
}

inline void Socrates::completeHeal()
{
	setHealth(100);
}

inline bool Socrates::damageable() const
{
	return false;
}

inline int Socrates::soundWhenHurt() const
{
	return SOUND_PLAYER_HURT;
}

inline int Socrates::soundWhenDie() const
{
	return SOUND_PLAYER_DIE;
}

inline int Bacteria::checkMove() const
{
	return m_movePlan;
}

inline bool Bacteria::aggressiveBehavior()
{
	return false;
}

inline int Salmonella::soundWhenHurt() const
{
	return SOUND_SALMONELLA_HURT;
}

inline int Salmonella::soundWhenDie() const
{
	return SOUND_SALMONELLA_DIE;
}

inline int Ecoli::soundWhenHurt() const
{
	return SOUND_ECOLI_HURT;
}

inline int Ecoli::soundWhenDie() const
{
	return SOUND_ECOLI_DIE;
}

inline bool Dirt::canOverlap() const
{
	return true;
}

inline bool Dirt::canBlock() const
{
	return true;
}

inline bool Food::edible() const
{
	return true;
}

inline bool Food::damageable() const
{
	return false;
}

inline bool Projectile::rangeReached() const
{
	return m_distTraveled >= m_range;
}

inline bool Projectile::damageable() const
{
	return false;
}


inline bool Goodie::LifetimeReached() const
{
	return m_age >= m_lifetime;
}

inline bool Pit::damageable() const
{
	return false;
}

inline bool Pit::empty() const
{
	return (numSalmon == 0 && numAggroSalmon == 0 && numEcoli == 0);
}
#endif // ACTOR_H_