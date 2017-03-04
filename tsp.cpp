#include "tsp.h"
#include <iostream>
#include <fstream>
#include <limits>  // numeric_limits
#include <chrono>
#include <algorithm>

#define __DEBUG false

std::vector<int> SolveTSP(char const * filename)
{
	std::vector<int> result;
	TSPSolver solver;
	solver.read(filename);

#if __DEBUG
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
#else
	solver.CalculateLowerBound();
	solver.SolveTSPRecursively(0);
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
	currentPath(std::vector<int>()),
	bestPath(std::vector<int>(totalCity + 1)) {

}

void TSPSolver::SolveTSPRecursively(int currentNodeIndex)
{
	CostToIndexMap const & children = map[currentNodeIndex];

	// Termination
	if (currentNodeIndex == 0 && currentPath.size() == totalCity && bestCost > totalCostSoFar) {
		bestCost = totalCostSoFar;
		std::copy(currentPath.begin(), currentPath.end(), bestPath.begin() + 1);
	}

	CostToIndexMap::const_iterator iter;
	CostToIndexMap::const_iterator end;

	iter = children.begin();
	end = children.end();
	--end; // will be self

	while (iter != end) {
		if (std::find(currentPath.begin(), currentPath.end(), iter->second) != currentPath.end()) {
			++iter;
			continue;
		}

		currentPath.push_back(iter->second);
		totalCostSoFar += iter->first;
		if (totalCostSoFar < bestCost) {
			SolveTSPRecursively(iter->second);
		}
		currentPath.pop_back();
		totalCostSoFar -= iter->first;

		++iter;

	}

}

void TSPSolver::CalculateLowerBound()
{
	bestCost = 0;
	CostToIndexMap::const_iterator iter;
	CostToIndexMap::const_iterator end;

	int index = 0;

	do {
		currentPath.push_back(index);
		CostToIndexMap const & indexMap = map[index];
		iter = indexMap.begin();
		end = indexMap.end();
		--end;
		while (iter != end) {

			if (std::find(currentPath.begin(), currentPath.end(), iter->second) == currentPath.end()) {
				index = iter->second;
				bestCost += iter->first;
				break;
			}

			if (0 == iter->second && currentPath.size() == totalCity) {
				index = 0;
				bestCost += iter->first;
				break;
			}

			++iter;
		}
		bestPath.push_back(index);
	} while (index != 0);

	currentPath.clear();
}

std::vector<int> const & TSPSolver::Tour() const
{
	return bestPath;
}
