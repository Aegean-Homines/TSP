#include "tsp.h"
#include <iostream>
#include <fstream>
#include <limits>  // numeric_limits
#include <chrono>

std::vector<int> SolveTSP(char const * filename)
{
	std::vector<int> result;
	TSPSolver solver;
	solver.read(filename);
	solver.CalculateLowerBound();
	solver.SolveTSPRecursively(0);

#ifdef _DEBUG
	auto begin = std::chrono::system_clock::now();
	solver.CalculateLowerBound();
	auto end = std::chrono::system_clock::now();
	auto dif = end - begin;
	std::cout << "Calculate Lower Bound: " << dif.count() << std::endl;
	begin = std::chrono::system_clock::now();
	solver.SolveTSPRecursively(0);
	end = std::chrono::system_clock::now();
	dif = end - begin;
	std::cout << "SolveTSP: " << dif.count() << std::endl;
#endif // _DEBUG

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

	in.close();

}

TSPSolver::TSPSolver(int totalCity) 
	: totalCity(totalCity), 
	bestCost(std::numeric_limits<int>::max()), 
	totalCostSoFar(0),
	map(RecursionTreeRepresentation()),
	visitedNodes(IndexSet()),
	currentPath(std::vector<int>()),
	bestPath(std::vector<int>(totalCity+1)){ }

void TSPSolver::SolveTSPRecursively(int currentNodeIndex)
{
	visitedNodes.insert(currentNodeIndex);
	CostToIndexMap const & children = map[currentNodeIndex];

	// Base case
	if (0 == currentNodeIndex && totalCity == visitedNodes.size()) {
		if (bestCost > totalCostSoFar) {
			bestCost = totalCostSoFar;
			/*std::cout << "Visited nodes at best cost" << std::endl;
			for(auto i: visitedNodes) {
				std::cout << i << std::endl;
			}
			std::cout << "--------------------------" << std::endl;*/

			// This is the best path -> take a snapshot
			std::copy(visitedNodes.begin(), visitedNodes.end(), bestPath.begin());
		}

		return;
	}

	CostToIndexMap::const_iterator iter = children.begin();
	CostToIndexMap::const_iterator end = children.end();
	--end; //we're not adding the last element
		   // since children are orderd, max limit is the last element
		   // hence it's not this nodes child
	while(iter != end) {

		// Recursion part
		// If this child is not visited before
		if (visitedNodes.find(iter->second) == visitedNodes.end()
			|| (0 == iter->second && totalCity == visitedNodes.size())) {
			// Branch and bound
			if (totalCostSoFar + iter->first < bestCost) { 
				totalCostSoFar += iter->first;
				SolveTSPRecursively(iter->second);
				totalCostSoFar -= iter->first;
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

std::vector<int> const & TSPSolver::Tour() const
{
	return bestPath;
}
