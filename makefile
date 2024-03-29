FLAGS = -Wall -Wextra -g
EXECUTABLE = predictors

all:	predictor

predictor:	predictor.o predictorTable.o
	g++ predictor.o predictorTable.o -o predictors
	
predictor.o:	predictorTable.h predictor.cpp
	g++ -c $(FLAGS) predictor.cpp -o predictor.o

predictorTable.o:	predictorTable.h predictorTable.cpp
	g++ -c $(FLAGS) predictorTable.cpp -o predictorTable.o


clean:	
	rm -f *.o $(EXECUTABLE)

	
