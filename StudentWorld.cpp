#include "StudentWorld.h"
#include "GameController.h"
#include <string>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::~StudentWorld()
{
	//delete frack man
	delete frackPtr;
	//delete dirt objects
	for (int x = 0; x < VIEW_WIDTH; x++)
		for (int y = 0; y < 60; y++)
			if (dirtArr[x][y] != nullptr)
				delete dirtArr[x][y];
	// delete actors
	for (vector<Actor*>::iterator p = vActors.begin(); p != vActors.end(); p++)
		delete *p;
	//delete coordinates in Coord* arrays
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
			delete leaveArr[x][y];
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
			delete frackArr[x][y];
}

int StudentWorld::init()
{
	frackPtr = new FrackMan(this);	//add frack man to game
	initializeDirt();
	ticksSinceAddedP = 1000;
	numProtest = 0;
	//numLeaving = 0;
	//find number of boulders, nuggets, and barrels there should be
	int level = getLevel();
	int numB = min(level / 2 + 2, 6);   //#boulders
	int numG = max(5 - level / 2, 2);	//# gold
	int numO = min(2 + level, 20);		//# oil barrels
	//initialize oil requirements
	numOilFound = 0;
	targetOil = numO;
	//populate with boulders
	for (int k = 0; k < numB; k++)  
	{
		int x, y;
		do
		initRandPos(x, y); //stores a position into x, y that is not in shaft, at surface
		while (!isPosValid(x, y));  // re-eavluates a pos if pos is not valid (r<6)
		vActors.push_back(new Boulder(x, y, this));
		for (int dx = x; dx < x+4; dx++)  //remove dirt
			for (int dy = y; dy < y+4 ; dy++)
			{
				delete dirtArr[dx][dy];
				dirtArr[dx][dy] = nullptr;
			}
	}
	//populate with oil
	for (int k = 0; k < numO; k++)
	{
		int x, y;
		do
			initRandPos(x, y); //stores a position into x, y that is not in shaft, at surface
		while (!isPosValid(x, y));  // re-eavluates a pos if pos is not valid (r<6)
		vActors.push_back(new Oil(x, y, this));
	}
	//populate with gold
	for (int k = 0; k < numG; k++)
	{
		int x, y;
		do
			initRandPos(x, y);
		while (!isPosValid(x, y));
		vActors.push_back(new Gold(x, y, this));
	}
	//initialize Coord* arrays with nullptrs
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
			leaveArr[x][y] = nullptr;
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
			frackArr[x][y] = nullptr;
	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	//mapChangedInLastTick = false;
	//update status text on the top of the screen
	setGameStatText(getSBar());
	//ask actors to doSomething
	frackPtr->doSomething();
	for (vector<Actor*>::iterator p = vActors.begin(); p != vActors.end(); p++)
		if ((*p)->isAlive())
		{
			(*p)->doSomething();
			//check if frackman is Dead
			if (!frackPtr->isAlive())
			{
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}
			//if frackman colleccted all the oil
			if (numOilFound == targetOil) 
				return GWSTATUS_FINISHED_LEVEL;
		}
	//delete any dead actors
	vector<Actor*>::iterator p = vActors.begin();
	while (p != vActors.end())
	{
		if (!(*p)->isAlive())
		{
			if ((*p)->isProtest())
			{
				numProtest--;
				//numLeaving--;
			}
			delete *p;
			p = vActors.erase(p);  //increments p
		}
		else
			p++;  //increment p damn those infinite loops
	}	
	//check if frackman is Dead
	if (!frackPtr->isAlive())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	//if frackman colleccted all the oil
	if (numOilFound == targetOil)
	return GWSTATUS_FINISHED_LEVEL;
	//add new actors
	ticksSinceAddedP++;
	addnewGoodie();
	addnewProtester();

	return GWSTATUS_CONTINUE_GAME;  //return other values later
}

void StudentWorld::cleanUp()
{
	//delete frack man
	delete frackPtr;
	//delete dirt objects
	for (int x = 0; x < VIEW_WIDTH; x++)
		for (int y = 0; y < 60; y++)
			if (dirtArr[x][y] != nullptr)
				delete dirtArr[x][y];
	// delete actors
	vector<Actor*>::iterator p = vActors.begin();
	while (p != vActors.end())
	{
		delete *p;
		p = vActors.erase(p);
	}
	//delete coordinates in Coord* arrays
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
		{
			delete leaveArr[x][y];
		}
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
		{
			delete frackArr[x][y];
		}
}
/////////////////////////////////////////////////////////

void StudentWorld::evalLeaveArr()
{
	//clear array
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
		{
			delete leaveArr[x][y];
			leaveArr[x][y] = nullptr;
		}
	//initialize queue
	queue<Coord> coordQ;
	leaveArr[60][60] = new Coord(60, 60, 0);  //start at exit
	coordQ.push(*leaveArr[60][60]);
	while (!coordQ.empty())
	{
		Coord curr = coordQ.front();
		int x = curr.x();
		int y = curr.y();
		coordQ.pop();
		//NORTH
		if (y!=60 && leaveArr[x][y + 1] == nullptr && canMoveTo(x, y + 1))
		{
			leaveArr[x][y + 1] = new Coord(x, y + 1, 2); //down
			coordQ.push(*leaveArr[x][y + 1]);
		}
		//EAST
		if (x !=60 && leaveArr[x+1][y]==nullptr && canMoveTo(x + 1, y))
		{
			leaveArr[x + 1][y] = new Coord(x + 1, y, 3); //left
			coordQ.push(*leaveArr[x + 1][y]);
		}
		//SOUTH
		if (y != 0 && leaveArr[x][y-1] == nullptr && canMoveTo(x, y - 1))
		{
			leaveArr[x][y - 1] = new Coord(x, y - 1, 1); //up
			coordQ.push(*leaveArr[x][y - 1]);
		}
		//WEST
		if (x != 0 && leaveArr[x -1][y] == nullptr && canMoveTo(x - 1, y) )
		{
			leaveArr[x - 1][y] = new Coord(x - 1, y, 4);  //right
			coordQ.push(*leaveArr[x - 1][y]);
		}
	}
}

void StudentWorld::evalFrackArr(int fx, int fy)
{
	//clear array
	for (int x = 0; x < 61; x++)
		for (int y = 0; y < 61; y++)
		{
			delete frackArr[x][y];
			frackArr[x][y] = nullptr;
		}
	//initialize queue
	queue<Coord> coordQ;
	frackArr[fx][fy] = new Coord(fx, fy, 0);  //start at frackman
	coordQ.push(*frackArr[fx][fy]);
	//int loop = 0;
	int maxSteps = 16 + getLevel() * 2;
	while (!coordQ.empty())
	{
		Coord curr = coordQ.front();
		int x = curr.x();
		int y = curr.y();
		coordQ.pop();
		
		int steps = 0;
		int xs = x;
		int ys = y;
		while (!(xs == fx && ys == fy))
		{
			if (frackArr[xs][ys] == nullptr)  //something went wrong
				return;
			if (frackArr[xs][ys]->dir() == 0)
				return;
			if (frackArr[xs][ys]->dir() == 2) //down
				ys--;
			if (frackArr[xs][ys]->dir() == 3) //left
				xs--;
			if (frackArr[xs][ys]->dir() == 1) //up
				ys++;
			if (frackArr[xs][ys]->dir() == 4) //right
				xs++;
			steps++;
		}
		if (steps >= maxSteps)
			return;

		//NORTH
		if (y!=60 && frackArr[x][y + 1] == nullptr && canMoveTo(x, y + 1))
		{
			frackArr[x][y + 1] = new Coord(x, y + 1, 2); //down
			coordQ.push(*frackArr[x][y + 1]);
		}
		//EAST
		if (x !=60 && frackArr[x + 1][y] == nullptr && canMoveTo(x + 1, y))
		{
			frackArr[x + 1][y] = new Coord(x + 1, y, 3); //left
			coordQ.push(*frackArr[x + 1][y]);
		}
		//SOUTH
		if (y != 0 && frackArr[x][y - 1] == nullptr && canMoveTo(x, y - 1))
		{
			frackArr[x][y - 1] = new Coord(x, y - 1, 1); //up
			coordQ.push(*frackArr[x][y - 1]);
		}
		//WEST
		if (x != 0 && frackArr[x - 1][y] == nullptr && canMoveTo(x - 1, y) )
		{
			frackArr[x - 1][y] = new Coord(x - 1, y, 4);  //right
			coordQ.push(*frackArr[x - 1][y]);
		}
		//loop++;
	}
}

bool StudentWorld::canMoveTo(int ex, int ey) const
{
	if (ex < 0 || ey  > 60)
		return false;
	if (ex < 0 || ex  > 60)
		return false;
	return (!isDirtAt(ex, ey) && !isMoveOnBould(ex, ey));
}

void StudentWorld::addnewGoodie()
{
	int G = getLevel() * 25 + 300;
	if (rand() % G == 1)   // 1 in G chance of adding goodie
	{
		if (rand() % 5 == 1)  //1 in 5 chance of adding sonar
			vActors.push_back(new Sonar(this));
		else  //add water
		{
			int x, y;
			findPosInField(x, y);
			vActors.push_back(new Water(x, y, this));
		}
	}
}

void StudentWorld::addnewProtester()
{
	int level = getLevel();
	int T = max(25, 200 - level);
	int templatesSuck = level* 1.5;
	int maxP = min(12, 2 + templatesSuck);
	if (ticksSinceAddedP > T && numProtest < maxP)  //can add a protester
	{
		int probHardcore = min(90, level * 10 + 30);
		if (probHardcore < rand() % 100 + 1)  // probHardcore/100 chance to add hardcore
			vActors.push_back(new HardcoreP(this));
		else
			vActors.push_back(new RegP(this));
		ticksSinceAddedP = 0;
		numProtest++;
	}
}

void StudentWorld::dig(int x, int y)
{
	if (dirtArr[x][y] != nullptr)
	{
		delete dirtArr[x][y];
		GameController::getInstance().playSound(SOUND_DIG);
		dirtArr[x][y] = nullptr;
		//mapChangedInLastTick = true;  
	}
}

void StudentWorld::setFrackDead()
{
	frackPtr->setDead();
}

void StudentWorld::foundWater()
{
	frackPtr->incNumWater();
}

void StudentWorld::foundSonar()
{
	frackPtr->incNumSonar();
}

void StudentWorld::foundGold()
{
	frackPtr->incNumGold();
}

void StudentWorld::revealObjects(int radius)
{
	for(vector<Actor*>::const_iterator p = vActors.begin(); p != vActors.end(); p++)
	{
		double r = getRadius(frackPtr->getX() + 2, frackPtr->getY() + 2, (*p)->getX() + 2, (*p)->getY() + 2);
		if (r <= radius)
			((*p)->setVisible(true));
	}
}

void StudentWorld::protestToFrack()
{
	frackPtr->annoy(true);
}

void StudentWorld::addBribe(int x, int y) 
{
	vActors.push_back(new Bribe(x, y, this));
}

void StudentWorld::addSquirt(int x, int y, int dir)
{
	if (dir == 1)  //up
		vActors.push_back(new Squirt(x, y + 4, this, dir));
	if (dir == 2)  //down
		vActors.push_back(new Squirt(x, y - 4, this, dir));
	if (dir == 3)  //left
		vActors.push_back(new Squirt(x-4, y , this, dir));
	if (dir == 4)  //right
		vActors.push_back(new Squirt(x+4, y, this, dir));
}

bool StudentWorld::annoyAllProtestAt(int x, int y, int radius)
{
	//for squirts
	int numAnnoyed = 0;
	for (vector<Actor*>::const_iterator p = vActors.begin(); p != vActors.end(); p++)
	{
		if ((*p)->isProtest())
		{
			int px = (*p)->getX() + 2;
			int py = (*p)->getY() + 2;
			if (getRadius(x + 2, y + 2, px, py) <= radius)
			{
				(*p)->annoy(false);  //annoy by squirt
				numAnnoyed++;
			}
		}
	}
	if (numAnnoyed == 0)  //no protesters annoyed
		return false;
	return true;
}
////////////////////////////////////////////////////////////////////
bool StudentWorld::isMoveInField(int x, int y) const
{
	if (x < 0 || x > 60)
		return false;
	if (y<0 || y > 60)
		return false;
	return true;
}

bool StudentWorld::isMoveOnBould(int x, int y) const
{
	//frackman can't be an a square that is <= 3 from center of boulder
	for (vector<Actor*>::const_iterator p = vActors.begin(); p != vActors.end(); p++)
		if ((*p)->isBoulder())
		{
			const int cx = (*p)->getX() + 2; // get coordinates of center of boulder
			const int cy = (*p)->getY() + 2 ;
			//find radius from middle of frackman to center of boulder
			double r = getRadius(x + 2, y + 2, cx, cy);
			if (r <= 3.0)
				return true;
		}
	return false;
}

bool StudentWorld::isDirtUnderBould(int x, int y) const
{
	if (y == 0)
		return true;  //boulder is at the bottom -> it can't fall
	for (int k = 0; k < 4; k++)
		if (dirtArr[x + k][y - 1] != nullptr)
			return true;
	return false;  //no dirt
}

bool StudentWorld::isBouldBelow(int x, int y) const
{
	for (vector<Actor*>::const_iterator p = vActors.begin(); p != vActors.end(); p++)
	{
		if ((*p)->isBoulder())
		{
			int bx = (*p)->getX();
			int byTop = (*p)->getY() +3 ; //top of stable boulder
			for (int k = 0; k < 4; k++)
				for (int i = 0; i < 4; i++)
					if ((x + k) == (bx + i) && y == byTop)
						return true;
		}
	}
	return false;
}

bool StudentWorld::bHitsFrack(int x, int y) const
{
	int fx = frackPtr->getX() +2 ;  //center of frackman
	int fy = frackPtr->getY() + 2;  
	if (getRadius(fx, fy, x+2, y+2) <= 3)
			return true;
	return false;
}

Actor* StudentWorld::findNearbyFrackMan(Actor* a, int radius) const
{
	// If the FrackMan is within radius of a, return a pointer to the
	// FrackMan, otherwise null.
	int ax = a->getX() +2 ;
	int ay = a->getY()+2 ;
	int fx = frackPtr->getX()+2;
	int fy = frackPtr->getY()+2;
	if (getRadius(ax, ay, fx, fy) <= radius)
		return frackPtr;
	else
		return nullptr;
}

Actor* StudentWorld::findNearbyProtest(Actor* a, int radius) const
{
	// If the portest is within radius of a, return a pointer to the
	// protester, otherwise null.
	int ax = a->getX() + 2;
	int ay = a->getY() + 2;
	for (vector<Actor*>::const_iterator p = vActors.begin(); p != vActors.end(); p++)
	{
		if ((*p)->isProtest())
		{
			int px = (*p)->getX()+ 2;
			int py = (*p)->getY() +2;
			if (getRadius(ax, ay, px, py) <= radius)
				return *p;
		}
	}
	return nullptr;
}

void StudentWorld::playSound(const int sound) const
{
	GameController::getInstance().playSound(sound);
}

bool StudentWorld::isFacingFrack(Actor* a, int dir) const  //given hes a 4 units away already
{
	int ax = a->getX() ;
	int ay = a->getY() ;
	int fx = frackPtr->getX() ;
	int fy = frackPtr->getY() ;
	if (dir == 1) // up
		for (int k = 0; k <= 4; k++)
			if (ay + k == fy)
				return true;
	if (dir == 2)  //down
		for (int k = 0; k <= 4; k++)
			if (ay - k == fy)
				return true;
	if (dir == 3)  //left
		for (int k = 0; k <= 4; k++)
			if (ax - k == fx)
				return true;
	if (dir == 4)  //right
		for (int k = 0; k <= 4; k++)
			if (ax + k == fx)
				return true;
	return false;
}

int StudentWorld::getLeaveDir(int x, int y) const
{
	if (leaveArr[x][y] == nullptr)
		return 0;   //this aint supposed to happen
	return leaveArr[x][y]->dir();
}

int StudentWorld::getFrackDir(int x, int y) const
{
	if (frackArr[x][y] == nullptr)
		return 0;   
	return frackArr[x][y]->dir();
}

//////////////////////////////////////////////////////////////////////

void StudentWorld::initializeDirt()
{
	//dirt covers 64 wide, 60 high
	for (int x = 0; x < VIEW_WIDTH; x++)
		for (int y = 0; y < 60; y++)
			dirtArr[x][y] = new Dirt(x, y, this);
	//set nullptr for top part where there is no nullptr
	for (int x = 0; x < VIEW_WIDTH; x++)
		for (int y = 60; y < VIEW_HEIGHT; y++)
			dirtArr[x][y] = nullptr;
	//create a single mine shaft
	for (int x = 30; x <= 33; x++)
		for (int y = 4; y < 60; y++)
		{
			delete dirtArr[x][y];
			dirtArr[x][y] = nullptr;
		}
}

template<typename T>
T StudentWorld::min(T a, T b) const
{
	if (a < b)
		return a;
	else return b;
}

template<typename T>
T StudentWorld::max(T a, T b) const
{
	if (a > b)
		return a;
	else return b;
}

void StudentWorld::initRandPos(int &x, int& y) 
{
	x = -1;
	y = -1;
	while (! ((x >= 0 && x <= 26) || (x >= 34 && x <= 60)) ) //no objects in shaft/ surface
		x = rand() % 61;  // rand number between 0 - 60
	while (!( (y >= 0 && y <= 56)) ) //no obj at surface
		y = rand() % 57;  // rund number between 0 - 56 (no objects at surface)
}

bool StudentWorld::isPosValid(int x, int y) const
{
	vector<Actor*>::const_iterator p = vActors.begin();
	while (p != vActors.end())  //look through all actors placed already
	{
		if (getRadius(x, y, (*p)->getX(), (*p)->getY()) <= 6)
			return false;
		p++;
	}
	return true;
}

double StudentWorld::getRadius(int x1, int y1, int x2, int y2) const
{
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

void StudentWorld::findPosInField(int &x, int &y)
{
	do
	{
		x = rand() % 61;   // rand number between 0 - 60
		y = rand() % 57;   // rand number between 0 - 56 (no objects at surface)
	} while (isDirtAt(x, y));  //re-do if there is dirt at position
}

bool StudentWorld::isDirtAt(int x, int y) const
{
	for (int k = 0; k < 4; k++)
		for (int i = 0; i < 4; i++)
			if (dirtArr[x + k][y + i] != nullptr)  //there is dirt
				return true;
	return false;
}

std::string StudentWorld::getSBar() const
{
	/*scr: 321000 Lvl:52 Lives: 4 Hlth: 80% Wtr: 20 Gld:3 Sonar: 1 Oil Left: 2*/
	string txt = "Scr: ";
	txt += formatInt(getScore(), 6, '0');
	txt += "  Lvl: ";
	txt += formatInt(getLevel(), 2, ' ');
	txt += "  Lives: ";
	txt += std::to_string(getLives());
	txt += "  Hlth: "; //3
	txt += formatInt((frackPtr->getHitPts()) * 10, 3, ' ');
	txt += "%  Wtr: ";
	txt += formatInt(frackPtr->getNumWater(), 2, ' ');
	txt += "  Gld: ";
	txt += formatInt(frackPtr->getNumGold(), 2, ' ');
	txt += "  Sonar: ";
	txt += formatInt(frackPtr->getNumSonar(), 2, ' ');
	txt += "  Oil Left: ";
	txt += formatInt(targetOil - numOilFound, 2, ' ');
	return txt;
}

std::string StudentWorld::formatInt(int num, int size, char c) const
{
	string txt = "";
	string score = std::to_string(num);
	for (int k = 0; k < (size - score.size()); k++)
		txt += c;
	txt += score;
	return txt;
}