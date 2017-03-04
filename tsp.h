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
	void read(char const* filename);
	TSPSolver(int totalCity = 0);
	void SolveTSPRecursively(int currentNodeIndex);
	void CalculateLowerBound();

	std::vector<int> const & Tour();

private:
	unsigned int totalCity;
	int bestCost;
	int totalCostSoFar;

	RecursionTreeRepresentation map;
	IndexSet visitedNodes;
	std::vector<int> bestPath;
};
#endif
