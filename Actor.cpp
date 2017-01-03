#include "Actor.h"
#include "StudentWorld.h"
//using namespace std; //** remove later for this file only

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp
/*
Actor(); //set visible to true
~Actor();
virtual void doSomething() = 0;
*/

//GRAPHOBJECT FUNCTIONS
//GraphObject(int imageID, int startX, int startY, Direction dir = right,
//           double size = 1.0, unsigned int depth = 0)
//void setVisible(bool shouldIDisplay)   weather or not to display obj
//int getX() const
//int getY() const
//void moveTo(int x, int y)
//Direction getDirection() const      up, dwon left , right
//void setDirection(Direction d)

///////////////////////////////////////
//ACTOR
Actor::Actor(int imageID, int sx, int sy, Direction d, double size,
	unsigned int depth, StudentWorld* wldptr)
	:GraphObject(imageID, sx, sy, d, size, depth), m_isAlive(true)
{
	setVisible(true);  //display actor
	m_world = wldptr;
}

Actor::~Actor()
{}

bool Actor::canMove(Direction d)
{
	int x = getX();
	int y = getY();
	if (d == up)
	{
		if (y + 1 < 0 || y + 1 >60)
			return false;
		return (!worldPtr()->isDirtAt(x, y + 1) && !worldPtr()->isMoveOnBould(x, y + 1));
	}
	else if (d == down)
	{
		if (y - 1 < 0 || y - 1>60)
			return false;
		return (!worldPtr()->isDirtAt(x, y - 1) && !worldPtr()->isMoveOnBould(x, y - 1));
	}
	else if (d == left)
	{
		if (x - 1 < 0 || x - 1 > 60)
			return false;
		return (!worldPtr()->isDirtAt(x - 1, y) && !worldPtr()->isMoveOnBould(x - 1, y));
	}
	else
	{
		if (x + 1< 0 || x + 1> 60)
			return false;
		return (!worldPtr()->isDirtAt(x + 1, y) && !worldPtr()->isMoveOnBould(x + 1, y));
	}
}

//////////////////////////////////////////////////
//FRACK MAN
FrackMan::FrackMan(StudentWorld* wldptr) 
	: Actor(IID_PLAYER, 30,60, right, 1.0, 0, wldptr)
{
	hitPts = 10;
	numWater = 5;
	numSonar = 1;
	numGold = 0;
}

FrackMan::~FrackMan()
{}

void FrackMan::doSomething()
{
	if (!isAlive())
		return;    //player is dead
	 //Completely annoyed
	if (hitPts <= 0)
	{
		setDead();
		worldPtr()->playSound(SOUND_PLAYER_GIVE_UP);
		return;
	}
	int x = getX();
	int y = getY();
	//update frackArray for hardcore protesters
	worldPtr()->evalFrackArr(x, y);
	//dig through dirt that overlaps with man's position
	for (int fx = x; fx <= x + 3; fx++)
		for (int fy = y; fy <= y + 3; fy++)
			worldPtr()->dig(fx, fy);  //dig will play sound
	
	//player pressed a key. ch is set to the key with getKey
	int ch;
	if (worldPtr()->getKey(ch) == true)  
	{
		switch (ch)
		{
	//escape key
		case KEY_PRESS_ESCAPE:
			hitPts = 0;     //aka dead
			break;
	//****spacebar = squirt
		case KEY_PRESS_SPACE:
			if (numWater > 0)
			{
				numWater--;
				worldPtr()->addSquirt(getX(), getY(), getDirection());
			}
			break;
	//Move (If possible) 
		case KEY_PRESS_LEFT:
			if (getDirection() != left)
				setDirection(left);
			else if (worldPtr()->isMoveInField(x - 1, y) 
				&& !( worldPtr()->isMoveOnBould(x-1,y) ) ) //moving left possible
				moveTo(x - 1, y);
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() != right)
				setDirection(right);
			else  if (worldPtr()->isMoveInField(x + 1, y) 
				&& !(worldPtr()->isMoveOnBould(x+1, y)) )
				moveTo(x + 1, y);
			break;
		case KEY_PRESS_UP:
			if (getDirection() != up)
				setDirection(up);
			else if (worldPtr()->isMoveInField(x, y + 1) 
				&& !(worldPtr()->isMoveOnBould(x, y+1)) )
				moveTo(x, y + 1);
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() != down)
				setDirection(down);
			else if (worldPtr()->isMoveInField(x, y -1)
				&& !(worldPtr()->isMoveOnBould(x, y-1)) )
				moveTo(x, y - 1);
			break;
	//Z , z use  sonar
		case 'Z':
		case 'z':
			if (numSonar>0)
			{
				numSonar--;
				worldPtr()->revealObjects(12);
			}
			break;
	//tab to drop bribe
		case KEY_PRESS_TAB:
			if (numGold > 0)
			{
				numGold--;
				worldPtr()->addBribe(getX(), getY());
			}
			break;
		}
	}

}


////////////////////////////////////////////
//DIRT
Dirt::Dirt(int x, int y, StudentWorld* wldptr) 
	:Actor(IID_DIRT, x,y, right, .25, 3, wldptr)
{}

Dirt::~Dirt()
{}


/////////////////////////////////////////////////
//BOULDER
Boulder::Boulder(int x, int y, StudentWorld* wldptr)
	:Actor(IID_BOULDER, x, y, down, 1.0, 1, wldptr), m_state(stable), m_waitTime(0)
{}

Boulder::~Boulder()
{}

void Boulder::doSomething()
{
	if (!isAlive())  //boulder not alive
		return;
	int x = getX();
	int y = getY();
	if (m_state == stable)
	{
		if (!(worldPtr()->isDirtUnderBould(x, y)))  //check for dirt
			m_state = waiting;
	}
	else if (m_state == waiting)
	{
		if (m_waitTime >= 30)
		{
			m_state = falling;
			worldPtr()->playSound(SOUND_FALLING_ROCK);
		}
		else
			m_waitTime++;
	}
	else if (m_state == falling)
	{
		//hits dirt, bottom of oil field, another boulder, frackman, protester
		if ((worldPtr()->isDirtUnderBould(x, y)) ) //hits dirt
			setDead();
		else if (y == 0)  //bottom of oil field
			setDead();
		else if (worldPtr()->isBouldBelow(x, y))
			setDead();
		//boulder hits at least one protester or frackman
		Actor* p = worldPtr()->findNearbyProtest(this, 3);
		if (p != nullptr)  //there is a protester to be hit
		{
			p->annoy(true);
		}
		//boulder hits frackman
		if (worldPtr()->bHitsFrack(x, y))
			worldPtr()->setFrackDead();
		//hits nothing -> moves
		moveTo(getX(), getY() - 1);
	}
}
/////////////////////////////////////////////////////////////////
//Squirt
Squirt::Squirt(int x, int y, StudentWorld* worldptr, int d)
	:Actor(IID_WATER_SPURT, x, y, none, 1.0, 1, worldptr), ticksLeft(4)
{
	if (d == 1) // up
		setDirection(up);
	if (d == 2)  //down
		setDirection(down);
	if (d == 3) //left
		setDirection(left);
	if (d == 4) //right
		setDirection(right);
}

void Squirt::doSomething()
{
	//annoy protesters
	if (worldPtr()->annoyAllProtestAt(getX(), getY(), 3 ) == true)
	{
		setDead();
		return;
	} //die
	if (ticksLeft == 0)
	{
		setDead();
		return;
	}
	//blocked
	Direction dir = getDirection();
	if (!canMove(dir))
	{
		setDead();
		return;
	}
	else  //move 
	{
		if (dir == up)
			moveTo(getX(), getY() + 1);
		if (dir == down)
			moveTo(getX(), getY() - 1);
		if (dir == left)
			moveTo(getX()-1, getY() );
		if (dir == right)
			moveTo(getX() +1 , getY());
	}
	ticksLeft--;
}



////////////////////////////////////////////////////////////////////////
//ActivatingObject

ActivatingObject::ActivatingObject(int x, int y, StudentWorld* worldptr,
	int imageID, bool initVisibility, bool isTemp) : 
	Actor(imageID, x,y,right, 1.0, 2, worldptr), isTempObj(isTemp)
{
	if (initVisibility == false)
		setVisible(false);
}

void ActivatingObject::doSomething()
{
	if (!isAlive())  //not alive
		return;
	if (canFrackGet() == true)
	{
		if (worldPtr()->findNearbyFrackMan(this, 3) != nullptr)  //frackman found obj
		{
			frackFoundObj(getSoundID(), getScoreInc());
			informObjObtained();
		}
		else if (!isTempObj &&
			worldPtr()->findNearbyFrackMan(this, 4) != nullptr)  //frack near permanent obj
		{
			frackNearObj();
			return;
		}
	}
	if (canFrackGet() == false)  //stuff for bribe
	{
		Actor* p = worldPtr()->findNearbyProtest(this, 3);
		if (p != nullptr)   // there is a protester that can be bribed
		{
			setDead(); 
			worldPtr()->playSound(SOUND_PROTESTER_FOUND_GOLD);
			//bribe the protester. score increased in function
			(p)->beBribed();
		}
	}
	if (isTempObj)
		tempObjDoesStuff();
}

void ActivatingObject::frackFoundObj(int soundID, int scoreInc)
{
	setDead();
	worldPtr()->playSound(soundID);
	worldPtr()->increaseScore(scoreInc);
}


////////////////////////////////////////////////////////////////////////
//OIL Barrel

Oil::Oil(int x, int y, StudentWorld* worldptr)
	:ActivatingObject(x, y, worldptr, IID_BARREL, false, false)
{}

void Oil::frackNearObj()
{
	setVisible(true);
}

void Oil::informObjObtained()
{
	worldPtr()->incOilFound();
}

//////////////////////////////////////////////////////////////////////////
//Gold nugget (only pickup- able by frack man)

Gold::Gold(int x, int y, StudentWorld* worldptr)
	:ActivatingObject(x, y, worldptr, IID_GOLD, false, false)
{}

void Gold::frackNearObj()
{
	setVisible(true);
}

void Gold::informObjObtained()
{
	worldPtr()->foundGold();
}

/////////////////////////////////////////////////////////////////////////
//Bribe (only pick-upable by protesters)

Bribe::Bribe(int x, int y, StudentWorld* worldptr)
	:ActivatingObject(x, y, worldptr, IID_GOLD, true, true)
{
	ticksLeft = 100;
}

void Bribe::tempObjDoesStuff()
{
	if (ticksLeft <= 0)
		setDead();
	else
		ticksLeft--;
}

/////////////////////////////////////////////////////////////////////////
//Sonar

Sonar::Sonar( StudentWorld* worldptr)
	:ActivatingObject(0,60, worldptr, IID_SONAR, true, true)
{
	if (100 > 300 - 10 * (worldptr->getLevel()))
		ticksLeft = 100;
	else
		ticksLeft = 300 - 10 * (worldptr->getLevel());
}

void Sonar::informObjObtained()
{
	worldPtr()->foundSonar();
}

void Sonar::tempObjDoesStuff()
{
	if (ticksLeft <= 0)
		setDead();
	else
		ticksLeft--;
}

////////////////////////////////////////////////////////////////////////
//Water
Water::Water(int x, int y, StudentWorld* worldptr)
	:ActivatingObject(x, y, worldptr, IID_WATER_POOL, true, true)
{
	if (100 > 300 - 10 * worldptr->getLevel())
		ticksLeft = 100;
	else
		ticksLeft = 300 - 10 * worldptr->getLevel();
}

void Water::informObjObtained()
{
	worldPtr()->foundWater();
}

void Water::tempObjDoesStuff()
{
	if (ticksLeft <= 0)
		setDead();
	else
		ticksLeft--;
}

///////////////////////////////////////////////////////////////
//PROTESTER
Protester::Protester(StudentWorld* wldptr, int imageID, int pts)
	:Actor(imageID, 60, 60, left, 1.0, 0, wldptr), m_isLeaving(false),
	ticksToYell(0), numCurrDir(0), ticksSincePerpTurn(0), hitPts(pts)
{
	if (0 > 3 - wldptr->getLevel() / 4)
		restingTicks = 0;
	else
		restingTicks = 3 - wldptr->getLevel() / 4;
}

void Protester::doSomething()
{
	ticksSincePerpTurn++;
	if (!isAlive())
		return;
	if (restingTicks != 0)  //resting -> do nothing
	{
		restingTicks--;
		return;
	}
	else
		setRestingTicks();  // re-set resting ticks	
	ticksToYell--;  //nonresting ticks
	//leaving state
	if (m_isLeaving)
	{
		int x = getX();
		int y = getY();
		if (x == 60 && y == 60)
		{
			setDead();
			return;
		}
		worldPtr()->evalLeaveArr();
		int d = worldPtr()->getLeaveDir(getX(), getY());
		Direction newDir;
		if (d == 0)
			newDir = none;
		if (d == 1)
			newDir = up;
		if (d == 2)
			newDir = down;
		if (d == 3)
			newDir = left;
		if (d == 4)
			newDir = right;
		setDirection(newDir);
		attemptMove();
		return;
	}
	//within 4 units of frack, facing frack  -->yell every 15
	if (yellIfPossible())
		return;
	//hardcore searches for frack
	if (doHardcoreDetect() == true)
		return;
	//hoiz/vert line of sight, >4 units away, can move in straight line 
	Direction possibleDir = none;
	if (inLineOfSight(possibleDir))
	{
		setDirection(possibleDir);
		attemptMove();
		numCurrDir = 0;
		return;
	}
	//numCurrDir and dir:  decrement numCurDir, or re-evaluate (numCurDir ==0)
		//at intersection: re-evaluate numCurDir
	evalNewDir();
	//attempt to move in current Direction . if blocked -> set numCurrDir = 0
	attemptMove();
}

void Protester::setRestingTicks()
{
	if (0 > 3 - worldPtr()->getLevel() / 4)
		restingTicks = 0;
	else
		restingTicks = 3 - worldPtr()->getLevel() / 4;
}

void Protester::evalNewDir()
{
	numCurrDir--;
	Direction prevDir = getDirection();
	Direction newDir = none;
	//no more steps
	if (numCurrDir <= 0)
	{
		do     //find a direction that it can move in
		{
			newDir = right;
			int randNum = rand() % 4 + 1;  //yields 1-4
			if (randNum == 1)
				 newDir = up;
			else if (randNum == 2)
				 newDir = down;
			else if (randNum == 3)
				 newDir = left;
		} while (!canMove(newDir));  //re-evaluates if can't move in chosen dir
		numCurrDir = rand() % 53 + 8;   //yields 8-60
	}
	//at intersection and hasn't turned perp in 200 ticks
	if (ticksSincePerpTurn >= 200)
	{
		if (prevDir == left || prevDir == right)
		{
			if (canMove(up) || canMove(down))  //make a perp turn
			{
				int randNum = rand() % 2;
				if (randNum == 1)  //try to move up first
				{
					if (canMove(up))
						newDir = up;
					else
						newDir = down;
				}
				else    //try to move down first
				{
					if (canMove(down))
						newDir = down;
					else
						newDir = up;
				}
				ticksSincePerpTurn = 0;
				numCurrDir = rand() % 53 + 8;   //yields 8-60
			}
		}
		else if (prevDir == up || prevDir == down)
		{
			if (canMove(left) || canMove(right))  //make a perp turn
			{
				int randNum = rand() % 2;
				if (randNum == 1)
				{
					if (canMove(left))
						newDir = left;
					else
						newDir = right;
				}
				else
				{
					if (canMove(right))
						newDir = right;
					else
						newDir = left;
				}
				ticksSincePerpTurn = 0;
				numCurrDir = rand() % 53 + 8;
			}
		}
	}
	if (newDir != none)
		setDirection(newDir);
}

bool Protester::canMoveTo(int ex, int ey)
{
	if (ex  < 0 || ey  > 60)
		return false;
	if (ex < 0 || ex  > 60)
		return false;
	return (!worldPtr()->isDirtAt(ex, ey) && !worldPtr()->isMoveOnBould(ex, ey));
}

void Protester::attemptMove()
{
	Direction d = getDirection();
	if (canMove(d))
	{
		if (d == up)
			moveTo(getX(), getY() + 1);
		 if (d == down)
			moveTo(getX(), getY() - 1);
		 if (d == left)
			moveTo(getX() - 1, getY());
		 if (d == right)
			moveTo(getX() + 1, getY());
	}
	else if (!canMove(d))//can't move
		numCurrDir = 0;
}

bool Protester::yellIfPossible()
{
	if (ticksToYell <= 0 && worldPtr()->findNearbyFrackMan(this, 4) != nullptr)
	{
		if (worldPtr()->isFacingFrack(this, getDirection()))
		{
			worldPtr()->playSound(SOUND_PROTESTER_YELL);
			worldPtr()->protestToFrack();
			ticksToYell = 15;
			return true;
		}
	}
	return false;
}

bool Protester::inLineOfSight(Direction& newDir)
{
	if (worldPtr()->findNearbyFrackMan(this, 4) == nullptr)  //more that 4 units away
	{
		const int px = getX();
		const int py = getY();
		Actor* frack = worldPtr()->findNearbyFrackMan(this, 120);
		if (frack == nullptr)  //not there?!
			return false;
		const int fx = frack->getX();
		const int fy = frack->getY();
		if (px == fx) //in vertical sight
		{
			if (fy > py)  //frack above protester
			{
				for (int k = 1; k + py < fy; k++)
					if (!canMoveTo(px, py + k))
						return false;
				newDir = up;
				return true;
			}
			if (fy < py)  //frack below protester
			{
				for (int k = 1; py - k > fy; k++)
					if (!canMoveTo(px, py - k))
						return false;
				newDir = down;
				return true;
			}
		}
		if (py == fy) //in horiz sight
		{
			if (fx > px)  //frack to right
			{
				for (int k = 1; k + px < fx; k++)
					if (!canMoveTo(px + k, py))
						return false;
				newDir = right;
				return true;
			}
			if (fx < px)  //frack to left
			{
				for (int k = 1; px - k > fx; k++)
					if (!canMoveTo(px - k, py))
						return false;
				newDir = left;
				return true;
			}
		}
	}
	return false;
}

void Protester::annoy(bool byBould)
{
	if (m_isLeaving)
		return;
	if (byBould == true)
	{
		decHitPts(100);
	}
	else  //squirt
		decHitPts(2);
	if (hitPts > 0) //not completely annoyed
	{
		worldPtr()->playSound(SOUND_PROTESTER_ANNOYED);
		int N;
		if (50 > 100 - worldPtr()->getLevel() * 10)
			N = 50;
		else
			N = 100 - worldPtr()->getLevel() * 10;
		restingTicks = N;
		return;
	}
	if (hitPts <= 0)  //completely annoyed
	{
		m_isLeaving = true;
		//worldPtr()->incNumLeaving();
		worldPtr()->playSound(SOUND_PROTESTER_GIVE_UP);
		worldPtr()->evalLeaveArr();  //initialize array (map may not change in next tick)
		restingTicks = 0;
		if (byBould == true)
		{
			worldPtr()->increaseScore(500);
		}
		else
			worldPtr()->increaseScore(squirtAnnoyScore());
	}
}

/////////////////////////////////////////////////////////////////////////

RegP::RegP(StudentWorld* wldptr)
	:Protester(wldptr, IID_PROTESTER, 5)
{}

void RegP::beBribed()
{
	worldPtr()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	worldPtr()->increaseScore(25);
	setLeave();
}

//////////////////////////////////////////////////////////
HardcoreP::HardcoreP(StudentWorld* wldptr)
	: Protester(wldptr, IID_HARD_CORE_PROTESTER, 20)
{}

void HardcoreP::beBribed()
{
	worldPtr()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	worldPtr()->increaseScore(50);
	int ticksStare;
	if (50 > 100 - worldPtr()->getLevel() * 10)
		ticksStare = 50;
	else
		ticksStare = 100 - worldPtr()->getLevel() * 10;
	setStare(ticksStare);
}

bool HardcoreP::doHardcoreDetect()
{
	int d = worldPtr()->getFrackDir(getX(), getY());
	Direction newDir;
	if (d == 0)
		return false; //dont do anything
	if (d == 1)
		newDir = up;
	if (d == 2)
		newDir = down;
	if (d == 3)
		newDir = left;
	if (d == 4)
		newDir = right;
	setDirection(newDir);
	attemptMove();
	return true;
}
