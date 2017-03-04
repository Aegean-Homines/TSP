#include "tsp.h"
#include <iostream>
#include <fstream>
#include <limits>  // numeric_limits

std::vector<int> SolveTSP(char const * filename)
{
	std::vector<int> result;
	TSPSolver solver;
	solver.read(filename);
	solver.CalculateLowerBound();
	solver.SolveTSPRecursively(0);

	return solver.Tour();
}

void TSPSolver::read(char const * filename)
{
	map.clear();
	std::ifstream in(filename, std::ifstream::in);
	if (!in.is_open()) {
		std::cout << "problem reading " << filename << std::endl;
		return;
	}
	in >> totalCity;
	map.resize(totalCity);
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

			orderedMap.insert(std::pair<int, int>(cost, j));
			CostToIndexMap & symmetricSide = map[j];
			symmetricSide.insert(std::pair<int, int>(cost, i));
		}
	}

}

TSPSolver::TSPSolver(int totalCity) 
	: totalCity(totalCity), bestCost(std::numeric_limits<int>::max()), totalCostSoFar(0) {
	
	// +1 for storing the first node twice
	bestPath.resize(totalCity+1);
}

void TSPSolver::SolveTSPRecursively(int currentNodeIndex)
{
	visitedNodes.insert(currentNodeIndex);
	CostToIndexMap const & children = map[currentNodeIndex];

	CostToIndexMap::const_iterator iter = children.begin();
	CostToIndexMap::const_iterator end = children.end();
	--end; //we're not adding the last element
		   // since children are orderd, max limit is the last element
		   // hence it's not this nodes child
	while(iter != end) {

		// If the child is the start node and if I visited all the nodes -> possible solution
		// Base case
		if(0 == iter->second && totalCity == visitedNodes.size()) {
			if(bestCost > totalCostSoFar + iter->first) {
				bestCost = totalCostSoFar + iter->first;
				// This is the best path -> take a snapshot
				std::copy(visitedNodes.begin(), visitedNodes.end(), bestPath.begin());
			}else {
				break;
			}

			return;
		}

		// Recursion part
		// If this child is not visited before
		if (visitedNodes.find(iter->second) == visitedNodes.end()) {
			totalCostSoFar += iter->first;
			// Branch and bound
			if (totalCostSoFar < bestCost) { 
				SolveTSPRecursively(iter->second);
				totalCostSoFar -= iter->first;
			}
			else {
				totalCostSoFar -= iter->first;
				break;
			}

		}


		++iter;
	}

	visitedNodes.erase(currentNodeIndex);

}

void TSPSolver::CalculateLowerBound()
{
	bestCost = 0;
	CostToIndexMap::const_iterator iter;
	CostToIndexMap::const_iterator end;

	int index = 0;

	do {
		visitedNodes.insert(index);
		CostToIndexMap const & indexMap = map[index];
		iter = indexMap.begin();
		end = indexMap.end();
		--end;
		while (iter != end) {

			if (visitedNodes.find(iter->second) == visitedNodes.end()) {
				index = iter->second;
				bestCost += iter->first;
				break;
			}
			
			if(0 == iter->second && visitedNodes.size() == totalCity) {
				index = 0;
				bestCost += iter->first;
				break;
			}

			++iter;
		}
		bestPath.push_back(index);
	} while (index != 0);

	visitedNodes.clear();
}

std::vector<int> const & TSPSolver::Tour()
{
	return bestPath;
}
