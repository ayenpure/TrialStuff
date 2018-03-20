#include <bitset>
#include <iostream>
#include <set>
#include <vector>

int main(int argc, char **argv) {

  /*std::vector<int> cases = {7,   15,  23,  31,  39,  47,  55,  63,
                            71,  79,  87,  95,  103, 111, 119, 127,
                            135, 143, 151, 159, 167, 175, 183, 191,
                            199, 207, 215, 223, 231, 239, 247, 255};
  */

  const int numEdges = 12;
  int edges[numEdges][2] = {{0, 1}, {1, 3}, {2, 3}, {0, 2}, {4, 5}, {5, 7},
                            {6, 7}, {4, 6}, {0, 4}, {1, 5}, {3, 7}, {2, 6}};

  int numCases = 256;
  std::cout << "number of cases to deal with : " << numCases << std::endl;

  std::bitset<8> *casebits = new std::bitset<8>[numCases];
  for (int caseind = 0; caseind < numCases; caseind++) {
    casebits[caseind] = std::bitset<8>(caseind);
    std::cout << "Case " << caseind << " : " << casebits[caseind]
              << std::endl;
  }

  std::set<int> *affectededges = new std::set<int>[numCases];
  for (int caseind = 0; caseind < numCases; caseind++) {
    affectededges[caseind] = std::set<int>();
    for (int edgeind = 0; edgeind < numEdges; edgeind++) {
      int end1 = edges[edgeind][0];
      int end2 = edges[edgeind][1];
      if ((casebits[caseind][end1] == 1 && casebits[caseind][end2] == 0) ||
          (casebits[caseind][end2] == 1 && casebits[caseind][end1] == 0))
        affectededges[caseind].insert(edgeind);
    }
    std::cout << "Case " << caseind << " : "
              << "Number of edges : " << affectededges[caseind].size()
              << std::endl;
  }
  return 0;
}
