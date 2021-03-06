
// v1.4.2

#include <map>
#include <set>
#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <queue>

#define PB push_back
#define MP make_pair
#define SZ(a) a.size()
#define DEBUG(x) cout << #x << " = " << x << endl;

#define PR(A,n)  { cout << #A << " = "; for (int iter = 1, _n = (n); iter <= _n; iter++) \
    cout << A[iter] << ' '; cout << endl; }
#define PR0(A,n) { cout << #A << " = "; for (int iter = 0, _n = (n); iter < _n; iter++) \
    cout << A[iter] << ' '; cout << endl; }

using namespace std;

struct Position {
    int x, y;
};

const int oo = 1000000;
const int nRows = 20;
const int nColumns = 30;
const int Threshold = 0;

const int dX[4] = {0, -1, 0, 1};
const int dY[4] = {1, 0, -1, 0};


enum semanticMoves { RIGHT, UP, LEFT, DOWN };

int board[nRows + 5][nColumns + 5];
int nBots, myBotId, myStableNumber, myUnstableNumber;
int curRow, curCol;
int lastMove;
int firstMove;

int prevRow, prevCol;

Position currentBotPosition[10];
Position trace[nRows + 5][nColumns + 5];
queue< pair<int, int> > q;
bool visited[nRows + 5][nColumns + 5];

Position currentDestination = {-1, -1};
vector<Position> exDestination;

int *perm;

bool isInsideBoard(int x, int y);
semanticMoves moveBeforeGoOutTo(int u, int v);
semanticMoves otherStrategyMove();
semanticMoves greedyMove();
bool isStableCell(int x, int y);

int f[nRows + 5][nColumns + 5];

// variables here
int sample[12][4] = {{0, 1, 2, 3}, {1, 2, 0, 3}, {2, 1, 0, 3}, {3, 2, 0, 1}, {0, 1, 3, 2},
    {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 1, 2}, {0, 3, 2, 1},
    {1, 2, 3, 0}, {1, 3, 0, 2}, {3, 0, 1, 2}};

int *getPerm() {
    srand (time(NULL));
    int r = rand() % 12;
    int *perm = sample[r];

    return perm;
}

void initBoard(){
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nColumns; j++){
            board[i][j] = 9;
        }

    }
}

void printBoard(){
    for (int i = 0; i < nRows; i++){
        for (int j = 0; j < nColumns; j++) {
            cout << board[i][j];
        }
        cout << endl;
    }
}

int manhattanDistance(Position p1, Position p2) {
    return (abs(p1.x - p2.x) + abs(p1.y - p2.y));
}

int stableDistance(Position inner, Position outter) {
    // constraint: outter is adjacent to stable area
    // inner is in stable area
    // return the minimum path on stable area from inner to outter
    if (inner.x == outter.x && inner.y == outter.y) return 0;

    q = queue< pair<int, int> >();
    memset(visited, false, sizeof(visited));
    //    memset(trace, 0, sizeof(trace));
    memset(f, 0, sizeof(f));


    q.push(make_pair(inner.x, inner.y));
    visited[inner.x][inner.y] = true;
    f[inner.x][inner.y] = 0;

    while (!q.empty()) {
        int u = q.front().first;
        int v = q.front().second;
        q.pop();

        for (int i = 0; i <= 3; i++) {
            int uNext = u + dX[i];
            int vNext = v + dY[i];

            if (visited[uNext][vNext]) continue;
            //            if (uNext == uEx && vNext == vEx) continue;

            if (isStableCell(uNext, vNext)) {
                visited[uNext][vNext] = true;
                q.push(make_pair(uNext, vNext));    
                f[uNext][vNext] = f[u][v] + 1;
            } else {
                if (isInsideBoard(uNext, vNext)) {
                    if (uNext == outter.x && vNext == outter.y) {
                        return f[u][v] + 1;
                    }
                }
            }
        }
    }

    return 1000;
}

void checkReachabilityCurrentDestination() {
    q = queue< pair<int, int> >();
    memset(visited, false, sizeof(visited));


    q.push(make_pair(curRow, curCol));
    visited[curRow][curCol] = true;

    while (!q.empty()) {
        int u = q.front().first;
        int v = q.front().second;
        q.pop();

        for (int i = 0; i <= 3; i++) {
            int uNext = u + dX[i];
            int vNext = v + dY[i];

            if (visited[uNext][vNext]) continue;

            if (isStableCell(uNext, vNext)) {
                visited[uNext][vNext] = true;
                q.push(make_pair(uNext, vNext));    
            } else {
                if (isInsideBoard(uNext, vNext)) {
                    if (uNext == currentDestination.x && vNext == currentDestination.y) {
                        return;
                    }
                }
            }
        }
    }

    
    // not reachable by stable move
    currentDestination = {-1, -1};
    exDestination.clear();

}

void printMove(semanticMoves move){
    switch (move){
        case RIGHT:
            cout << "RIGHT" << endl;
            break;
        case UP:
            cout << "UP" << endl;
            break;
        case LEFT:
            cout << "LEFT" << endl;
            break;
        case DOWN:
            cout << "DOWN" << endl;
    }
}

bool isThisMoveValid(semanticMoves move, int& nextVal){
    semanticMoves lastSemanticMove;
    bool isFirstMove = false;
    if (lastMove == -1)
        isFirstMove = true;
    else
        lastSemanticMove = static_cast<semanticMoves>(lastMove);

    switch (move) {
        case RIGHT: {
                        if (!isFirstMove && lastSemanticMove == LEFT)
                            return false;
                        if ((curCol + 1 < nColumns) && (board[curRow][curCol + 1] != myUnstableNumber)){
                            nextVal = board[curRow][curCol + 1];
                            return true;
                        }
                        return false;
                    }
        case UP: {
                     if (!isFirstMove && lastSemanticMove == DOWN)
                         return false;
                     if ((curRow - 1 >= 0 ) && (board[curRow - 1][curCol] != myUnstableNumber)){
                         nextVal = board[curRow - 1][curCol];
                         return true;
                     }
                     return false;
                 }
        case LEFT: {
                       if (!isFirstMove && lastSemanticMove == RIGHT)
                           return false;
                       if ((curCol - 1 >= 0 ) && (board[curRow][curCol - 1] != myUnstableNumber)){
                           nextVal = board[curRow][curCol - 1];
                           return true;
                       }
                       return false;
                   }
        case DOWN: {
                       if (!isFirstMove && lastSemanticMove == UP)
                           return false;
                       if ((curRow + 1 < nRows ) && (board[curRow + 1][curCol] != myUnstableNumber)){
                           nextVal = board[curRow + 1][curCol];
                           return true;
                       }
                       return false;
                   }
    }
}

semanticMoves defensiveMove(){
    int i, nextVal;
    semanticMoves realMove;
    for (i = 0; i < 4; i++){
        realMove = static_cast<semanticMoves>(i);
        if (isThisMoveValid(realMove, nextVal))
            if (nextVal == myStableNumber)
                return realMove;
    }
}

semanticMoves noNameMove(){
    int i, nextVal;
    semanticMoves realMove;
    for (i = 0; i < 4; i++){
        realMove = static_cast<semanticMoves>(i);
        if (isThisMoveValid(realMove, nextVal))
            return realMove;
    }
}

semanticMoves fromPosition(int destRow, int destCol) {
    for (int i = 0; i <= 3; i++) {
        if (dX[i] == (destRow - curRow) && dY[i] == (destCol - curCol)) {
            return static_cast<semanticMoves>(i);
        }
    }
}

bool isPosOfAnotherBot(int x, int y) {
//    DEBUG(x);
//    DEBUG(y);
    for (int i = 1; i <= nBots - 1; i++) {
        if (x == currentBotPosition[i].x && y == currentBotPosition[i].y) {

            return true;
        }
    }

    return false;
}

bool isStableCell(int x, int y) {
    return (board[x][y] == myStableNumber);
}

bool isInsideBoard(int x, int y) {
    return ((x >= 0 && x < nRows) && (y >= 0 && y < nColumns));
}

int numAdjStableCell(int x, int y) {
    int count = 0;
    for (int i = 0; i <= 3; i++) {
        int uu = x + dX[i];
        int vv = y + dY[i];
        if (isStableCell(uu, vv)) {
            count++;
        }
    }

    return count;
}

int distanceToAnotherBot(int x, int y) {
    int res = 0;
    for (int i = 1; i <= nBots - 1; i++) {
        int u = currentBotPosition[i].x;
        int v = currentBotPosition[i].y;

        if (abs(u - x) + abs(v - y) > res) 
            res = abs(u - x) + abs(v - y);
    }

    return res;
}

semanticMoves dumbStableMove() {
    int nextVal;
    // CASE: have a stable cell to arrive.
    // --> random move to that stable cell
    for (int i = 0; i <= 3; i++) {
        int uu = curRow + dX[i];
        int vv = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);

        if (isThisMoveValid(move, nextVal)) {
            if (isStableCell(uu, vv)) {
                return move;
            }
        }
    }

    for (int i = 0; i <= 3; i++) {
        int uu = curRow + dX[i];
        int vv = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);

        if (isThisMoveValid(move, nextVal)) {
            return move;
        }
    }
    
    return static_cast<semanticMoves>(0);
}

semanticMoves moveBeforeGoOutTo(int u, int v) {

    for (int i = 1; i <= nBots - 1; i++) {
        int xBot = currentBotPosition[i].x;
        int yBot = currentBotPosition[i].y;

        if (abs(xBot - curRow) + abs(yBot - curCol) <= 5) {
            exDestination.push_back({u, v});
                       
            return otherStrategyMove();
        }
    }
    
    exDestination.clear();
    currentDestination = {-1, -1};
    return fromPosition(u, v);
    
}

semanticMoves greedyMove() {
    


    int minDistance = oo;
    int minX = 0;
    int minY = 0;
    int nextVal;
    perm = getPerm();

    for (int i = 0; i <= 3; i++) {
        int uNext = curRow + dX[perm[i]];
        int vNext = curCol + dY[perm[i]];
        //        if (!isStableCell(uNext, vNext)) continue;

        semanticMoves move = static_cast<semanticMoves>(perm[i]);
        if (isThisMoveValid(move, nextVal)) {
            //            int distance = abs(uNext - currentDestination.x) + abs(vNext - currentDestination.y);
            if (isStableCell(uNext, vNext) || (uNext == currentDestination.x && vNext == currentDestination.y)) {
                int distance = stableDistance({uNext, vNext}, currentDestination);
                //            DEBUG(distance);
                //            DEBUG(uNext);
                //            DEBUG(vNext);
                if (distance < minDistance) {
                    minDistance = distance;
                    minX = uNext;
                    minY = vNext;
                }

            }
        }
    }
//    DEBUG(currentDestination.x);   
//    DEBUG(currentDestination.y); 
//    DEBUG(minX);
//    DEBUG(minY);
//    DEBUG(minDistance);

    int cnt = 0;
    if (minDistance < oo) {

        if (minX == currentDestination.x && minY == currentDestination.y) {
            return moveBeforeGoOutTo(minX, minY);
        }

        perm = getPerm();
        for (int i = 0; i < 4; i++) {
            int uNext = curRow + dX[perm[i]];
            int vNext = curCol + dY[perm[i]];
            //        if (!isStableCell(uNext, vNext)) continue;

            semanticMoves move = static_cast<semanticMoves>(perm[i]);
            if (isThisMoveValid(move, nextVal) && isStableCell(uNext, vNext)) {
                //            int distance = abs(uNext - currentDestination.x) + abs(vNext - currentDestination.y);
                int distance = stableDistance({uNext, vNext}, currentDestination);
                if (distance == minDistance) {
                    return fromPosition(uNext, vNext);
                }

            }
        }

        return fromPosition(minX, minY);
    } else {
        currentDestination = {-1, -1};
        exDestination.clear();
        perm = getPerm();

        for (int i = 0; i < 4; i++) {
            int uNext = curRow + dX[perm[i]];
            int vNext = curCol + dY[perm[i]];

            semanticMoves move = static_cast<semanticMoves>(perm[i]);
            if (isThisMoveValid(move, nextVal)) {
                return move;
            }
        }
    }

}


semanticMoves otherStrategyMove() {

    q = queue< pair<int, int> >();
    memset(visited, false, sizeof(visited));
    //    memset(trace, 0, sizeof(trace));
    memset(f, 0, sizeof(f));

    q.push(make_pair(curRow, curCol));
    visited[curRow][curCol] = true;
    f[curRow][curCol] = 0;

    bool havePrimaryPos = false;
    Position primaryPos;
    int primDis = 100000;

    bool haveSecondaryPos = false;
    Position secondaryPos;
    int sedDis = 100000;

    for (int i = 0; i < exDestination.size(); i++) {
        visited[exDestination[i].x][exDestination[i].y] = true;
    }

    while (!q.empty()) {
        int u = q.front().first;
        int v = q.front().second;
        q.pop();

        for (int i = 0; i <= 3; i++) {
            int uNext = u + dX[i];
            int vNext = v + dY[i];

            // NOTE
            semanticMoves move = static_cast<semanticMoves>(i);

            if (visited[uNext][vNext]) continue;
            //            if (uNext == uEx && vNext == vEx) continue;

            if (isStableCell(uNext, vNext)) {
                visited[uNext][vNext] = true;
                q.push(make_pair(uNext, vNext));    
                f[uNext][vNext] = f[u][v] + 1;
            } else {
                if (isInsideBoard(uNext, vNext)) {
                    if (numAdjStableCell(uNext, vNext) >= 2) {
                        // uNext, vNext is the expected position.
                        primaryPos = {uNext, vNext};
                        havePrimaryPos = true;
                        f[uNext][vNext] = f[u][v] + 1;
                        primDis = f[uNext][vNext];
                        break;
                    } else {
                        if (!haveSecondaryPos) {
                            haveSecondaryPos = true;
                            secondaryPos = {uNext, vNext};
                            f[uNext][vNext] = f[u][v] + 1;
                            sedDis = f[uNext][vNext];

                        }
                    }
                }
            }
        }
        if (havePrimaryPos) break;
    }

    int nextVal;
    if (havePrimaryPos || haveSecondaryPos) {
        if (!havePrimaryPos) {
            havePrimaryPos = true;
            primaryPos = secondaryPos;

        } else {
            if (haveSecondaryPos) {
                if (sedDis + Threshold < primDis) {
                    havePrimaryPos = true;
                    primaryPos = secondaryPos;
                }
            }
        }

        currentDestination = primaryPos;
        //        DEBUG(currentDestination.x);
        //        DEBUG(currentDestination.y);

        return greedyMove();
    }

    return dumbStableMove();

}


semanticMoves safeStrategyFromStable() {
    // reset q and visited
    q = queue< pair<int, int> >();
    memset(visited, false, sizeof(visited));
    memset(trace, 0, sizeof(trace));
    memset(f, 0, sizeof(f));


    q.push(make_pair(curRow, curCol));
    visited[curRow][curCol] = true;
    f[curRow][curCol] = 0;

    bool haveSecondaryPos = false;
    Position secondaryPos;
    int sedDis = 100000;

    bool havePrimaryPos = false;
    Position primaryPos;
    int primDis = 100000;

    while (!q.empty()) {
        int u = q.front().first;
        int v = q.front().second;
        q.pop();

        for (int i = 0; i <= 3; i++) {
            int uNext = u + dX[i];
            int vNext = v + dY[i];

            // NOTE
            semanticMoves move = static_cast<semanticMoves>(i);

            if (visited[uNext][vNext]) continue;

            if (isStableCell(uNext, vNext)) {
                visited[uNext][vNext] = true;
                q.push(make_pair(uNext, vNext));    
                trace[uNext][vNext] = {u, v};
                f[uNext][vNext] = f[u][v] + 1;
            } else {
                if (isInsideBoard(uNext, vNext)) {         

                    if (numAdjStableCell(uNext, vNext) >= 2) {
                        // uNext, vNext is the expected position.
                        primaryPos = {uNext, vNext};
                        havePrimaryPos = true;
                        trace[uNext][vNext] = {u, v};
                        f[uNext][vNext] = f[u][v] + 1;
                        primDis = f[uNext][vNext];
                        break;
                    } else {
                        if (!haveSecondaryPos) {
                            haveSecondaryPos = true;
                            secondaryPos = {uNext, vNext};
                            trace[uNext][vNext] = {u, v};
                            f[uNext][vNext] = f[u][v] + 1;
                            sedDis = f[uNext][vNext];
                        }
                    }
                }
            }
        }
        if (havePrimaryPos) break;
    }


    if (havePrimaryPos || haveSecondaryPos) {
        if (!havePrimaryPos) {
            havePrimaryPos = true;
            primaryPos = secondaryPos;
        } else {
            if (haveSecondaryPos) {
                if (sedDis + Threshold < primDis) {
                    havePrimaryPos = true;
                    primaryPos = secondaryPos;
                }
            }
        }

        currentDestination = primaryPos;
//        DEBUG(currentDestination.x);
//        DEBUG(currentDestination.y);
        
        return greedyMove();

    }
    return dumbStableMove();
}



bool isDangerous() {

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nColumns; j++) {
            if (board[i][j] == myUnstableNumber) {
                for (int k = 1; k <= nBots - 1; k++) {
                    int u = currentBotPosition[k].x;
                    int v = currentBotPosition[k].y;

                    if (curRow == i && curCol == j) {
                        if (abs(u - i) + abs(v - j) <= 4) return true;
                    } else
                    if (abs(u - i) + abs(v - j) <= 2) return true;
                }
            }
        }
    }
    
    return false;
}

int distanceToStable(int u, int v) {
    int res = 10000000;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nColumns; j++) {
            if (isStableCell(i, j)) {
                if (abs(u - i) + abs(v - j) < res) {
                    res = abs(u - i) + abs(v - j);
                }
            }
        }
    }
    return res;
}

semanticMoves safeStrategyFromUnstable() {
    currentDestination = {-1, -1};
    
    int nextVal;

    // CASE: Isn't dangerous: 
    // -> move to empty cell which is beside the stable area.
    //      

    if (!isDangerous()) {
        for (int i = 0; i <= 3; i++) {
            int uu = curRow + dX[i];
            int vv = curCol + dY[i];
            semanticMoves move = static_cast<semanticMoves>(i);
            if (isThisMoveValid(move, nextVal)) {
                if (!isStableCell(uu, vv)) {
                    bool ok = false;
                    for (int j = 0; j <= 3; j++) {
                        if (uu + dX[j] != curRow || vv + dY[j] != curCol) {
                            if (isStableCell(uu + dX[j], vv + dY[j])) {
                                return move;
                            }
                        }       
                    }
                }
            }
        }
    }


    // CASE: Is dangerous && have a stable cell to arrive.
    for (int i = 0; i <= 3; i++) {
        int uu = curRow + dX[i];
        int vv = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);

        if (isThisMoveValid(move, nextVal)) {
            if (isStableCell(uu, vv)) {
                currentDestination = {-1, -1};
                exDestination.clear();

                return move;
            }
        }
    }
    

    // CASE: Is dangerous && have no stable cell to arrive.
    int minDistance = 10000000;
    semanticMoves minMove;

    for (int i = 0; i <= 3; i++) {
        int uNext = curRow + dX[i];
        int vNext = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);
        
        if (!isThisMoveValid(move, nextVal)) continue;

        int d = distanceToStable(uNext, vNext);
        if (d < minDistance) {
            minDistance = d;
            minMove = move;
        }
    }

    if (minDistance != 10000000) {
        return minMove;
    }


    
    // CASE: otherwise.
    // --> random move
    
    for (int i = 0; i <= 3; i++) {
        int uu = curRow + dX[i];
        int vv = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);

        if (isThisMoveValid(move, nextVal)) {
            return move;
        }
    }

}

// safe stratery move: 
// if current pos is adjacent to another bot -> go to that bot's pos             (1)
//
// if current pos is stable: find an empty cell that beside at lease 2 stable cell
// if current pos is unstable: 
//      + if we find a adj cell that from it we can go to stable area in 1 move -> move to it
//      + else: go to the adj stable cell.
//

semanticMoves safeStrategyMove() {
    int nextVal;
    bool isFirstMove = false;

    if (lastMove == -1)
        isFirstMove = true;

    for (int i = 0; i <= 3; i++) {
        int nextRow = curRow + dX[i];
        int nextCol = curCol + dY[i];
        semanticMoves move = static_cast<semanticMoves>(i);
        
        // (1)
        if (isThisMoveValid(move, nextVal)) {
            if (!isPosOfAnotherBot(nextRow, nextCol) && board[nextRow][nextCol] % 2 == 0) {
                if (board[nextRow][nextCol] == 0) continue;
                if (nBots > 2) continue;
                
                return move;
            }    
        }      
    }

    // CASE: distance (myBots, otherBots) == 2
    for (int i = 1; i <= nBots - 1; i++) {
        int dBot = manhattanDistance({curRow, curCol}, currentBotPosition[i]);
        if (dBot <= 2) {
            bool ok = false;
            semanticMoves backup;
            for (int k = 0; k <= 3; k++) {
                int xNext = curRow + dX[k];
                int yNext = curCol + dY[k];
                semanticMoves move = static_cast<semanticMoves>(k);
                int d = manhattanDistance({xNext, yNext}, currentBotPosition[i]);
                if (isThisMoveValid(move, nextVal)) {
                    if (d > dBot) {
                        if (isStableCell(xNext, yNext)) {
//                            DEBUG("den day doi");
                            currentDestination = {-1, -1};
                            exDestination.clear();
                            return move;
                        } else {
                            backup = move;
                        }
                    }
                }

            }
            currentDestination = {-1, -1};
            exDestination.clear();
            return backup;
        }
    }


    if (board[curRow][curCol] == myStableNumber) {
        if (currentDestination.x != -1) {
            checkReachabilityCurrentDestination();
        }

        // ensure reachable
        if (currentDestination.x != -1) {
//                    cout << "den day" << endl;
            return greedyMove();
        }


        return safeStrategyFromStable();
    } else {

        return safeStrategyFromUnstable();
    }

}

void makeBestMove(){
    // int randomMove = rand() % 4;
    semanticMoves realMove;
    //
    // while (!isThisMoveValid(realMove)){
    //     randomMove = rand() % 4;
    //     semanticMoves realMove = static_cast<semanticMoves>(randomMove);
    // }

    int i;
    int nextVal;

    // You want to try a new strategy?
    // Please create a method with descriptive name and call it like this
    // realMove = noNameMove();
//    realMove = defensiveMove();
    
    realMove = safeStrategyMove();

    lastMove = realMove;
    printMove(realMove);
}


int main() {
    srand(time(NULL));
//    freopen("bug.txt", "r", stdin);
    int tempRow, tempCol;
    char temp;

//    initBoard();
    // Read initial inputs
    cin >> nBots;
    cin >> myBotId;

    myStableNumber = 2 * myBotId - 1;
    myUnstableNumber = myStableNumber + 1;
    lastMove = -1;
    //cout << "myUnstableNumber: " << " " << myUnstableNumber << endl;

    prevRow = -1;
    prevCol = -1;


    while(true){
        if (!feof(stdin)){
            // Read current state of the board
            for (int i = 0; i < nRows; i++){
                for (int j = 0; j < nColumns; j++){
                    cin >> temp;
                    board[i][j] = temp - 48;
                }
            }
            
//            printBoard();

            int otherBotCounts = 0;
            // Read current position of all bots
            for (int i = 1; i <= nBots; i++){
                cin >> tempRow >> tempCol;
                if (i == myBotId){
                    curRow = tempRow;
                    curCol = tempCol;
                } else {
                    otherBotCounts++;
                    currentBotPosition[otherBotCounts] = {tempRow, tempCol};
                }
                //board[tempRow][tempCol] = 10 * i; //TODO: Mapping function
            }
            // printBoard();

//            lastMove = RIGHT;
//            currentDestination = {5, 15};
//            exDestination.push_back({8, 13});
            makeBestMove();
//            DEBUG(exDestination.size());
        }
    }
}

