#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include "predictorTable.h"

using namespace std;

int main(int argc, char* argv[]) {

    /*
    if (argc != 3)
    {
        std::cout << "Error: user must input 3 arguments, ending program now" << endl;
        return -1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];
    */
    std::ios::sync_with_stdio(false);
    //TEMPORARY REPLACE WHEN COMPILING IN UNIX
    string inputFile = "short_trace1.txt";
    string outputFile = "output.txt";
    
    predictorTable* bimodalPredictor = new predictorTable(inputFile);
    bimodalPredictor->fillPredictorTable(); //Fills up the predictor table using the input file name
    bimodalPredictor->checkPredictorTable();

    fstream file;

    file.open(outputFile, ios::out);
    streambuf* fBuf = file.rdbuf();
    cout.rdbuf(fBuf);

    //Always Taken Predictor Results
    std::cout << bimodalPredictor->getAlwaysTaken() << ',' << bimodalPredictor->getTotal() << ';' << endl;

    //Always notTaken Predictor Results
    std::cout << bimodalPredictor->getAlwaysNT() << ',' << bimodalPredictor->getTotal() << ';' << endl;

    for (int x = 0; x < 7; x++)
    {
        std::cout << bimodalPredictor->getPredicted(x) << "," << bimodalPredictor->getTotal() << "; ";
    }
    std::cout << endl;
    for (int y = 7; y < 14; y++)
    {
        std::cout << bimodalPredictor->getPredicted(y) << "," << bimodalPredictor->getTotal() << "; ";
  
    }
    std::cout << endl;
    for (int q = 0; q < 9; q++)
    {
        std::cout << bimodalPredictor->getGSPredicted(q) << "," << bimodalPredictor->getTotal() << "; ";
    }
    std::cout << endl;
    
    std::cout << bimodalPredictor->getSelectorPredicted() << "," << bimodalPredictor->getTotal() << "; " << endl;
    std::cout << bimodalPredictor->getBTBCorrect() << "," << bimodalPredictor->getTotalBTB() << "; " << endl;

    file.close();

    return 0;
}





