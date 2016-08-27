#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <map>
#include <cmath>
#include <fstream>

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QPainter>
#include <QTextStream>
#include <QDebug>
#include <Qpen>
#include <QMouseEvent>
#include <QSound>
#include <QLabel>
#include <QFont>

using std::vector;
using std::pair;
using std::map;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    void drawGrid(QPainter *pt);
    void drawMap(QPainter *pt);
    void drapTraj(QPainter *pt);

    ~MainWindow();
    void mousePressEvent( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );

private slots:
    void on_actionExit_triggered();

    void on_actionLoad_triggered();

//    void on_actionAdd_triggered();

    void on_action5_5_triggered();

    void on_action6_6_triggered();

    void on_action7_7_triggered();

    void on_actionReset_triggered();

    void on_actionPre_level_triggered();

    void on_actionNext_level_triggered();

    void on_actionWithdraw_triggered();

    void on_actionSol_triggered();

    void on_actionHint_triggered();

    void on_action8_8_triggered();

private:
    // return false if (x, y) is out of range
    bool getColRow(int x, int y, int &col, int &row);
    // clean certain color from the trajectory, return false if no changes were made
    bool cleanTraj(int col);
    bool checkSolve();
    void resetTraj();
    void resetPara(); // Must set numRow beforehand
    bool trajChanged(const vector<vector<int> > &v1, const vector<vector<int> > &v2);
    void setTrajForPipe(int col);
    void fileRead();

    Ui::MainWindow *ui;

    // geometry
    int winH, borderH, ballW, pipeW, indcaW;
    //
    int numRow, gapL;
    //
    int mouseX, mouseY;
    int curTrajCol; // -1 means don't draw trajectory
    vector<vector<int> > curTraj;
    vector<vector<vector<int> > > hisTraj;
    vector<map<int, vector<pair<int, int> > > > hisPipe;
    // gameMaps[i][n] is the n-th i*i map
    map<int, vector<vector<vector<int> > > > gameMaps;
    map<int, vector<pair<int, int> > > pipeMaps;
    int curMapN;
    // curMap = gameMaps[numRow][curMapN]
    vector<vector<int> > curMap;
    pair<int,int> srcPos, curPos;
    bool passed;
};

#endif // MAINWINDOW_H
