// Created by Daniil on 19/04/2024.
#ifndef UTILS_H
#define UTILS_H

#include "dynArray/dynamic_array.h"
#include "time.h"

#define WINDOW_TITLE "Ball sim"
#define FPS 144
#define PHYSICS_FPS 1000.0f
#define TIME_SPEED 1.0f
#define BALL_COUNT 2000
#define BOUNCE 0.95f
#define DRAG 0.003f
#define DENSITY 0.001f

typedef struct {
    int a;
    int b;
} intPair;
typedef struct {
    ArrayList *balls;
    ArrayList *indices;
    void* tpool;
    float* physTime;
    bool* threaded;
} physics_args;
typedef struct {
    ArrayList *balls;
    ArrayList *indices;
    float dt;
    intPair bounds;
    int column;
    bool debugMsg;
    bool reversed;
} customArgs;

int max(int x, int y);
int min(int x, int y);
float maxf(float x, float y);
float minf(float x, float y);

void swap(BallToIntPair * a, BallToIntPair * b);

extern const int STEPS_PER_FRAME;
extern const float PHYSICS_DURATION;
extern const float FRAME_TIME;
extern const Vector2 GRAVITY;
extern const Vector2 R_RANGE;
extern const int R_GRID_FACTOR;
extern intPair BOUNDS;

void sortByCellHash(ArrayList* arrayList);

int computeCellHash(intPair p);//x and y must be >= 1 when the function is called

intPair gridPosFromBall(Ball* ball, intPair bounds);

void computeStartIndices(ArrayList* ballPairList, ArrayList* indicesList, intPair bounds);

void updateBallsCellHashes(ArrayList* ballPairList, intPair bounds);

Ball createBall(int id, Vector2 pos, Vector2 vel, Vector2 acc, float radius, float mass, Color color);

Ball createRandBallWithId(int id, intPair bounds);

void initBallsDynamic(ArrayList *balls, intPair bounds);

void initLinearGrid(ArrayList *grid, intPair bounds);

Vector2 parseCoord(Vector2 v, intPair bounds);

void initThreads(ArrayList* threads, int count);

#endif