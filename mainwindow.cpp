/* Author: Bo Gao
 * 2015/8/28
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "solveff.h"

using namespace std;

#define COLOR_NUM 7
static QColor color[COLOR_NUM] = {Qt::red, Qt::yellow, Qt::green, Qt::blue, Qt::white, Qt::cyan, Qt::magenta};
static QColor semitranC[COLOR_NUM] = {Qt::red, Qt::yellow, Qt::green, Qt::blue, Qt::white, Qt::cyan, Qt::magenta};

QString stepqs;

static int map1[5][5] = {
    {0, -1, -1, -1, 0},
    {-1, -1, -1, -1, -1},
    {-1, 1, 2, -1, 3},
    {-1, -1, -1, -1, 2},
    {-1, 3, -1, -1, 1}
};
map<int, vector<vector<int> > > initTraj;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    passed = false;

    for (int i = 0; i < COLOR_NUM; ++i) semitranC[i].setAlpha(127);
    for (int i = 0; i < 5; ++i) {
        curMap.push_back(vector<int>(map1[i], map1[i]+5));
    }
    for (int i = 5; i <= 8; ++i)
        initTraj[i] = vector<vector<int> >(i, vector<int>(i, -1));
    curTraj = initTraj[5];
    hisTraj.push_back(curTraj);
//    menuH = this->menuBar()->height();
    borderH = 100;
    winH = 630;

    numRow = 5;
    resetPara();
    for (int i = 5; i <= 8; ++i)
        gameMaps[i] = vector<vector<vector<int> > >();
    gameMaps[5].push_back(curMap);
    curMapN = 0;

    fileRead(); // this line will read mapinfo from ./gameMaps

    curTrajCol = -1;

    this->setFixedSize(winH+2*borderH, winH+2*borderH);
    this->setStyleSheet("QMainWindow {background: 'black';}");    

    QFont f("Arial", 18, QFont::Bold);
    stepqs = QString("Step: ");
    ui->msgLabel->setFont(f);
    ui->stepLabel->setFont(f);
    ui->msgLabel->setGeometry(borderH+50, 20, 330, 30);
    ui->msgLabel->setStyleSheet("QLabel { background-color : white; color : black; }");
    ui->msgLabel->setText("This is a msg");
    ui->stepLabel->setGeometry(borderH+winH-50-100, 20, 100, 30);
    ui->stepLabel->setStyleSheet("QLabel { background-color : white; color : black; }");
    ui->stepLabel->setText(stepqs+QString::number(0));
//    ui->stepLabel->show();
}

void MainWindow::mousePressEvent( QMouseEvent * e ) {
    mouseX = e->x();
    mouseY = e->y();

    int tx, ty;
    if (!getColRow(mouseX, mouseY, tx, ty)) {
        qDebug()<<"Out of range.";
        ui->msgLabel->setText("Out of range");
        return;
    }

    if(e->button()==Qt::LeftButton) {
        if (curMap[ty][tx] >= 0) {
            cleanTraj(curMap[ty][tx]);
            curTrajCol = curMap[ty][tx];
            curTraj[ty][tx] = curMap[ty][tx];
            srcPos = make_pair(ty, tx);
            curPos = make_pair(ty, tx);
            pipeMaps[curTrajCol] = vector<pair<int, int> >(1, srcPos);
        } else {
            qDebug() << "Must start from a colored point.";
            ui->msgLabel->setText("Start from a colored point");
        }
    } else if (e->button()==Qt::RightButton) {
        qDebug() << "Left button only.";
        ui->msgLabel->setText("Left button only");
    }
    update();
}

void MainWindow::mouseReleaseEvent( QMouseEvent * e) {
    if(e->button()==Qt::LeftButton) {
        mouseX = e->x();
        mouseY = e->y();
        int tx, ty;
        if (!getColRow(mouseX, mouseY, tx, ty)) {
            qDebug()<<"Out of range.";
            ui->msgLabel->setText("Out of range");
            if (curTrajCol >= 0 && checkSolve()) {
                QSound::play("./sounds/complete.wav");
                qDebug() << "Free Flow completed!";
                ui->msgLabel->setText("Free Flow completed!");
                passed = 1;
            }
            if (trajChanged(hisTraj.back(), curTraj)) {
                hisTraj.push_back(curTraj);
                hisPipe.push_back(pipeMaps);
            }
            curTrajCol = -1;
            update();
            return;
        }
        if (curTrajCol >= 0 && (curMap[ty][tx] >= 0 && curMap[ty][tx] == curTrajCol)\
                && (ty!=srcPos.first || tx!=srcPos.second)) {
            // found dst
            curTraj[ty][tx] = curTrajCol;
            if (checkSolve()) {
                QSound::play("./sounds/complete.wav");
                qDebug() << "Free Flow completed!";
                ui->msgLabel->setText("Free Flow completed!");
                passed = 1;
            } else {
                QSound::play("./sounds/piped.wav");
                qDebug() << "Another pipe connected!";
                ui->msgLabel->setText("Another pipe connected!");
            }
        } else if (curTraj[ty][tx] >= 0 && curTraj[ty][tx] != curTrajCol) {
            QSound::play("./sounds/conflict.wav");
            qDebug() << "Pipe conflict.";
            ui->msgLabel->setText("Pipe conflict");
        }
        if (trajChanged(hisTraj.back(), curTraj)) {
            hisTraj.push_back(curTraj);
            hisPipe.push_back(pipeMaps);
        }
        curTrajCol = -1;
    } else if (e->button()==Qt::RightButton) {
        qDebug() << "Left button only";
        ui->msgLabel->setText("Left button only");
    }
    update();
}

void MainWindow::mouseMoveEvent ( QMouseEvent * e ) {
    if (curTrajCol >= 0) {
        mouseX = e->x();
        mouseY = e->y();
        int tx, ty;
        if (!getColRow(mouseX, mouseY, tx, ty)) {
            return;
        } if (curTraj[ty][tx] == curTrajCol && ty==curPos.first && tx==curPos.second) {
            // no change
        } else if ((curMap[ty][tx] == curTrajCol && (ty!=srcPos.first || tx!=srcPos.second)) // found dst, but not confirmed until mouse released
                   || (curMap[ty][tx] < 0 && curTraj[ty][tx] < 0)) { // new grid explored
            // check for missed grids
            int tPrey = pipeMaps[curTrajCol].back().first, tPrex = pipeMaps[curTrajCol].back().second;
            if (ty == tPrey) {
                if (tx > tPrex+1) for (int i = tPrex+1; i<tx; ++i) {
                    if ((curTraj[ty][i]<0 || curTraj[ty][i]==curTrajCol) && curMap[ty][i]<0) curTraj[ty][i] = curTrajCol;
                    else return;
                } else if (tPrex > tx+1) for (int i = tx+1; i<tPrex; ++i) {
                    if ((curTraj[ty][i]<0 || curTraj[ty][i]==curTrajCol) && curMap[ty][i]<0) curTraj[ty][i] = curTrajCol;
                    else return;
                }
            } else if (tx == tPrex) {
                if (ty > tPrey+1) for (int i = tPrey+1; i<ty; ++i) {
                    if ((curTraj[i][tx]<0 || curTraj[i][tx]==curTrajCol) && curMap[i][tx]<0) curTraj[i][tx] = curTrajCol;
                    else return;
                }
                else if (tPrey > ty+1) for (int i = ty+1; i<tPrey; ++i) {
                    if ((curTraj[i][tx]<0 || curTraj[i][tx]==curTrajCol) && curMap[i][tx]<0) curTraj[i][tx] = curTrajCol;
                    else return;
                }
            } else {
                return;
            }
            curTraj[ty][tx] = curTrajCol;
            curPos = make_pair(ty, tx);
            pipeMaps[curTrajCol].push_back(curPos);
        } else {
            if (checkSolve()) {
                QSound::play("./sounds/complete.wav");
                qDebug() << "Free Flow completed!";
                ui->msgLabel->setText("Free Flow completed!");
                passed = 1;
                curTrajCol = -1;
            } else {
                // conflict, but not confirmed until mouse released
//                QSound::play("./sounds/conflict.wav");
//                qDebug() << "Pipe conflict.";
//                ui->msgLabel->setText("Pipe conflict");
            }
        }
        update();
    }
}


void MainWindow::paintEvent(QPaintEvent *) {
    QPainter pt(this);

    if (curTrajCol >= 0) {
        pt.setPen(QPen(semitranC[curTrajCol],indcaW,Qt::SolidLine,Qt::RoundCap));
        pt.drawPoint(mouseX, mouseY);
    }
//    this->stepLable->setNum(3);
    ui->stepLabel->setText(stepqs+QString::number(hisTraj.size()-1));
    drawGrid(&pt);
    drawMap(&pt);
    drapTraj(&pt);
    if (passed) {
        this->setStyleSheet("QMainWindow {border-image: url(:/backgrounds/success_bk.png)}");
    }
}

void MainWindow::drawGrid(QPainter *pt) {
    pt->setPen(QPen(Qt::yellow,1,Qt::SolidLine,Qt::RoundCap));
    for (int x = borderH; x <= borderH+winH; x+=gapL) {
        pt->drawLine(x, borderH, x, borderH+winH);
        pt->drawLine(borderH, x, borderH+winH, x);
    }
}

void MainWindow::drawMap(QPainter *pt) {
    for (int i = 0; i < numRow; ++i)
        for (int j = 0; j < numRow; ++j) {
            if (curMap[i][j] < 0) continue;
            pt->setPen(QPen(color[curMap[i][j]],ballW,Qt::SolidLine,Qt::RoundCap));
            pt->drawPoint(borderH+j*gapL+gapL/2, borderH+i*gapL+gapL/2);
        }
}

void MainWindow::drapTraj(QPainter *pt) {
    // fill grids
    for (int i = 0; i < numRow; ++i)
        for (int j = 0; j < numRow; ++j) {
            if (curTraj[i][j] < 0) continue;
            pt->fillRect(borderH+j*gapL, borderH+i*gapL, gapL, gapL, semitranC[curTraj[i][j]]);
        }

    // drap pipes
    map<int, vector<pair<int, int> > >::iterator it = pipeMaps.begin();
    for (; it != pipeMaps.end(); ++it) {
        pt->setPen(QPen(color[it->first],pipeW,Qt::SolidLine,Qt::RoundCap));
        vector<pair<int, int> > tvp = it->second;
        for (int i = 0; i < (int)tvp.size()-1; ++i) {
            pt->drawLine(borderH+tvp[i].second*gapL+gapL/2, borderH+tvp[i].first*gapL+gapL/2,\
                         borderH+tvp[i+1].second*gapL+gapL/2, borderH+tvp[i+1].first*gapL+gapL/2);
        }
    }
}

void MainWindow::on_actionReset_triggered() {
    this->resetTraj();
}

void MainWindow::on_actionWithdraw_triggered() {
    if (hisTraj.size() <= 1) {
        qDebug() << "No history step available.";
        ui->msgLabel->setText("No history step available");
        return;
    }
    qDebug() << "Withdraw successfully.";
    ui->msgLabel->setText("Withdraw successfully");
    hisTraj.pop_back();
    curTraj = hisTraj.back();
    hisPipe.pop_back();
    if (hisPipe.empty()) pipeMaps.clear();
    else pipeMaps = hisPipe.back();
    update();
}

void MainWindow::on_action5_5_triggered() {
    if (numRow == 5) return;
    if (gameMaps[5].empty()) {
        qDebug() << "Maps for 5*5 not available";
        ui->msgLabel->setText("Maps for 5*5 not available");
        return;
    }
    numRow = 5;
    resetPara();
    curMapN = 0;
    curMap = gameMaps[numRow][curMapN];
    resetTraj();
    this->update();
}

void MainWindow::on_action6_6_triggered() {
    if (numRow == 6) return;
    if (gameMaps[6].empty()) {
        qDebug() << "Maps for 6*6 not available";
        ui->msgLabel->setText("Maps for 6*6 not available");
        return;
    }
    numRow = 6;
    resetPara();
    curMapN = 0;
    curMap = gameMaps[numRow][curMapN];
    resetTraj();
    this->update();
}

void MainWindow::on_action7_7_triggered() {
    if (numRow == 7) return;
    if (gameMaps[7].empty()) {
        qDebug() << "Maps for 7*7 not available";
        ui->msgLabel->setText("Maps for 7*7 not available");
        return;
    }
    numRow = 7;
    resetPara();
    curMapN = 0;
    curMap = gameMaps[numRow][curMapN];
    resetTraj();
    this->update();
}

void MainWindow::on_action8_8_triggered() {
    if (numRow == 8) return;
    if (gameMaps[8].empty()) {
        qDebug() << "Maps for 8*8 not available";
        ui->msgLabel->setText("Maps for 8*8 not available");
        return;
    }
    numRow = 8;
    resetPara();
    curMapN = 0;
    curMap = gameMaps[numRow][curMapN];
    resetTraj();
    this->update();
}

void MainWindow::on_actionLoad_triggered() {
    QString name = QFileDialog::getOpenFileName(this);
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    QTextStream qin(&file);
    int numM, scaleM;
    qin >> numM;
    for (int i = 0; i < numM; ++i) {
        qin >> scaleM;
        gameMaps[scaleM].push_back(vector<vector<int> >(scaleM, vector<int>(scaleM, -1)));
        int endi = gameMaps[scaleM].size()-1;
        for (int ii = 0; ii < scaleM; ++ii)
            for (int jj = 0; jj < scaleM; ++jj)
                qin >> gameMaps[scaleM][endi][ii][jj];
    }
    file.close();
}

void MainWindow::on_actionSol_triggered() {
    if (solveFF(curMap, curTraj, pipeMaps)) {
        ui->msgLabel->setText("Solution");
    } else {
        ui->msgLabel->setText("Unsolvable");
    }
    update();
}

void MainWindow::setTrajForPipe(int col) {
    if (pipeMaps.find(col)==pipeMaps.end() || pipeMaps[col].empty())
        return;
    for (int i = 0; i < (int)pipeMaps[col].size(); ++i)
        curTraj[pipeMaps[col][i].first][pipeMaps[col][i].second] = col;
}

void MainWindow::on_actionHint_triggered() {
    vector<vector<int> > tTraj;
    map<int, vector<pair<int, int> > > tPipe;
    if (solveFF(curMap, tTraj, tPipe)) {
        ui->msgLabel->setText("Hint");
        for (int i = 0; i < (int)tPipe.size(); ++i)
            if (pipeMaps.find(i)==pipeMaps.end() || pipeMaps[i].empty()) {
                pipeMaps[i] = tPipe[i];
                setTrajForPipe(i);
                break;
            }
    } else {
        ui->msgLabel->setText("Unsolvable");
    }
    update();
}

void MainWindow::on_actionPre_level_triggered() {
    if (curMapN <= 0) {
        qDebug() << "Previous map not available.";
        ui->msgLabel->setText("Previous map not available");
        return;
    }
    --curMapN;
    curMap = gameMaps[numRow][curMapN];
    this->resetTraj();
}

void MainWindow::on_actionNext_level_triggered() {
    if (curMapN >= (int)gameMaps[numRow].size()-1) {
        qDebug() << "Next map not available.";
        ui->msgLabel->setText("Next map not available");
        return;
    }
    ++curMapN;
    curMap = gameMaps[numRow][curMapN];
    this->resetTraj();
}

void MainWindow::resetTraj() {
    pipeMaps.clear();
    hisPipe.clear();
    curTraj = initTraj[numRow];
    hisTraj.clear();
    hisTraj.push_back(curTraj);
    curTrajCol = -1;
    passed = false;
    this->setStyleSheet("QMainWindow {background: 'black';}");
    this->update();
}

void MainWindow::resetPara() {
    gapL = winH / numRow;
    ballW = (2*winH) / (3*numRow);
    pipeW = ballW / 2;
    indcaW = 5*ballW/2;
}

bool MainWindow::getColRow(int x, int y, int &col, int &row) {
    if (x<borderH || y<borderH || x>=(winH+borderH) || y>=(winH+borderH)) return false;
    col = (x-borderH)/gapL;
    row = (y-borderH)/gapL;
    return true;
}

bool MainWindow::cleanTraj(int col) {
    bool changed = 0;
    pipeMaps[col].clear();
    for (int i = 0; i < numRow; ++i)
        for (int j = 0; j < numRow; ++j)
            if (curTraj[j][i] == col) {
                curTraj[j][i] = -1;
                changed = 1;
            }
    return changed;
}

bool MainWindow::checkSolve() {
    for (int i = 0; i < numRow; ++i)
        for (int j = 0; j < numRow; ++j)
            if (curTraj[j][i] == -1) return false;
    return true;
}

bool MainWindow::trajChanged(const vector<vector<int> > &v1, const vector<vector<int> > &v2) {
    int tscale = v1.size();
    for (int i = 0; i < tscale; ++i)
        for (int j = 0; j < tscale; ++j)
            if (v1[i][j] != v2[i][j]) return true;
    return false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered() {
    this->close();
}

void MainWindow::fileRead() {
    ifstream myfile ("./gameMaps");
    if (myfile.is_open())
    {
        int numM, scaleM;
        myfile >> numM;
        for (int i = 0; i < numM; ++i) {
            myfile >> scaleM;
            gameMaps[scaleM].push_back(vector<vector<int> >(scaleM, vector<int>(scaleM, -1)));
            int endi = gameMaps[scaleM].size()-1;
            for (int ii = 0; ii < scaleM; ++ii)
                for (int jj = 0; jj < scaleM; ++jj)
                    myfile >> gameMaps[scaleM][endi][ii][jj];
        }
    }
    else{
        cout << "Unable to open file";
    }
}
