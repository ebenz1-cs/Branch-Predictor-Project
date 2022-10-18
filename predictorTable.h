#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<stdlib.h>
#include <cmath>
#include <vector>


using namespace std;

class predictorEntry;

class predictorTable {

public:

	//Constructors
	predictorTable();
	predictorTable(string fileNameThis);
	friend class predictorEntry;


	//Methods;
	void fillPredictorTable();
	unsigned int getPredicted(int index);
	unsigned int getGSPredicted(int index);
	unsigned int getSelectorPredicted();
	unsigned int getBTBCorrect();
	unsigned int getTotalBTB();
	unsigned int getAlwaysTaken();
	unsigned int getAlwaysNT();
	unsigned int getTotal();
	void checkPredictorTable();
	void insert(unsigned long long addr, unsigned long long target, int tableSize, int tableCounter);
	
	//Variables
	//int tableValue[7]{};
	int GHR[9]{};
	int GHRSIZE[9]{};
	unsigned int GSCorrect[9]{};
	unsigned int selectorCorrect;
	unsigned int totalBranches;
	unsigned int takenCount[14]{};
	unsigned int notTakenCount[14]{};
	unsigned int alwaysTaken;
	unsigned int alwaysNT;
	unsigned int BTBCorrect;
	unsigned int totalBTB;
	
	
	predictorEntry** tablePointer;
	string fileName;
};

class predictorEntry
{
	unsigned long long addr;
	unsigned long long target;
	int predictorStateOne;
	int predictorStateTwo;
	int selectorState;
	int GSPredictorState[9]{};
	friend class predictorTable;

	predictorEntry();
};
