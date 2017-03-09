#ifndef TSP_H
#define TSP_H

#include <vector>
#include <unordered_set>
#include <map>

typedef std::vector< std::vector<int> > MAP;
typedef std::unordered_set<int> IndexSet;
typedef std::multimap<int, int> CostToIndexMap; //for ordering the tree branches
typedef std::vector<CostToIndexMap> RecursionTreeRepresentation;

std::vector<int> SolveTSP(char const* filename);

class TSPSolver {
public:
	TSPSolver(char const* filename);
	void SolveTSPRecursively(int currentNodeIndex);

	std::vector<int> const & Tour() const;

private:
	unsigned int totalCity;
	int bestCost;
	int totalCostSoFar;

	MAP actualCostMap;
	RecursionTreeRepresentation map;
	std::vector<int> currentPath;
	std::vector<int> bestPath;
	std::vector<bool> isVisited;

	int CalculateLowerBound();
	void read(char const* filename);

};
#endif
