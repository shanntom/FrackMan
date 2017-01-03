#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include "Actor.h"  //need this .h file bc actors are created in world

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Coord;
class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir): GameWorld(assetDir)
	{}
	~StudentWorld();
	virtual int init(); 
	virtual int move();
	virtual void cleanUp();
	////////////////////////////////////
	
	//Mutators
	virtual void dig(int x, int y);
	virtual void setFrackDead();
	virtual void incOilFound() {numOilFound++; }
	virtual void foundWater();
	virtual void foundSonar();
	virtual void foundGold();
	virtual void revealObjects(int radius);
	virtual void protestToFrack();
	virtual void addBribe(int x, int y);
	virtual void addSquirt(int x, int y, int dir);
	virtual bool annoyAllProtestAt(int x, int y, int radius);
	void evalLeaveArr();
	void evalFrackArr(int fx, int fy);
	
	//Accessors
	virtual bool isMoveInField(int x, int y) const;
	virtual bool isMoveOnBould(int x, int y) const;   //r<=3
	virtual bool isDirtUnderBould(int x, int y) const;  
	virtual bool isBouldBelow(int x, int y) const;
	virtual bool bHitsFrack(int x, int y) const;  //r<=3
	virtual Actor* findNearbyFrackMan(Actor* a, int radius) const;
	virtual Actor* findNearbyProtest(Actor* a, int radius) const;
	virtual void playSound(const int sound) const;
	virtual bool isDirtAt(int x, int y) const;
	virtual bool isFacingFrack(Actor* a, int dir) const;
	virtual bool canMoveTo(int ex, int ey) const;
	virtual int getLeaveDir(int x, int y) const;
	virtual int getFrackDir(int x, int y) const;

private:
	/////////////
	class Coord
	{
	public:
		Coord(int xx, int yy, int dir) : m_x(xx), m_y(yy), m_dir(dir) {}
		int x() const { return m_x; }
		int y() const { return m_y; }
		char dir() const { return m_dir; }
	private:
		int m_x;
		int m_y;
		int m_dir;  // { none = 0, up =1, down =2, left=3 , right=4 }
	};////////////////
	std::vector<Actor*> vActors;
	FrackMan* frackPtr;   //pointer to frack man
	Actor* dirtArr[VIEW_WIDTH][VIEW_HEIGHT];   //2D array of pointers to Dirt objects
	Coord* leaveArr[61][61];  //pointers to leaving directions
	Coord* frackArr[61][61];
	
	int numOilFound;
	int targetOil;
	int ticksSinceAddedP;
	int numProtest;
	//int numLeaving;
	//bool mapChangedInLastTick;

	//functions
	void initializeDirt();
	void initRandPos(int &x, int &y);
	bool isPosValid(int x, int y) const;
	double getRadius(int x1, int y1, int x2, int y2) const;
	void findPosInField(int &x, int &y);
	void addnewGoodie();
	void addnewProtester();
	std::string getSBar() const;
	std::string formatInt(int num, int size, char c) const;
	template<typename T>    T min(T a, T b) const;
	template<typename T>    T max(T a, T b) const;
};

#endif // STUDENTWORLD_H_
