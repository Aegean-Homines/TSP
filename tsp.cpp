/*!
* \file tsp.cpp
* \author Egemen Koku
* \date 12 Mar 2017
* \brief Implementation of @b tsp.h
*
* \copyright Digipen Institute of Technology
* \mainpage TSP
*
*/

#include "tsp.h"
#include <iostream>
#include <fstream>
#include <limits>  // numeric_limits

#define __DEBUG false

// LowerBoundMap definition for storing LowerBound values at each recursion level
typedef std::multimap<int, int> LowerBoundMap;

std::vector<int> SolveTSP(char const * filename)
{
	TSPSolver solver(filename);
	solver.SolveTSPRecursively(0); //recursive function starts with city 0
	std::vector<int> bestPath;
	solver.GetBestPath(bestPath);
	return bestPath;
}

void TSPSolver::read(char const * filename)
{
	map.clear();
	actualCostMap.clear();
	std::ifstream in(filename, std::ifstream::in);
	if (!in.is_open()) {
		std::cout << "problem reading " << filename << std::endl;
		return;
	}
	in >> totalCity;
	map.resize(totalCity);

	for (unsigned int i = 0; i < totalCity; ++i) {
		std::vector<int> row;
		for (unsigned int j = 0; j < totalCity; ++j) {
			row.push_back(std::numeric_limits<int>::max());
		}
		actualCostMap.push_back(row);
	}

	for (unsigned int i = 0; i < totalCity; ++i) {
		CostToIndexMap & orderedMap = map[i];
		orderedMap.insert(std::pair<int, int>(std::numeric_limits<int>::max(), i));
		for (unsigned int j = i + 1; j < totalCity; ++j) {
			if (!in.good()) {
				std::cout << "problem reading " << filename << std::endl;
				return;
			}
			int cost;
			in >> cost;

			// the above loop is the same with the one in the driver
			// this is the additional part: I'm storing map the way driver function does
			// but I'm also storing them in <cost, index> order to my map for possibly faster lookup later on
			orderedMap.insert(std::pair<int, int>(cost, j));
			CostToIndexMap & symmetricSide = map[j];
			symmetricSide.insert(std::pair<int, int>(cost, i));

			actualCostMap[i][j] = cost;
			actualCostMap[j][i] = actualCostMap[i][j];
		}
	}

	in.close();

}

TSPSolver::TSPSolver(char const* filename)
	: totalCity(0),
	bestCost(std::numeric_limits<int>::max()),
	totalCostSoFar(0),
	actualCostMap(MAP()),
	map(RecursionTreeRepresentation()),
	currentPath(std::stack<int>()),
	bestPath(std::stack<int>()),
	isVisited(std::vector<bool>()){

	read(filename);
	//bestPath.resize(totalCity+1);
	isVisited.resize(totalCity, false);

	// First city is already in the list
	isVisited[0] = true;
	currentPath.push(0);
}

void TSPSolver::SolveTSPRecursively(int currentNodeIndex)
{

	// Termination
	if (currentPath.size() == totalCity) {
		// It goes into this termination step only when all the cities are in (except for the first city)
		// So I'm adding the first city as well and then store the path in that form
		totalCostSoFar += actualCostMap[currentNodeIndex][0];
		if(bestCost > totalCostSoFar) {
			currentPath.push(0); 
			bestCost = totalCostSoFar;
			bestPath = currentPath;
			currentPath.pop();
		}
		totalCostSoFar -= actualCostMap[currentNodeIndex][0];
		return;
	}

	// Lower bound calculation
	// pair<lowerBound, nodeIndex>
	LowerBoundMap orderedByLowerBound;

	for (unsigned int i = 0; i < totalCity; ++i) {
		if(!isVisited[i]) {
			// Set
			currentPath.push(i);
			totalCostSoFar += actualCostMap[currentNodeIndex][i];
			isVisited[i] = true;
			// Calculate
			int lowerBound = CalculateLowerBound();
			orderedByLowerBound.insert(std::pair<int, int>(lowerBound, i));
			// Reset
			currentPath.pop();
			totalCostSoFar -= actualCostMap[currentNodeIndex][i];
			isVisited[i] = false;
		}
	}

	LowerBoundMap::const_iterator iter = orderedByLowerBound.begin();
	LowerBoundMap::const_iterator end = orderedByLowerBound.end();

	// Go through the lowerBounds in the ascending order
	while (iter != end) {
		
		// Set
		currentPath.push(iter->second);
		totalCostSoFar += actualCostMap[currentNodeIndex][iter->second];
		isVisited[iter->second] = true;

		// Recurse
		if(iter->first < bestCost) {
			SolveTSPRecursively(iter->second);
		}

		// Reset
		currentPath.pop();
		totalCostSoFar -= actualCostMap[currentNodeIndex][iter->second];
		isVisited[iter->second] = false;

		++iter;

	}

}

int TSPSolver::CalculateLowerBound()
{
	int lowerBound = totalCostSoFar;

	// cheapest for first node
	// Hack: At lower bound calculation point, it should also consider the first city
	// However, it is already set as visited (because we started at this point) so I'm setting this to false for
	// bound calculation.
	isVisited[0] = false;
	for (unsigned int i = 0; i < totalCity; ++i) {
		if(!isVisited[i]) {
			CostToIndexMap const & costTable = map[i];
			int minimumPathCost = std::numeric_limits<int>::max();
			CostToIndexMap::const_iterator iter = costTable.begin();

			while(iter != costTable.end()) {
				if(!isVisited[iter->second] && minimumPathCost > iter->first) {
					// This is where I'm using my cost table map
					// The first one that is not visited is guaranteed to be the one with the lowest cost
					// so I can just break out of this loop
					minimumPathCost = iter->first;
					break;
				}
				++iter;
			}

			lowerBound += minimumPathCost;
		}
	}
	// I reset the state of the first node
	isVisited[0] = true;

	return lowerBound;
}

void TSPSolver::GetBestPath(std::vector<int> & pathToFill)
{
	while (!bestPath.empty()) {
		pathToFill.push_back(bestPath.top());
		bestPath.pop();
	}
}
