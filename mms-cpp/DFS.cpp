#include <iostream>
#include <string>
#include <stack>
#include "API.h"

namespace commands{
    inline void mf(int distance = 1) { API::moveForward(distance);}
    inline void mfh(int distance = 1) { API::moveForwardHalf(distance);}
    inline void tr() { API::turnRight();}
    inline void tl() { API::turnLeft();}
    inline void trh() { API::turnRight45();}
    inline void tlh() { API::turnLeft45();}
    inline void sc(int x, int y, char color) { API::setColor(x, y, color);}
    inline void st(int x, int y, const std::string& text) { API::setText(x, y, text);}
    inline void sw(int x, int y, char direction) { API::setWall(x, y, direction);}
    inline void cw(int x, int y, char direction) { API::clearWall(x, y, direction);}
    inline bool wl() { API::wallLeft();}
    inline bool wr() { API::wallRight();}
    inline bool wf() { API::wallFront();} 
    inline bool wb() { API::wallBack();} 

}

enum Direction { S=(1<<0), W=(1<<1), N=(1<<2), E=(1<<3) };
int visited[16][16];
std::stack<char> memory;
struct Node { int x; int y; int walls;};
std::stack<Node> branch;


using namespace commands;
using namespace std;


void log(const std::string& text) {
    std::cerr << text << std::endl;
}

int turnTo(int* currentAngle, int targetAngle)
{
    int deg = 0;
    if(targetAngle >= 360) {  targetAngle -= 360;}
    if(targetAngle < 0) {  targetAngle += 360;}
    if(*currentAngle == targetAngle) { return 0;}
    if( (targetAngle % 45) != 0 ){ log("Can't turn that angle: " + to_string(targetAngle)); return 404;}
    
    if(*currentAngle == 270 && targetAngle == 0)
    {
        tl();
        *currentAngle = 0;
    }
    else if(*currentAngle > targetAngle)
    {
        while(*currentAngle != targetAngle)
        {
            tr();
            *currentAngle -= 90;
        }
    }

    else if(*currentAngle == 0 && targetAngle == 270)
    {
        tr();
        *currentAngle = 270;
    }

    else if(*currentAngle < targetAngle)
    {
       while(*currentAngle != targetAngle)
        {
            tl();
            *currentAngle += 90;
        } 
    }
    return 0;
}

int angleNorm(int* angle)
{
    if(*angle >= 360) { return *angle -= 360;}
    if(*angle < 0) { return *angle += 360;}
}


int updateCor(int* x, int* y, int* angle)
{
    angleNorm(angle);
    if(*angle == 0) { *x += 1; visited[15-*y][*x] = 1; memory.push('E'); return 0; }
    else if(*angle == 90) { *y += 1; visited[15-*y][*x] = 1; memory.push('N'); return 0;}
    else if(*angle == 180) { *x -= 1; visited[15-*y][*x] = 1; memory.push('W'); return 0;}
    else if(*angle == 270) { *y -= 1; visited[15-*y][*x] = 1; memory.push('S'); return 0;}
}


char cardinal(int angle)
{
    angleNorm(&angle);
    if (angle == 0) { return 'e';}
    else if(angle == 90) { return 'n';}
    else if(angle == 180) { return 'w';}
    else if(angle == 270) { return 's';}
}

int locToGlobal(int angle)
{
    angleNorm(&angle);
    if(angle == 0){return 8;}
    else if(angle == 90){return 4;}
    else if(angle == 180){return 2;}
    else if(angle == 270){return 1;}
}

int checkWalls(int x, int y, int angle)
{
    // check east, north, west, south walls respectivly
    // the return value is int binary format of them like 1111
    int sum = 0;
    if(wl()) { sw(x, y, cardinal(angle + 90)); sum += locToGlobal(angle + 90);}  
    if(wf()) { sw(x, y, cardinal(angle)); sum += locToGlobal(angle);}  
    if(wr()) { sw(x, y, cardinal(angle - 90)); sum += locToGlobal(angle - 90);} 
    if(wb()) { sw(x, y, cardinal(angle + 180)); sum += locToGlobal(angle + 180);} 
    return sum;
}

void printVisited()
{
    for(int i=0; i<16; i++){
            int j = 0;
            log(
    to_string(visited[i][j])   + " " +
    to_string(visited[i][j+1]) + " " +
    to_string(visited[i][j+2]) + " " +
    to_string(visited[i][j+3]) + " " +
    to_string(visited[i][j+4]) + " " +
    to_string(visited[i][j+5]) + " " +
    to_string(visited[i][j+6]) + " " +
    to_string(visited[i][j+7]) + " " +
    to_string(visited[i][j+8]) + " " +
    to_string(visited[i][j+9]) + " " +
    to_string(visited[i][j+10]) + " " +
    to_string(visited[i][j+11]) + " " +
    to_string(visited[i][j+12]) + " " +
    to_string(visited[i][j+13]) + " " +
    to_string(visited[i][j+14]) + " " +
    to_string(visited[i][j+15]) 
    );
    }
    log(" ");
    log(" ");
    log(" ");
}

int goBack(int* x, int* y, int* angle)
{
    char command;
    Node cur = branch.top();
    int Tx = cur.x;
    int Ty = cur.y;
    tl(); tl(); *angle += 180; angleNorm(angle);
    while( *x != Tx || *y != Ty){
    command = memory.top();
    memory.pop();
    if (branch.empty()) { log("ERROR: branch empty"); return 1; }
    if (memory.empty()) { log("ERROR: memory empty"); return 1; }
    if(command == 'N') { turnTo(angle, 270); mf(); updateCor(x, y, angle); memory.pop();}
    else if (command == 'S')
    {
        
        turnTo(angle, 90);
        mf();
        updateCor(x, y, angle); memory.pop();
        
    }
    else if (command == 'E')
    {

        turnTo(angle, 180);
        mf();
        updateCor(x, y, angle); memory.pop();
        
    }

    else if(command == 'W')
    {
        turnTo(angle, 0);
        mf();
        updateCor(x, y, angle); memory.pop();
    }
    printVisited();
    }
    branch.pop();
    return 0;
}

void next(int* x, int* y, int* angle, int* walls)
{
    int row = 15 - *y;
    int col = *x;
    // East
    if ((visited[row][col+1] != 1) && !(*walls & (1<<3))) {
        turnTo(angle, 0);
        mf();
        updateCor(x, y, angle);
    }

    // North
    else if ((visited[row-1][col] != 1) && !(*walls & (1<<2))) {
        turnTo(angle, 90);
        mf();
        updateCor(x, y, angle);
    }

    // West
    else if ((visited[row][col-1] != 1) && !(*walls & (1<<1))) {
        turnTo(angle, 180);
        mf();
        updateCor(x, y, angle);
    }

    // South
    else if ((visited[row+1][col] != 1) && !(*walls & (1<<0))) {
        turnTo(angle, 270);
        mf();
        updateCor(x, y, angle);
    }

    // All Visited
    else if ( 
        (visited[row+1][col] == 1 || (*walls & (1<<0))) &&
        (visited[row][col-1] == 1 || (*walls & (1<<1))) &&
        (visited[row-1][col] == 1 || (*walls & (1<<2))) &&
        (visited[row][col+1] == 1 || (*walls & (1<<3)))
    ) {
         Node cur = branch.top();
         int Brow = 15 - cur.y;
        int Bcol = cur.x;
        int BranchWalls = cur.walls;
        while(
        (visited[Brow+1][Bcol] == 1 || (BranchWalls & (1<<0))) &&
        (visited[Brow][Bcol-1] == 1 || (BranchWalls & (1<<1))) &&
        (visited[Brow-1][Bcol] == 1 || (BranchWalls & (1<<2))) &&
        (visited[Brow][Bcol+1] == 1 || (BranchWalls & (1<<3))))
        {
            branch.pop();
            cur = branch.top();
            Brow = 15 - cur.y;
            Bcol = cur.x;
            BranchWalls = cur.walls;
        }
        goBack(x, y, angle);
    }

    else {
        goBack(x, y, angle);
    }
}
void DFS_Algorithm()
{
    int x = 0, y = 0;
    int angle = 90;
    int walls;
    for(int i=0; i<16; i++){
        for(int j=0; j<16; j++){
            visited[i][j] = 0;
        }
    }
    visited[15][0] = 1;
    log("Running...");
    sc(0, 0, 'G');
    st(0, 0, "Start");

    while(true){
        walls = checkWalls(x, y, angle);
        //branch detected
        if( walls == 0 || walls == (1<<0) || walls == (1<<1) || walls == (1<<2) || walls == (1<<3))
        {
            branch.push({x, y, walls});
            sc(x, y, 'R');
        }
        if( (x == 7 && y == 7) || (x == 7 && y == 8) || (x == 8 && y == 7) || (x == 8 && y == 8)  )
        {
            log("Let's Goooooooooooooooo");
            break;
        } 
        next(&x, &y, &angle, &walls);
        printVisited();
    }

}