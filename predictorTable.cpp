#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<stdlib.h>
#include "predictorTable.h"
#include <cmath>
#include <map>
#include <vector>


predictorTable::predictorTable()
{
	fileName = "";
	int counterArray = 0;
	int tableSize = 16;
	tablePointer = new predictorEntry * [7];

	while (counterArray < 7)
	{
		tablePointer[counterArray] = new predictorEntry[tableSize];
		tableSize *= 2;
		if (tableSize == 64)
			tableSize = 128;
		++counterArray;
	}
	totalBranches = 0;
}

predictorTable::predictorTable(string fileNameThis)
{
	fileName = fileNameThis;

	int counterArray = 0;
	int tableSize = 16;
	tablePointer = new predictorEntry * [7];
	while (counterArray < 7)
	{
		tablePointer[counterArray] = new predictorEntry[tableSize];
		tableSize *= 2;
		if (tableSize == 64)
			tableSize = 128;
		++counterArray;
	}

	counterArray = 3;
	for (int i = 0; i < 9; i++)
	{
		GHR[i] = 0;
		GHRSIZE[i] = counterArray;
		GSCorrect[i] = 0;
		counterArray++;
	}

	counterArray = 0;
	while (counterArray < 14)
	{
		takenCount[counterArray] = 0;
		notTakenCount[counterArray] = 0;
		++counterArray;
	}
	selectorCorrect = 0;
	alwaysTaken = 0;
	alwaysNT = 0;
	BTBCorrect = 0;
	totalBTB = 0;
	totalBranches = 0;
	std::ios::sync_with_stdio(false); //Possible I/O performance increase
}

predictorEntry::predictorEntry() {
	addr = 0;
	target = 0;
	predictorStateOne = 2;		//1-BIT COUNTER: 1 = NT 2 = T
	predictorStateTwo = 4;		//2-BIT COUNTER: 1 = Strong NT 2 = NT 3 = T 4 = Strong T
	selectorState = 1;			//2-BIT COUNTER: 1 = Strong Pref GSHARE 2 = Pref GSHARE 3 = Pref Bimodal 4 = Strong Pref Bimodal

	for (int i = 0; i < 9; i++)
	{
		GSPredictorState[i] = 4;
	}
	
}


void predictorTable::fillPredictorTable()
{
	unsigned long long addr;
	unsigned long long target;
	string behavior, line;
	ifstream infile(fileName);
	int loopCounter = 0;
	int x[7] = {16, 32, 128, 256, 512, 1024, 2048};
	int tableCounter = 0;
	int tableSize = 0;
	while (getline(infile, line) && loopCounter < 2048)
	{
		stringstream s(line);
		s >> std::hex >> addr >> behavior >> std::hex >> target;
		
		while (tableCounter < 7)
		{
			tableSize = x[tableCounter];

			if ((loopCounter < tableSize))
			{
				insert(addr, target, tableSize, tableCounter);
			}
			tableCounter++;
		}
		loopCounter++;
		tableCounter = 0;
	}

}


//THE MAIN PROCESSING FUNCTION USED
void predictorTable::checkPredictorTable()
{
	unsigned long long addr;
	unsigned long long targetAddr;
	ifstream infile(fileName);
	string behavior, line;

	predictorEntry* tableArray;
	unsigned int entryIndex = 0, size = 0, x = 0;
	int tableValue[7] = { 16, 32, 128, 256, 512, 1024, 2048 };
	
	int bimodalMispredict = 0;

	while (infile >> std::hex >> addr >> behavior >> std::hex >> targetAddr)
	{
		x = 0;
		predictorEntry* branchInstruction;
		while (x < 7)
		{
			bimodalMispredict = 0;

			tableArray = tablePointer[x];
			size = tableValue[x];
			entryIndex = addr % size;
			branchInstruction = &tableArray[entryIndex]; 
		
			
			if (x == 4) //BTB LOGIC
			{
				if (branchInstruction->target == targetAddr && branchInstruction -> predictorStateOne == 2)
					BTBCorrect++;		//CANT FIGURE OUT WHY CORRECT BTB PREDICTIONS WONT MATCH CORRECT OUTPUT WHEN EVERYTHING ELSE MATCHES

				if (branchInstruction->predictorStateOne == 2)
					totalBTB++;

				if (behavior == "T" && branchInstruction->target != targetAddr)
					branchInstruction->target = targetAddr;
			}
			

			if (behavior == "T") {
				if (x == 0) //Always taken only needs to run once
					++alwaysTaken;

				//BIMODAL 1-bit and 2-bit LOGIC
				if (branchInstruction->predictorStateOne == 2)
					++takenCount[x];

				else
					branchInstruction->predictorStateOne = 2;

				
				if (branchInstruction->predictorStateTwo > 2)
				{
					if (branchInstruction->predictorStateTwo == 3)
						branchInstruction->predictorStateTwo = 4;
						
					++takenCount[x+7]; //First seven index of takenCount are for the 1-bit bimodal, the next 7 are for 2-bit
				}
				else
				{
					bimodalMispredict = 1;
					++branchInstruction->predictorStateTwo;
				}
				
			}

			else {
				if (x == 0)
					++alwaysNT;
		
				//BIMODAL 1-bit and 2-bit LOGIC
				if (branchInstruction->predictorStateOne == 1)
					++notTakenCount[x];
				else
					branchInstruction->predictorStateOne = 1;
				

				if (branchInstruction->predictorStateTwo <= 2)
				{
					if (branchInstruction->predictorStateTwo == 2)
						branchInstruction->predictorStateTwo = 1;

					++notTakenCount[x+7];
				}
				else
				{
					bimodalMispredict = 1;
					--branchInstruction->predictorStateTwo;
				}
			}
		
			if (x == 6) //GSHARE LOGIC for size 2048 entries
			{
				int historySize = 0;
				int bitMask = 0;
				int isTaken = 0;
				int GSMispredict = 0;
				int GSState = 0;
				predictorEntry* branchInstruction1;
				

				for (int i = 0; i < 9; i++) //Incrementing the GHR bits by 1 found in GHRSIZE[i]
				{

					entryIndex = (addr ^ GHR[i]) %size;		//PC XOR GHR MOD TABLESIZE

					branchInstruction1 = &tableArray[entryIndex];
					GSState = branchInstruction1->GSPredictorState[i];

					historySize = GHRSIZE[i];
					bitMask = pow(2, historySize) - 1;		//Use a bit mask to choose with GHR bits to keep 
					GSMispredict = 0;						//Helper Var for selector Logic later

					if (behavior == "T")
					{
						isTaken = 1;
						if (GSState > 2) //Predict Taken
						{
							if (GSState == 3)
								branchInstruction1->GSPredictorState[i] = 4;

							++GSCorrect[i];
						}
						else //Predict Not Taken
						{
								GSMispredict = 1;
							++branchInstruction1->GSPredictorState[i];
						}

					}
					else if (behavior == "NT")
					{
						isTaken = 0;
						if (GSState <= 2) //Predict Taken
						{
							if (GSState == 2)
								branchInstruction1->GSPredictorState[i] = 1;

							++GSCorrect[i];
						}
						else //Predict Not Taken
						{
								GSMispredict = 1;
							--branchInstruction1->GSPredictorState[i];
						}

					}

					GHR[i] = GHR[i] << 1;	//Shift left to make space for most recent branch taken or not taken history

					if(isTaken>0)				//If taken add 1
						GHR[i] = GHR[i] + 1; 

 					GHR[i] = GHR[i] & bitMask;  //Since we're shifting left we want to get rid of nums outside of our size by using a bit mask
					

					//SELECTOR LOGIC 
					if (i == 8)		//When i == 8 GHR will have a size of 11 bits SELECTOR LOGIC
					{
						if (branchInstruction->selectorState > 2) //Prefer bimodal predictor
						{
							if (GSMispredict == 0 && bimodalMispredict == 0)
								++selectorCorrect;

							else if (GSMispredict == 1 && bimodalMispredict == 0)
							{
								if (branchInstruction->selectorState == 3)
									branchInstruction->selectorState = 4; //Move to strongly prefer bimodal predictor

								++selectorCorrect;

							}
							else if (GSMispredict == 0 && bimodalMispredict == 1)
							{
								--branchInstruction->selectorState; //decrement towards prefering GSHARE
							}
							//if both mispredict do nothing
						}
						else if (branchInstruction->selectorState <= 2) //Prefer GSHARE predictor
						{
							if (GSMispredict == 0 && bimodalMispredict == 0)
								++selectorCorrect;
							else if (GSMispredict == 0 && bimodalMispredict == 1)
							{
								if (branchInstruction->selectorState == 2)
									branchInstruction->selectorState = 1; //Move to strongly prefer GSHARE predictor

								++selectorCorrect;
							}
							else if (GSMispredict == 1 && bimodalMispredict == 0)
							{
								++branchInstruction->selectorState; //Increment towards prefering bimodal predictor
							}
							//If both mispredict do nothing
						}
					}
				}
			}
			++x;
		}
		++totalBranches;
	}
}

void predictorTable::insert(unsigned long long address, unsigned long long targetThis, int tableSize, int tableCounter)
{
	predictorEntry* branchInstructionI = new predictorEntry();
	predictorEntry* tableArray;
	branchInstructionI->addr = address;
	branchInstructionI->target = targetThis;
	
	
	unsigned long long index = address % tableSize;
	tableArray = tablePointer[tableCounter];
	tableArray[index] = *branchInstructionI;


}
	

//GETTER FUNCTIONS
unsigned int predictorTable::getPredicted(int index)
{
	return takenCount[index] + notTakenCount[index];
}

unsigned int predictorTable::getGSPredicted(int index)
{
	return GSCorrect[index];
}

unsigned int predictorTable::getSelectorPredicted()
{
	return selectorCorrect;
}

unsigned int predictorTable::getBTBCorrect()
{
	return BTBCorrect;
}

unsigned int predictorTable::getTotalBTB()
{
	return totalBTB;
}

unsigned int predictorTable::getAlwaysTaken()
{
	return alwaysTaken;
}

unsigned int predictorTable::getAlwaysNT()
{
	return alwaysNT;
}

unsigned int predictorTable::getTotal()
{
	return totalBranches;
}





