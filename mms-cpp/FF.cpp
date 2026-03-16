#include <iostream>
#include <deque>
#include <string>
#include "API.h"
#include "BFS.h"
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

using namespace commands;
using namespace std;

struct Location{
    int x;
    int y;
};

deque<Location> stack;
int rows = 16;
int cols = 16;



void log(const std::string& text) {
    std::cerr << text;
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

int checkWalls(int x, int y, int angle, int* walls)
{
    // check east, north, west, south walls respectivly
    // the return value is int binary format of them like 1111
    int sum = 0; 
    if(wl()) { sw(x, 15 - y, cardinal(angle + 90)); sum += locToGlobal(angle + 90);}  
    if(wf()) { sw(x, 15 - y, cardinal(angle)); sum += locToGlobal(angle);}  
    if(wr()) { sw(x, 15 - y, cardinal(angle - 90)); sum += locToGlobal(angle - 90);} 
    if(wb()) { sw(x, 15 - y, cardinal(angle + 180)); sum += locToGlobal(angle + 180);} 
    *(walls + x + 16 * y) = sum;
    log("the sum is: " + to_string(sum) + "\n");
    return sum;
}

int updateCor(int* x, int* y, int* angle)
{
    angleNorm(angle);
    if(*angle == 0) { *x += 1; return 0; }
    else if(*angle == 90) { *y -= 1; return 0;}
    else if(*angle == 180) { *x -= 1; return 0;}
    else if(*angle == 270) { *y += 1; return 0;}
}


void printmat(int* ptr)
{
    // i => row
    for(int i=0; i<16; i++){
    for(int j=0; j<16; j++){
        log(to_string(*(ptr + j + i * 16)) + " ");
        if(*(ptr + j + i * 16) < 10){log(" ");}
    }
    log("\n");
    }
    log("\n");
    log("\n");
    log("\n");
}

void fillmat(int* ptr, int* walls)
{
    int temp[16][16];

    for (int x = 0; x < rows; x++) {
        for (int y = 0; y < cols; y++) {
            temp[x][y] = *(walls + x + cols * y);
        }
    }

    for (int x = 0; x < rows; x++) {
         for (int y = 0; y < cols; y++) {
             *(ptr + x + cols * y) = 0;
         }
    }

    while (!stack.empty()) {
        stack.pop_back();
    }

    // center 4 cells
    *(ptr + rows / 2     + cols * (cols / 2))     = 1;
    *(ptr + rows / 2     + cols * (cols / 2 - 1)) = 1;
    *(ptr + rows / 2 - 1 + cols * (cols / 2))     = 1;
    *(ptr + rows / 2 - 1 + cols * (cols / 2 - 1)) = 1;

    stack.push_front({rows / 2,     cols / 2});
    stack.push_front({rows / 2 - 1, cols / 2});
    stack.push_front({rows / 2,     cols / 2 - 1});
    stack.push_front({rows / 2 - 1, cols / 2 - 1});

    Location cur;
    while (!stack.empty())
    {
        cur = stack.back();
        stack.pop_back();

        // move east: (x+1, y)
        if (
            cur.x + 1 < rows &&
            *(ptr + (cur.x + 1) + cols * cur.y) == 0 &&
            ((temp[cur.x + 1][cur.y] & (1<<1)) == 0) &&
            ((temp[cur.x][cur.y]     & (1<<3)) == 0)
        )
        {
            *(ptr + (cur.x + 1) + cols * cur.y) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x + 1, cur.y});
        }

        // move west: (x-1, y)
        if (
            cur.x - 1 >= 0 &&
            *(ptr + (cur.x - 1) + cols * cur.y) == 0 &&
            ((temp[cur.x - 1][cur.y] & (1<<3)) == 0) &&
            ((temp[cur.x][cur.y]     & (1<<1)) == 0)
        )
        {
            *(ptr + (cur.x - 1) + cols * cur.y) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x - 1, cur.y});
        }

        // move north: (x, y+1)
        if (
            cur.y + 1 < cols &&
            *(ptr + cur.x + cols * (cur.y + 1)) == 0 &&
            ((temp[cur.x][cur.y + 1] & N) == 0) &&
            ((temp[cur.x][cur.y]     & S) == 0)
        )
        {
            *(ptr + cur.x + cols * (cur.y + 1)) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x, cur.y + 1});
        }

        // move south: (x, y-1)
        if (
            cur.y - 1 >= 0 &&
            *(ptr + cur.x + cols * (cur.y - 1)) == 0 &&
            ((temp[cur.x][cur.y - 1] & S) == 0) &&
            ((temp[cur.x][cur.y]     & N) == 0)
        )
        {
            *(ptr + cur.x + cols * (cur.y - 1)) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x, cur.y - 1});
        }
    }
}



void fillmatStart(int* ptr, int* walls)
{
    int temp[16][16];

    for (int x = 0; x < rows; x++) {
        for (int y = 0; y < cols; y++) {
            temp[x][y] = *(walls + x + cols * y);
        }
    }

    for (int x = 0; x < rows; x++) {
         for (int y = 0; y < cols; y++) {
             *(ptr + x + cols * y) = 0;
         }
    }

    while (!stack.empty()) {
        stack.pop_back();
    }

    *(ptr + cols * 15)     = 1;

    stack.push_front({0, 15});

    Location cur;
    while (!stack.empty())
    {
        cur = stack.back();
        stack.pop_back();

        // move east: (x+1, y)
        if (
            cur.x + 1 < rows &&
            *(ptr + (cur.x + 1) + cols * cur.y) == 0 &&
            ((temp[cur.x + 1][cur.y] & (1<<1)) == 0) &&
            ((temp[cur.x][cur.y]     & (1<<3)) == 0)
        )
        {
            *(ptr + (cur.x + 1) + cols * cur.y) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x + 1, cur.y});
        }

        // move west: (x-1, y)
        if (
            cur.x - 1 >= 0 &&
            *(ptr + (cur.x - 1) + cols * cur.y) == 0 &&
            ((temp[cur.x - 1][cur.y] & (1<<3)) == 0) &&
            ((temp[cur.x][cur.y]     & (1<<1)) == 0)
        )
        {
            *(ptr + (cur.x - 1) + cols * cur.y) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x - 1, cur.y});
        }

        // move north: (x, y+1)
        if (
            cur.y + 1 < cols &&
            *(ptr + cur.x + cols * (cur.y + 1)) == 0 &&
            ((temp[cur.x][cur.y + 1] & N) == 0) &&
            ((temp[cur.x][cur.y]     & S) == 0)
        )
        {
            *(ptr + cur.x + cols * (cur.y + 1)) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x, cur.y + 1});
        }

        // move south: (x, y-1)
        if (
            cur.y - 1 >= 0 &&
            *(ptr + cur.x + cols * (cur.y - 1)) == 0 &&
            ((temp[cur.x][cur.y - 1] & S) == 0) &&
            ((temp[cur.x][cur.y]     & N) == 0)
        )
        {
            *(ptr + cur.x + cols * (cur.y - 1)) = *(ptr + cur.x + cols * cur.y) + 1;
            stack.push_front({cur.x, cur.y - 1});
        }
    }
}


void next(int* mat, int* walls, int x, int y, int angle, int* xp, int* yp, int* anglep)
{
    //this procedure checks where's the next optimal route (aka having the smallest value in the matrix)
    //then it goes that way

    //ex pseudo code for checking if north has the smallest value
        // if no wall in north
            // if (no wall in east) {if(north > east) {goto east}}
            // if (no wall in west) {if(north > east) {goto east}}
            // if (no wall in east) {if(north > east) {goto east}}
            // move north
        if( (*(walls + x + 16 * y)&(1<<2)) == 0)
        {
            
                if((*(walls + x + 16 * y)&(1<<3)) == 0) {
                    if(*(mat + x + 16 * (y - 1)) > *(mat + x + 1 + 16 * y)) {goto east;}
                }
                
                if((*(walls + x + 16 * y)&(1<<1)) == 0){
                    if( *(mat + x + 16 * (y - 1)) > *(mat + x - 1 + 16 * y)) {goto east;}
                }
                
                if((*(walls + x + 16 * y)&(1<<0)) == 0){
                    if(*(mat + x + 16 * (y - 1)) > *(mat + x + 16 * (y + 1))){goto east;}
                }    
                if(angle != 90) {turnTo(anglep, 90);}
                mf();
                updateCor(xp, yp, anglep);
                goto end;
        }

        // check east
        east:
        if( (*(walls + x + 16 * y)&(1<<3)) == 0)
        {
            
                if((*(walls + x + 16 * y)&(1<<2)) == 0) {
                    if(*(mat + x + 1 + 16 * y) > *(mat + x + 16 * (y - 1))) {goto south;}
                }
                
                if((*(walls + x + 16 * y)&(1<<1)) == 0){
                    if( *(mat + x + 1 + 16 * y) > *(mat + x - 1 + 16 * y)) {goto south;}
                }
                
                if((*(walls + x + 16 * y)&(1<<0)) == 0){
                    if(*(mat + x + 1 + 16 * y) > *(mat + x + 16 * (y + 1))){goto south;}
                }    
                if(angle != 0) {turnTo(anglep, 0);}
                mf();
                updateCor(xp, yp, anglep);
                goto end;
        }

        // check south
        south:
        if( (*(walls + x + 16 * y)&(1<<0)) == 0)
        {
            
                if((*(walls + x + 16 * y)&(1<<3)) == 0) {
                    if(*(mat + x + 16 * (y + 1)) > *(mat + x + 1 + 16 * y)) {goto west;}
                }
                
                if((*(walls + x + 16 * y)&(1<<1)) == 0){
                    if( *(mat + x + 16 * (y + 1)) > *(mat + x - 1 + 16 * y)) {goto west;}
                }
                
                if((*(walls + x + 16 * y)&(1<<2)) == 0){
                    if(*(mat + x + 16 * (y + 1)) > *(mat + x + 16 * (y - 1))){goto west;}
                }    
                if(angle != 270) {turnTo(anglep, 270);}
                mf();
                updateCor(xp, yp, anglep);
                goto end;
        }

        // check west
        west:
        if( (*(walls + x + 16 * y)&(1<<1)) == 0)
        {
            
                if((*(walls + x + 16 * y)&(1<<3)) == 0) {
                    if(*(mat + x - 1 + 16 * y) > *(mat + x + 1 + 16 * y)) {goto end;}
                }
                
                if((*(walls + x + 16 * y)&(1<<2)) == 0){
                    if( *(mat + x - 1 + 16 * y) > *(mat + x + 16 * (y - 1))) {goto end;}
                }
                
                if((*(walls + x + 16 * y)&(1<<0)) == 0){
                    if(*(mat + x - 1 + 16 * y) > *(mat + x + 16 * (y + 1))){goto end;}
                }    
                if(angle != 180) {turnTo(anglep, 180);}
                mf();
                updateCor(xp, yp, anglep);
                goto end;
        }
    
    end:
    log("(ツ)\n");
}

int checkEnd(int x, int y)
{
    if(
        x == 7 && (y == 7 || y == 8) ||
        x == 8 && (y == 7 || y == 8)
    ){
        return 0;
    }
    else {
        return 1;
    }
}

int checkStart(int x, int y)
{
    if(x == 0 && y == 15){
        return 0;
    }
    else {
        return 1;
    }
}


void setWalls(int* walls, int* visited)
{
    //set all unvisited walls to 1
    //for the final run
    int m;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            m = 15-j;
            if( *(visited + i + 16*m) == 0)
            {
                //outer walls
                if(i == 15)
                {
                    *(walls + i + 16*m) |= (1<<3);
                    sw(i, j, 'e');
                }
                if(i == 0)
                {
                    *(walls + i + 16*m) |= (1<<1);
                    sw(i, j, 'w');
                }
                if(m == 15)
                {
                    *(walls + i + 16*m) |= (1<<0);
                    sw(i, j, 's');
                }
                if(m == 0)
                {
                    *(walls + i + 16*m) |= (1<<2);
                    sw(i, j, 'n');
                }

                //east
                if(i+1 < 16){
                if( (*(visited + i + 1 + 16*m) == 0))
                {
                    *(walls + i + 16*m) |= (1<<3);
                    *(walls + i + 1 + 16*m) |= (1<<1);
                    sw(i, j, 'e');
                }}

                //west
                if(i-1 >= 0){
                if( *(visited + i - 1 + 16*m) == 0)
                {
                    *(walls + i + 16*m) |= (1<<1);
                    *(walls + i - 1 + 16*m) |= (1<<3);
                    sw(i, j, 'w');
                }}

                //south
                if(m+1 < 16){
                if( *(visited + i + 16*(m+1)) == 0)
                {
                    *(walls + i + 16*m) |= (1<<0);
                    *(walls + i + 16*(m+1)) |= (1<<2);
                    sw(i, j, 's');
                }}

                //north
                if(m-1 >= 0){
                if( *(visited + i + 16*(m-1)) == 0)
                {
                    *(walls + i + 16*m) |= (1<<2);
                    *(walls + i + 16*(m-1)) |= (1<<0);
                    sw(i, j, 'n');
                }}

            }
        }
    }
}

int FloodFill(){
    int matrix[rows][cols]; 
    int walls[rows][cols];
    int visited[rows][cols];
    int x = 0, y = 15;
    int angle = 90;
    log("Running...\n");
    sc(0, 0, 'G');
    st(0, 0, "Start");

    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            matrix[i][j] = 0;
        }
    }
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            walls[i][j] = 0;
        }
    }
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            visited[i][j] = 0;
        }
    }

    //solve maze run
    while(checkEnd(x, y)){
        log("x:" + to_string(x) + " y:" + to_string(y) + "\n");
        visited[y][x] = 1;
        sc(x, 15-y, 'g');
        checkWalls(x ,y, angle, &walls[0][0]);
        fillmat(&matrix[0][0], &walls[0][0]);
        printmat(&matrix[0][0]);
        next(&matrix[0][0], &walls[0][0], x, y, angle, &x, &y, &angle);
    }
    sc(x, 15-y, 'g');
    st(x, 15-y, ":D");

    //go back run
    while(checkStart(x, y)){
        log("x:" + to_string(x) + " y:" + to_string(y) + "\n");
        visited[y][x] = 1;
        sc(x, 15-y, 'g');
        checkWalls(x ,y, angle, &walls[0][0]);
        fillmatStart(&matrix[0][0], &walls[0][0]);
        printmat(&matrix[0][0]);
        next(&matrix[0][0], &walls[0][0], x, y, angle, &x, &y, &angle);
    }

    setWalls(&walls[0][0], &visited[0][0]);
    fillmat(&matrix[0][0], &walls[0][0]);
    printmat(&matrix[0][0]);

    //speedrun :)
    while(checkEnd(x, y)){
        log("x:" + to_string(x) + " y:" + to_string(y) + "\n");
        sc(x, 15-y, 'b');
        next(&matrix[0][0], &walls[0][0], x, y, angle, &x, &y, &angle);
    }
    log("CONGRATS!! :D");

}