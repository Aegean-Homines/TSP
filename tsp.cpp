#include "tsp.h"
#include <iostream>
#include <fstream>
#include <limits>  // numeric_limits
#include <chrono>
#include <algorithm>

#define __DEBUG false

typedef std::multimap<int, int> LowerBoundMap;

std::vector<int> SolveTSP(char const * filename)
{
	TSPSolver solver(filename);

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
	solver.SolveTSPRecursively(0);
#endif // _DEBUG

	return solver.Tour();
}

/*
 * map.clear();
    std::ifstream in( filename, std::ifstream::in );
    if( !in.is_open() ) {
        std::cout << "problem reading " << filename << std::endl;
        return;
    }
    in >> TotalCity;
    for( int i = 0; i < TotalCity; ++i ) {
        std::vector<int> row;
        for( int j = 0; j < TotalCity; ++j ) {
            row.push_back( std::numeric_limits<int>::max() );
        }
        map.push_back( row );
    }
    for( int i = 0; i < TotalCity; ++i ) {
        for( int j = i + 1; j < TotalCity; ++j ) {
            if( !in.good() ) {
                std::cout << "problem reading " << filename << std::endl;
                return;
            }
            in >> map[i][j];
            map[j][i] = map[i][j];
        }
    }
 */

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
	currentPath(std::vector<int>()),
	bestPath(std::vector<int>()),
	isVisited(std::vector<bool>()){

	read(filename);
	bestPath.resize(totalCity+1);
	isVisited.resize(totalCity, false);

	isVisited[0] = true;
	currentPath.push_back(0);
}

void TSPSolver::SolveTSPRecursively(int currentNodeIndex)
{

	// Termination
	if (currentPath.size() == totalCity) {
		totalCostSoFar += actualCostMap[currentNodeIndex][0];
		if(bestCost > totalCostSoFar) {
			currentPath.push_back(0);
			bestCost = totalCostSoFar;
			bestPath = currentPath;
			currentPath.pop_back();
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
			currentPath.push_back(i);
			totalCostSoFar += actualCostMap[currentNodeIndex][i];
			isVisited[i] = true;
			// Calculate
			int lowerBound = CalculateLowerBound();
			orderedByLowerBound.insert(std::pair<int, int>(lowerBound, i));
			// Reset
			currentPath.pop_back();
			totalCostSoFar -= actualCostMap[currentNodeIndex][i];
			isVisited[i] = false;
		}
	}

	LowerBoundMap::const_iterator iter = orderedByLowerBound.begin();
	LowerBoundMap::const_iterator end = orderedByLowerBound.end();

	while (iter != end) {
		
		currentPath.push_back(iter->second);
		totalCostSoFar += actualCostMap[currentNodeIndex][iter->second];
		isVisited[iter->second] = true;

		if(iter->first < bestCost) {
			SolveTSPRecursively(iter->second);
		}

		currentPath.pop_back();
		totalCostSoFar -= actualCostMap[currentNodeIndex][iter->second];
		isVisited[iter->second] = false;

		++iter;

	}

}

int TSPSolver::CalculateLowerBound()
{
	int lowerBound = totalCostSoFar;

	// cheapest for first node
	//lowerBound += map[0].begin()->first;
	isVisited[0] = false;
	for (unsigned int i = 0; i < totalCity; ++i) {
		if(!isVisited[i]) {
			CostToIndexMap const & costTable = map[i];
			int minimumPathCost = std::numeric_limits<int>::max();
			CostToIndexMap::const_iterator iter = costTable.begin();

			while(iter != costTable.end()) {
				if(!isVisited[iter->second] && minimumPathCost > iter->first) {
					minimumPathCost = iter->first;
					break;
				}
				++iter;
			}

			lowerBound += minimumPathCost;
		}
	}
	isVisited[0] = true;

	return lowerBound;
}

std::vector<int> const & TSPSolver::Tour() const
{
	return bestPath;
}
