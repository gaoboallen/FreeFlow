#ifndef SOLVEFF_H
#define SOLVEFF_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

using std::vector;
using std::pair;
using std::map;

bool conPipe(const vector<vector<int> > &tMap, vector<vector<int> > &tTraj, int curColor, int tRow, int tCol);
bool solveFF(const vector<vector<int> > &tMap, vector<vector<int> > &tTraj, map<int, vector<pair<int, int> > > &tPipe);

#endif // SOLVEFF_H
