#include "solveff.h"

using namespace std;

map<int, vector<pair<int, int> > > sMaps; // map< color, [source, sink] >, color range: [0, color.size()-1]
int scale, numCol;
vector<vector<int> > iTraj;
map<int, vector<pair<int, int> > > pipeMaps;
bool backTracking;


//bool isFF(const vector<vector<int> > &tTraj) {
//    map<int, vector<pair<int, int> > >::iterator it = sMaps.begin();
//    for (; it != sMaps.end(); ++it)
//        if (tTraj[it->second[0].first][it->second[0].second] != it->first || tTraj[it->second[1].first][it->second[1].second] != it->first)
//            return false;
//    return true;
//}

bool conPipe(const vector<vector<int> > &tMap, vector<vector<int> > &tTraj, int curColor, int tRow, int tCol) {
    if (tRow == sMaps[curColor][1].first && tCol == sMaps[curColor][1].second) { // is sink
        if (curColor == numCol-1) {
            pipeMaps[curColor] = vector<pair<int, int> >(1, make_pair(tRow, tCol));
            return true; // all color has been connected
        }
        ++curColor; // else connect next color
        bool tSuc = conPipe(tMap, tTraj, curColor, sMaps[curColor][0].first, sMaps[curColor][0].second);
        if (tSuc) {
            --curColor;
            pipeMaps[curColor] = vector<pair<int, int> >(1, make_pair(tRow, tCol));
        }
        return tSuc;
    }
    tTraj[tRow][tCol] = curColor;
    // left, up, right, down
    if (tCol > 0 && (tTraj[tRow][tCol-1] < 0 ||\
                     (tRow == sMaps[curColor][1].first && tCol-1 == sMaps[curColor][1].second)))
        if (conPipe(tMap, tTraj, curColor, tRow, tCol-1)) {
            pipeMaps[curColor].push_back(make_pair(tRow, tCol));
            return true;
        }
    if (tRow < scale-1 && (tTraj[tRow+1][tCol] < 0 ||\
                     (tRow+1 == sMaps[curColor][1].first && tCol == sMaps[curColor][1].second)))
        if (conPipe(tMap, tTraj, curColor, tRow+1, tCol)) {
            pipeMaps[curColor].push_back(make_pair(tRow, tCol));
            return true;
        }
    if (tCol < scale-1 && (tTraj[tRow][tCol+1] < 0 ||\
                     (tRow == sMaps[curColor][1].first && tCol+1 == sMaps[curColor][1].second)))
        if (conPipe(tMap, tTraj, curColor, tRow, tCol+1)) {
            pipeMaps[curColor].push_back(make_pair(tRow, tCol));
            return true;
        }
    if (tRow > 0 && (tTraj[tRow-1][tCol] < 0 ||\
                     (tRow-1 == sMaps[curColor][1].first && tCol == sMaps[curColor][1].second)))
        if (conPipe(tMap, tTraj, curColor, tRow-1, tCol)) {
            pipeMaps[curColor].push_back(make_pair(tRow, tCol));
            return true;
        }
    // cannot find possible solution
    if (tRow == sMaps[curColor][0].first && tCol == sMaps[curColor][0].second) return false; // is src
    tTraj[tRow][tCol] = -1; // else
    return false;
}

bool solveFF(const vector<vector<int> > &tMap, vector<vector<int> > &tTraj, map<int, vector<pair<int, int> > > &tPipe) {
    backTracking = 0;
    sMaps.clear();
    pipeMaps.clear();
    bool ans;
    scale = tMap.size();

    iTraj = vector<vector<int> >(scale, vector<int>(scale, -1));

    for (int i = 0; i < scale; ++i)
        for (int j = 0; j < scale; ++j)
            if (tMap[i][j] >= 0) {
                if (sMaps.find(tMap[i][j]) != sMaps.end())
                    sMaps[tMap[i][j]].push_back(make_pair(i, j)); // first is row, second is column
                else
                    sMaps[tMap[i][j]] = vector<pair<int, int> >(1, make_pair(i, j));
                iTraj[i][j] = tMap[i][j]; // init src and sink
            }
    numCol = sMaps.size();

    ans = conPipe(tMap, iTraj, 0, sMaps[0][0].first, sMaps[0][0].second);
    if (ans) {
        tTraj = iTraj;
        tPipe = pipeMaps;
    }
    return ans;
}
