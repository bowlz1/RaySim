#ifndef BALL_H_
#define BALL_H_

#include "raylib.h"
#include "raymath.h"

typedef struct // 1px = 1mm
{
    int id;
    Vector2 pos;   // in px
    Vector2 vel;   // in px/ms
    Vector2 acc;   // in px/ms^2
    float radius; // in px
    float mass;   // in grams
    Color color;
} Ball;

typedef struct {
    Ball ball;
    int hash;
} BallToIntPair;

#endif