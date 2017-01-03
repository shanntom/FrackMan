#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;  //pointer to studentWorld in here

//GRAPHOBJECT FUNCTIONS
//GraphObject(int imageID, int startX, int startY, Direction dir = right,
//           double size = 1.0, unsigned int depth = 0)
//void setVisible(bool shouldIDisplay)   weather or not to display obj
//int getX() const
//int getY() const
//void moveTo(int x, int y)
//Direction getDirection() const      up, dwon left , right
//void setDirection(Direction d)

class Actor: public GraphObject
{
public:
	Actor(int imageID, int sx, int sy, Direction d, double size,
		unsigned int depth, StudentWorld* wldptr); //set visible to true
	virtual ~Actor();
	//accessors
	virtual bool isAlive() const { return m_isAlive; }
	virtual StudentWorld* worldPtr() { return m_world; }
	virtual bool isBoulder() const = 0;
	virtual bool isProtest() const = 0;
	virtual bool canMove(Direction d);
	//mutators
	virtual void doSomething() = 0;  
	virtual void setDead() { m_isAlive = false; }
	virtual void annoy(bool byBould) = 0;
	virtual void beBribed() = 0;

private:
	bool m_isAlive;
	StudentWorld* m_world;

};

///////////////////////////////////////////////////////////////
class Squirt : public Actor
{
public:
	Squirt(int x, int y, StudentWorld* wldptr, int d);
	virtual ~Squirt() {}

	virtual bool isBoulder() const { return false; }
	virtual bool isProtest() const { return false; }

	virtual void doSomething();
	virtual void annoy(bool byBould) {}
	virtual void beBribed() {}

public:
	int ticksLeft;
};

////////////////////////////////////////////////////////////////
class Dirt : public Actor
{
public:
	Dirt(int x, int y, StudentWorld* wldptr);
	virtual ~Dirt();
	virtual void doSomething() {}  //does nothing
	virtual void annoy(bool byBould) {}  //does nothing
	//Accessors
	virtual bool isBoulder() const { return false; }
	virtual bool isProtest() const { return false; }
	virtual void beBribed() {}
};

////////////////////////////////////////////////////////////////////
class FrackMan : public Actor
{
public:
	FrackMan(StudentWorld* wldptr);
	virtual ~FrackMan();

	//Accessors
	int getHitPts() const { return hitPts; }
	int getNumWater() const { return numWater; }
	int getNumSonar() const { return numSonar; }
	int getNumGold() const { return numGold; }
	virtual bool isBoulder() const { return false; }
	virtual bool isProtest() const { return false; }
	virtual void beBribed() {}

	//Mutators
	virtual void doSomething();
	virtual void annoy(bool byBould) { hitPts -= 2; }  
	virtual void incNumWater() { numWater += 5; }
	virtual void incNumSonar() { numSonar++; }
	virtual void incNumGold() { numGold++; }

private:
	int hitPts;
	int numWater;
	int numSonar;
	int numGold;
};

////////////////////////////////////////////////////////////////////
class Boulder : public Actor
{
public:
	enum bState { stable, waiting, falling };
	Boulder(int x, int y, StudentWorld* wldptr);
	virtual ~Boulder();
	//Accessors
	bState getBState() { return m_state; }
	virtual bool isBoulder() const { return true; }
	virtual bool isProtest() const { return false; }
	virtual void beBribed() {}
	//Mutators
	virtual void doSomething(); 
	virtual void annoy(bool byBould) {}  //does nothing
	void changeBState(bState newState) { m_state = newState; }

private:
	bState m_state;
	int m_waitTime;
};

///////////////////////////////////////////////////////////////////////////
//oil, gold, sonar, water are derived classes of this
class ActivatingObject : public Actor
{
public:
	ActivatingObject(int x, int y, StudentWorld* worldptr, 
		int imageID, bool initVisibility, bool isTemp);
	virtual ~ActivatingObject() {}

	//Accessors
	virtual bool isBoulder() const { return false; }
	virtual bool isProtest() const { return false; }
	virtual bool canFrackGet() const = 0;
	virtual int getSoundID() = 0;
	virtual int getScoreInc() = 0;
	virtual void beBribed() {}

	//Mutators
	virtual void doSomething();
	virtual void annoy(bool byBould) {} // does nothing
	virtual void frackFoundObj(int soundID, int scoreInc);  //r<=3
	virtual void frackNearObj() = 0;   //r<=4
	virtual void informObjObtained() = 0;
	virtual void tempObjDoesStuff() = 0;

private:
	bool isTempObj;
};

/////////////////////////////////////////////////////////////

class Bribe : public ActivatingObject
{
public:
	Bribe(int x, int y, StudentWorld* worldptr);
	virtual ~Bribe() {}

	virtual int getSoundID() { return SOUND_PROTESTER_FOUND_GOLD; }
	virtual int getScoreInc() { return 25; }
	virtual bool canFrackGet() const { return false; }

	virtual void frackNearObj() {}
	virtual void informObjObtained() {};
	virtual void tempObjDoesStuff();

private:
	int ticksLeft;
};

/////////////////////////////////////////////////////////////////////////////
class Oil : public ActivatingObject
{
public:
	Oil(int x, int y, StudentWorld* worldptr);
	virtual ~Oil() {}
	
	virtual int getSoundID() { return SOUND_FOUND_OIL; }
	virtual int getScoreInc() { return 1000; }
	virtual bool canFrackGet() const { return true; }

	//Mutators
	virtual void frackNearObj();     //r<=4
	virtual void informObjObtained();
	virtual void tempObjDoesStuff() { return; }  //does nothing
};

/////////////////////////////////////////////////////////////////////

class Gold : public ActivatingObject
{
public:
	Gold(int x, int y, StudentWorld* worldptr);
	virtual ~Gold() {}

	virtual int getSoundID() { return SOUND_GOT_GOODIE; }
	virtual int getScoreInc() { return 10; }
	virtual bool canFrackGet() const { return true; }
	//Mutators
	virtual void frackNearObj();
	virtual void informObjObtained();
	virtual void tempObjDoesStuff() {} //does nothing
};

///////////////////////////////////////////////////////////////////
class Sonar : public ActivatingObject
{
public:
	Sonar(StudentWorld* worldptr);
	virtual ~Sonar() {}

	virtual int getSoundID() { return SOUND_GOT_GOODIE; }
	virtual int getScoreInc() { return 75; }
	virtual bool canFrackGet() const { return true; }
	//Mutators
	virtual void frackNearObj() {}  //does nothing
	virtual void informObjObtained();
	virtual void tempObjDoesStuff();
private:
	int ticksLeft;
};

//////////////////////////////////////////////////////////////////////
class Water : public ActivatingObject
{
public:
	Water(int x, int y, StudentWorld* worldptr);
	virtual ~Water() {}

	virtual int getSoundID() { return SOUND_GOT_GOODIE; }
	virtual int getScoreInc() { return 100; }
	virtual bool canFrackGet() const { return true; }
	//Mutators
	virtual void frackNearObj() {} //does nothing
	virtual void informObjObtained();
	virtual void tempObjDoesStuff();
private:
	int ticksLeft;
};

/////////////////////////////////////////////////////////////

class Protester : public Actor
{
public:
	Protester(StudentWorld* wldptr, int imageID, int pts);
	virtual ~Protester() {}

	//accessors
	virtual bool isBoulder() const { return false; }
	virtual bool isProtest() const { return true; }
	virtual int squirtAnnoyScore() const = 0;

	//mutators
	virtual void doSomething();
	virtual void annoy(bool byBould);  
	virtual bool doHardcoreDetect() = 0;
	virtual void decHitPts(int howMuch) { hitPts -= howMuch; }
	virtual void beBribed() = 0;
	virtual void setLeave() { m_isLeaving = true; }
	virtual void setStare(int howLong) { restingTicks = howLong; }
	virtual void attemptMove();

private:
	bool m_isLeaving;
	int restingTicks;
	int ticksToYell;  // init 0
	int numCurrDir;
	int ticksSincePerpTurn;  // init 0
	int hitPts;
	 //functions
	void setRestingTicks();
	void evalNewDir();
	
	bool yellIfPossible();
	bool inLineOfSight(Direction& newDir);
	bool canMoveTo(int ex, int ey);
};

///////////////////////////////////////////////////////////////////
//Regualr Protestor

class RegP : public Protester
{
public:
	RegP(StudentWorld* wldptr);
	virtual ~RegP() {}

	virtual bool doHardcoreDetect() { return false; }  //does nothing
	virtual int squirtAnnoyScore() const { return 100; }
	virtual void beBribed();  
};

///////////////////////////////////////////////////////////////////
//Harcore Protestor

class HardcoreP : public Protester
{
public:
	HardcoreP(StudentWorld* wldptr);
	virtual ~HardcoreP() {}

	virtual bool doHardcoreDetect();  
	virtual int squirtAnnoyScore() const { return 250; }
	virtual void beBribed();   
};

#endif // ACTOR_H_
