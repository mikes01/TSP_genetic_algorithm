#pragma once

#include "TravelingSalesmanProblem.h"
#include "GeneticAlgorithm.h"

class Menu
{
private:

	GeneticAlgorithm sa;
	TravelingSalesmanProblem tsp;
	void readCitiesMatrixFromKeyboard();
	void readCitiesMatrixFromFile();
	void printCitiesMatrix();
	void startAlgorithm();
	//void startAlgorithmStepByStep();
	void startMeasurements();
	void measureItr();
	void measureDelta();
	void measureFinalTemp();
	void measureSize();
	
public:
	
	Menu();
	~Menu();
	void start();
	void startAlgorithmStepByStep();
};

