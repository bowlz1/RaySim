#include "utils/utils.h"
#include "utils/tpool/pthread_pool.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* printTest(void* args) {
    printf("I am a thread! %llu\n", pthread_self());
}

void printBalls(ArrayList* balls) {
    printf("\n[");
    for (int i = 0; i < balls->size - 1; i++) {
        BallToIntPair *pear = (((BallToIntPair *) (at(balls, i))));
        printf("{%i}{Id: %i, Hash: %i, P(%.2f,%.2f), V(%.2f,%.2f), a(%.2f,%.2f), R: %.2lf, M: %.2lf}\n",
               i,
               pear->ball.id,
               pear->hash,
               pear->ball.pos.x,
               pear->ball.pos.y,
               pear->ball.vel.x,
               pear->ball.vel.y,
               pear->ball.acc.x,
               pear->ball.acc.y,
               pear->ball.radius,
               pear->ball.mass
        );
    }
    BallToIntPair *pear = (((BallToIntPair *) (at(balls, balls->size-1))));
    printf("{%i}{Id: %i, Hash: %i, P(%f.2,%.2f), V(%.2f,%.2f), a(%.2f,%.2f), R: %.2lf, M: %.2lf}]",
           balls->size-1,
           pear->ball.id,
           pear->hash,
           pear->ball.pos.x,
           pear->ball.pos.y,
           pear->ball.vel.x,
           pear->ball.vel.y,
           pear->ball.acc.x,
           pear->ball.acc.y,
           pear->ball.radius,
           pear->ball.mass
    );
}

void printIndices(ArrayList* indices) {
    printf("\n[");
    for (int i = 0; i < indices->size - 1; i++) {
        printf("{%i ->starts at-> %i}\n", i, *(int*)(at(indices, i)));
    }
    printf("{%i ->starts at-> %i}]", indices->size-1, *(int*)(at(indices, indices->size-1)));

}

void moveBall(Ball* ball1, float dt, bool reversed) {
    if (reversed) dt = -dt;
    float bouncyCoeff = (reversed ? 1.0f/BOUNCE : BOUNCE);
    float drag = (reversed ? -DRAG : DRAG);

//    if (ball1->id%2 == 0) {
        ball1->acc.x = .01 * cos(atan2((BOUNDS.b - GetMousePosition().y) - ball1->pos.y, GetMousePosition().x - ball1->pos.x));
        ball1->acc.y = .01 * sin(atan2((BOUNDS.b - GetMousePosition().y) - ball1->pos.y, GetMousePosition().x - ball1->pos.x));
//    }
//    if (ball1->id%2==1) {
//        ball1->acc.x = .01 * cos(atan2((BOUNDS.b - GetMousePosition().y) - ball1->pos.y, (BOUNDS.a - GetMousePosition().x) - ball1->pos.x));
//        ball1->acc.y = .01 * sin(atan2(GetMousePosition().y - ball1->pos.y, GetMousePosition().x - ball1->pos.x));
//    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        ball1->acc.x *= -1;
        ball1->acc.y *= -1;
    }
    //update vel
    ball1->vel.x = ball1->vel.x * (1 - drag * dt) + ball1->acc.x * dt;
    ball1->vel.y = ball1->vel.y * (1 - drag * dt) + ball1->acc.y * dt;
    //update pos
    ball1->pos.x = ball1->pos.x + ball1->vel.x * dt + 0.5 * ball1->acc.x * dt * dt;
    ball1->pos.y = ball1->pos.y + ball1->vel.y * dt + 0.5 * ball1->acc.y * dt * dt;

    //wall collision
    if (ball1->pos.x + ball1->radius > BOUNDS.a) {
        ball1->vel.x = -ball1->vel.x * bouncyCoeff;
        ball1->pos.x = BOUNDS.a - ball1->radius;
    }
    if (ball1->pos.x - ball1->radius < 0) {
        ball1->vel.x = -ball1->vel.x * bouncyCoeff;
        ball1->pos.x = ball1->radius;
    }
    if (ball1->pos.y + ball1->radius > BOUNDS.b) {
        ball1->vel.y = -ball1->vel.y * bouncyCoeff;
        ball1->pos.y = BOUNDS.b - ball1->radius;
    }
    if (ball1->pos.y - ball1->radius < 0) {
        ball1->vel.y = -ball1->vel.y * bouncyCoeff;
        ball1->pos.y = ball1->radius;
    }
}

void collideBalls(Ball* ball1, Ball* ball2, bool reversed) {
    if (ball1->id == ball2->id) return;
    float bouncyCoeff = (reversed ? 1.0f/BOUNCE : BOUNCE);
    Vector2 dist = Vector2Subtract(ball2->pos, ball1->pos);
    float distance = Vector2Length(dist);
    float overlap = ball1->radius + ball2->radius - distance;
    if (overlap > 0) {
        //push out
        float angle = atan2(dist.y, dist.x);
        ball1->pos.x -= cos(angle) * overlap/2;
        ball1->pos.y -= sin(angle) * overlap/2;
        ball2->pos.x += cos(angle) * overlap/2;
        ball2->pos.y += sin(angle) * overlap/2;
        //recalc dist and distance
        dist = Vector2Subtract(ball2->pos, ball1->pos);
        distance = Vector2Length(dist);
        //masses
        float mass1 = ball1->radius * ball1->radius * PI * DENSITY;
        float mass2 = ball2->radius * ball2->radius * PI * DENSITY;

        Vector2 unifNormal = (Vector2) {(ball2->pos.x - ball1->pos.x) / distance,
                                        (ball2->pos.y - ball1->pos.y) / distance};
        Vector2 unifTangent = (Vector2) {-unifNormal.y, unifNormal.x};

        float v1Norm = unifNormal.x * ball1->vel.x + unifNormal.y * ball1->vel.y;
        float v1Tang = unifTangent.x * ball1->vel.x + unifTangent.y * ball1->vel.y;
        float v2Norm = unifNormal.x * ball2->vel.x + unifNormal.y * ball2->vel.y;
        float v2Tang = unifTangent.x * ball2->vel.x + unifTangent.y * ball2->vel.y;

        float v1NormPost = ((v1Norm * (mass1 - mass2) + 2 * mass2 * v2Norm) / (mass1 + mass2)) * bouncyCoeff;
        float v2NormPost = ((v2Norm * (mass2 - mass1) + 2 * mass1 * v1Norm) / (mass1 + mass2)) * bouncyCoeff;

        ball1->vel.x = v1NormPost * unifNormal.x + v1Tang * unifTangent.x;
        ball1->vel.y = v1NormPost * unifNormal.y + v1Tang * unifTangent.y;
        ball2->vel.x = v2NormPost * unifNormal.x + v2Tang * unifTangent.x;
        ball2->vel.y = v2NormPost * unifNormal.y + v2Tang * unifTangent.y;
    }
}

void recalcBalls(ArrayList* balls, ArrayList* indices, intPair bounds) {
    updateBallsCellHashes(balls, bounds);//O(balls)
    sortByCellHash(balls);//O(balls * log(balls))
    computeStartIndices(balls, indices, bounds);//O(maxCells + balls)
}

void updateBallsGrid(ArrayList *balls, ArrayList* indices, float dt, intPair bounds, bool debugMsg, bool reversed) {//roughly O(balls^1.5)
    recalcBalls(balls, indices, bounds);
    if (debugMsg) {
        printf("\n--------------------------------------------------------------------------------");
        printBalls(balls);
        printIndices(indices);
    }
    int numOfCellsX = (int) floorf(((float) bounds.a)/(R_GRID_FACTOR));
    int numOfCellsY = (int) floorf(((float) bounds.b)/(R_GRID_FACTOR));
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;
    if (debugMsg) printf("\nX and Y number of cells: %i,%i",numOfCellsX, numOfCellsY);
    for (int x = 0; x < numOfCellsX; x++) {
        for (int y = 0; y < numOfCellsY; y++) {
            //calc current cell hash
            int cellHash1 = computeCellHash((intPair) {x, y});
            int* startIndex1 = (int*)(at(indices, cellHash1));
            if (debugMsg) printf("\n%i %i:%i %i",*startIndex1, x, y, cellHash1);
            if (startIndex1 == NULL || *startIndex1 < 0) {
                if (debugMsg) printf("\tNo balls in cell %i;%i", x, y);
                continue;
            }
            int offset1 = 0;
            while (*startIndex1 + offset1 < balls->size && ((BallToIntPair*)(at(balls, *startIndex1 + offset1)))->hash == cellHash1) {//while the current ball still belongs to cell number i
                Ball* ball1 = &(((BallToIntPair*)(at(balls, *startIndex1 + offset1)))->ball);
                if (debugMsg) printf("\n\tBall hash: %i, id: %i, SI+off: %i", ((BallToIntPair*)(at(balls, *startIndex1 + offset1)))->hash, ball1->id, *startIndex1+offset1);
                moveBall(ball1, dt, reversed);
                for (int dx = -1; dx <= 1; dx++) {
                    int dxx = x + dx;
                    if (dxx < 0 || dxx >= numOfCellsX) continue;
                    for (int dy = -1; dy <= 1; dy++) {
                        int dyy = y + dy;
                        if (dyy < 0 || dyy >= numOfCellsY) continue;
                        //check collisions with balls in current cell and valid adjacent cells
                        int cellHash2 = computeCellHash((intPair) {dxx, dyy});
                        int* startIndex2 = (int*)(at(indices, cellHash2));
                        if (startIndex2 == NULL || *startIndex2 < 0) continue;
                        int offset2 = 0;
                        while (*startIndex2 + offset2 < balls->size && ((BallToIntPair*)(at(balls, *startIndex2 + offset2)))->hash == cellHash2) {//while the current ball still belongs to cell number i
                            Ball *ball2 = &(((BallToIntPair *) (at(balls, *startIndex2 + offset2)))->ball);
                            collideBalls(ball1, ball2, reversed);
                            offset2 += 1;
                        }
                    }
                }
                offset1 += 1;
            }
        }
    }
}

void* updateColumn(void* args) {
    customArgs *cArgs = (customArgs *) args;
    ArrayList *balls = cArgs->balls;
    ArrayList *indices = cArgs->indices;
    float dt = cArgs->dt;
    intPair bounds = cArgs->bounds;
    int column = cArgs->column;
    bool debugMsg = cArgs->debugMsg;
    bool reversed = cArgs->reversed;

    int numOfCellsX = (int) floorf(((float) bounds.a) / (R_GRID_FACTOR));
    int numOfCellsY = (int) floorf(((float) bounds.b) / (R_GRID_FACTOR));
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;

    if (column >= numOfCellsX || column < 0) {
        if (debugMsg) printf("column %d out of range, thread id: %llu\n", column, pthread_self());
        return NULL;
    }

    if (debugMsg) {
        printf("\n-------------------------------------------------------------------------------");
        printBalls(balls);
        printIndices(indices);
    }

    int x = column;
    if (debugMsg) printf("\nColumn %i, of height %i", x, numOfCellsY);

    for (int y = 0; y < numOfCellsY; y++) {
        //calc current cell hash
        int cellHash1 = computeCellHash((intPair) {x, y});
        int *startIndex1 = (int *) (at(indices, cellHash1));
        if (debugMsg) printf("\n%i %i:%i %i", *startIndex1, x, y, cellHash1);
        if (startIndex1 == NULL || *startIndex1 < 0) {
            if (debugMsg) printf("\tNo balls in cell %i;%i", x, y);
            continue;
        }
        int offset1 = 0;
        while (*startIndex1 + offset1 < balls->size && ((BallToIntPair *) (at(balls, *startIndex1 + offset1)))->hash ==
                                                       cellHash1) {//while the current ball still belongs to cell number i
            Ball *ball1 = &(((BallToIntPair *) (at(balls, *startIndex1 + offset1)))->ball);
            if (debugMsg)
                printf("\n\tBall hash: %i, id: %i, SI+off: %i",
                       ((BallToIntPair *) (at(balls, *startIndex1 + offset1)))->hash, ball1->id,
                       *startIndex1 + offset1);
            moveBall(ball1, dt, reversed);
            for (int dx = -1; dx <= 1; dx++) {
                int dxx = x + dx;
                if (dxx < 0 || dxx >= numOfCellsX) continue;
                for (int dy = -1; dy <= 1; dy++) {
                    int dyy = y + dy;
                    if (dyy < 0 || dyy >= numOfCellsY) continue;
                    //check collisions with balls in current cell and valid adjacent cells
                    int cellHash2 = computeCellHash((intPair) {dxx, dyy});
                    int *startIndex2 = (int *) (at(indices, cellHash2));
                    if (startIndex2 == NULL || *startIndex2 < 0) continue;
                    int offset2 = 0;
                    while (*startIndex2 + offset2 < balls->size &&
                           ((BallToIntPair *) (at(balls, *startIndex2 + offset2)))->hash ==
                           cellHash2) {//while the current ball still belongs to cell number i
                        Ball *ball2 = &(((BallToIntPair *) (at(balls, *startIndex2 + offset2)))->ball);
                        collideBalls(ball1, ball2, reversed);
                        offset2 += 1;
                    }
                }
            }
            offset1 += 1;
        }
    }
}

void updateBallsThreaded(ArrayList *balls, ArrayList* indices, void * tpool, float dt, intPair bounds, bool debugMsg, bool reversed) {//roughly O(balls^1.5)
    PollInputEvents();
    unsigned int threadCount = ((pool*) tpool)->nthreads;
    recalcBalls(balls, indices, bounds);
    for (int a = 0; a < 3; a++) {
        for (unsigned int k = 0; k < threadCount; k++) {
            customArgs* args = (customArgs*) malloc(sizeof(customArgs));
            args->balls = balls;
            args->indices = indices;
            args->dt = dt;
            args->bounds = bounds;
            args->column = 3*k+a;
            args->debugMsg = debugMsg;
            args->reversed = reversed;
            pool_enqueue(tpool, args, 1);
        }
    }
    pool_wait(tpool);
}

_Noreturn void* physicsWin11(void* args) {
    printf("Physics thread started\n");
    bool debugMsg = false;
    bool manual = false;
    physics_args *p_args = (physics_args *)args;
    bool* threaded = p_args->threaded;
    ArrayList* balls = p_args->balls;
    ArrayList* indices = p_args->indices;
    void* tpool = p_args->tpool;
    float dtt = TIME_SPEED*(PHYSICS_DURATION);
    float numOfCellsX = floorf(((float) BOUNDS.a)/(R_GRID_FACTOR));
    float numOfCellsY = floorf(((float) BOUNDS.b)/(R_GRID_FACTOR));
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;

    clock_t frameClock, physClock;
    float simTime = 0.0;

    frameClock = clock() + PHYSICS_DURATION;
    while (true) {
        if (clock()-frameClock >= PHYSICS_DURATION - simTime) {
            physClock = clock();

//            PollInputEvents();

            if (BOUNDS.a != GetScreenWidth()) {
                BOUNDS.a = GetScreenWidth();
                numOfCellsX = floorf(((float) BOUNDS.a)/(R_GRID_FACTOR));
                numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
                pool_end(tpool);
                tpool = pool_start(updateColumn, (int)ceilf(numOfCellsX/3.0f));
            }
            if (BOUNDS.b != GetScreenHeight()) {
                BOUNDS.b = GetScreenHeight();
                numOfCellsY = floorf(((float) BOUNDS.b)/(R_GRID_FACTOR));
                numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;
            }

            if (IsKeyPressed(KEY_Q)) {
                pthread_mutex_lock(&mutex);

//                printf("pressed q %d\n", clock());
                initBallsDynamic(balls, BOUNDS);
                initLinearGrid(balls, BOUNDS);

                pthread_mutex_unlock(&mutex);
            }
            if (IsKeyPressed(KEY_W) || IsKeyDown(KEY_S)) {
                pthread_mutex_lock(&mutex);

                Ball ballToSpawn = createRandBallWithId(balls->size, BOUNDS);
                BallToIntPair pear = (BallToIntPair){ballToSpawn, computeCellHash(gridPosFromBall(&ballToSpawn, BOUNDS))};
                push(balls, (genericPtr) &pear);

                pthread_mutex_unlock(&mutex);
            }
            if (IsKeyDown(KEY_E) && balls->size > 0) {
                pthread_mutex_lock(&mutex);

                pop(balls);

                pthread_mutex_unlock(&mutex);
            }
            if (IsKeyPressed(KEY_RIGHT_SHIFT)) debugMsg = true;
            if (IsKeyReleased(KEY_RIGHT_SHIFT)) debugMsg = false;
            if (IsKeyPressed(KEY_M)) {
//                printf("aboba %d\n", clock());
                manual = !manual;
            }
            if (IsKeyPressed(KEY_TAB)) if (threaded != NULL) *threaded = !*threaded;

            //do physics
            if (manual) {
                if (IsKeyPressed(KEY_LEFT)) {
                    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
                        for (int s = 0; s < STEPS_PER_FRAME; s++) {
                            if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, true);
                            else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, true);
                        }
                    }
                    else if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, true);
                    else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, true);
                } else if (IsKeyPressed(KEY_RIGHT)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        for (int s = 0; s < STEPS_PER_FRAME; s++) {
                            if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, false);
                            else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, false);
                        }
                    }
                    if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, false);
                    else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, false);
                } else if (IsKeyDown(KEY_DOWN)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        for (int s = 0; s < STEPS_PER_FRAME; s++) {
                            if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, true);
                            else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, true);
                        }
                    }
                    if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, true);
                    else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, true);
                } else if (IsKeyDown(KEY_UP)) {
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        for (int s = 0; s < STEPS_PER_FRAME; s++) {
                            if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, false);
                            else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, false);
                        }
                    }
                    if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, false);
                    else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, false);
                }
            } else if (!manual) {
                for (int s = 0; s < STEPS_PER_FRAME; s++) {
                    if (threaded) updateBallsThreaded(balls, indices, tpool, dtt, BOUNDS, debugMsg, false);
                    else updateBallsGrid(balls, indices, dtt, BOUNDS, debugMsg, false);
                }
            }
            //

            simTime = clock() - physClock;//time necessary to compute physics
            *p_args->physTime = simTime;
            frameClock = clock();//reset frame timer
        }
    }
}
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
//    SetConfigFlags(FLAG_MSAA_4X_HINT);      // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(BOUNDS.a, BOUNDS.b, WINDOW_TITLE);
//    ToggleFullscreen();
//    SetConfigFlags(FLAG_VSYNC_HINT);
    SetTargetFPS(FPS);

    // grid attempt
    ArrayList* linearGrid = init(sizeof(BallToIntPair));
    initLinearGrid(linearGrid, BOUNDS);

    ArrayList* startIndices = init(sizeof(int));
    computeStartIndices(linearGrid, startIndices, BOUNDS);

    float numOfCellsX = floorf(((float) BOUNDS.a)/(R_GRID_FACTOR));
    float numOfCellsY = floorf(((float) BOUNDS.b)/(R_GRID_FACTOR));
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;

    bool grid = false;
    bool tooltips = false;
    bool threaded = true;
    bool render = true;
    bool bg = false;

    void* tpool = pool_start(updateColumn, (unsigned int)ceilf(numOfCellsX/3.0f));

    float physTime = 0.0f;
    pthread_t physics_thread;
    pthread_create(&physics_thread, NULL, physicsWin11, (void *)&(physics_args){linearGrid, startIndices, tpool, &physTime, &threaded});

    clock_t physClock, renderClock;

    printf("Keybinds: [DON'T REALLY WORK FOR NOW]"
           "\nQ -> generate new balls"
           "\nW -> spawn one ball"
           "\nS -> hold to spawn balls"
           "\nE -> hold to remove balls"
           "\nM -> toggle manual mode"
           "\nG -> toggle grid"
           "\nT -> toggle tooltips"
           "\nB -> cycle background color"
           "\nH -> cycle ball hue gradient"
           "\nTab -> toggle threaded mode"
           "\nRShift -> enable debug messages while held"
           "\nUP -> hold to advance by one step"
           "\nDOWN -> hold go back by one step"
           "\nRIGHT -> tap to advance by one step"
           "\nLEFT -> tap to go back by one step"
           "\nLShift -> hold to make the arrow keys move by one frame instead"
           );
    //    printf("le palle end\n");
    float hue = 0;
    float coeff = 1.0f;
    while (!WindowShouldClose()) {
        /*if (BOUNDS.a != GetScreenWidth()) {
            BOUNDS.a = GetScreenWidth();
            numOfCellsX = floorf(((float) BOUNDS.a)/(R_GRID_FACTOR));
            numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
            pool_end(tpool);
            tpool = pool_start(updateColumn, (int)ceilf(numOfCellsX/3.0f));
        }
        if (BOUNDS.b != GetScreenHeight()) {
            BOUNDS.b = GetScreenHeight();
            numOfCellsY = floorf(((float) BOUNDS.b)/(R_GRID_FACTOR));
            numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;
        }*/
//        printf("le palle 3\n");
        BeginDrawing();
        if (IsKeyPressed(KEY_B)) bg = !bg;
        if (bg) ClearBackground(RAYWHITE);
        else ClearBackground((Color) {20,20,20,255});
        /*if (IsKeyPressed(KEY_Q)) {
            initBallsDynamic(linearGrid, BOUNDS);
            initLinearGrid(linearGrid, BOUNDS);
        }
         if (IsKeyPressed(KEY_W) || IsKeyDown(KEY_S)) {
            Ball ballToSpawn = createRandBallWithId(linearGrid->size, BOUNDS);
            BallToIntPair pear = (BallToIntPair){ballToSpawn, computeCellHash(gridPosFromBall(&ballToSpawn, BOUNDS))};
            push(linearGrid, (genericPtr) &pear);
         }
        if (IsKeyDown(KEY_E) && linearGrid->size > 0) {
            pop(linearGrid);
        }
        if (IsKeyPressed(KEY_RIGHT_SHIFT)) debugMsg = true;
        if (IsKeyReleased(KEY_RIGHT_SHIFT)) debugMsg = false;
        if (IsKeyPressed(KEY_M)) manual = !manual;*/
        if (IsKeyPressed(KEY_G)) grid = !grid;
        if (IsKeyPressed(KEY_T)) tooltips = !tooltips;
        if (IsKeyPressed(KEY_R)) render = !render;
//        if (IsKeyPressed(KEY_TAB)) threaded = !threaded;


        float cellDimX = (float) BOUNDS.a/numOfCellsX;
        float cellDimY = (float) BOUNDS.b/numOfCellsY;
//        printf("le palle 4\n");
        //local physics here
//        physClock = clock();
////        delete(threads);
////        threads = init(sizeof(pthread_t));
//        physicsLocal(linearGrid, startIndices, tpool, manual, debugMsg, threaded);
//        long physDiff = clock()-physClock;
        //local physics here
        renderClock = clock();
        float base = 30.0f;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyDown(KEY_H)) {
            if (IsKeyDown(KEY_H)) base = 3;
            hue += base*coeff;
//                printf("increased hue\n");
            if (hue > 360.0f) hue = 0.0f;
        }
        for (unsigned int i = 0; i < linearGrid->size; ++i) {
            pthread_mutex_lock(&mutex);
            Ball* ball = &((BallToIntPair *) at(linearGrid, i))->ball;
            if (ball == NULL) continue;
            // draw the ball
            float mouseDist1 = fabsf(Vector2Distance(ball->pos, parseCoord(GetMousePosition(), BOUNDS)));
            float mouseDist2 = fabsf(Vector2Distance(ball->pos, (Vector2){BOUNDS.a - GetMousePosition().x, parseCoord(GetMousePosition(), BOUNDS).y}));
            float maxDist = sqrt(BOUNDS.a * BOUNDS.a + BOUNDS.b * BOUNDS.b)/4;
            float grad = 120.0f*(minf(mouseDist1, maxDist)/maxDist);
//            grad += 120.0f*(minf(mouseDist2, maxDist)/maxDist);
//            float grad = 120.0f*mouseDist/maxDist;
            float localHue = hue + fabsf(grad);

            ball->color = ColorFromHSV(localHue,1.0f,1.0f);
            if (render) {
                DrawCircleV(parseCoord(ball->pos, BOUNDS), ball->radius, ball->color);
            }

            // draw the velocity
            if (tooltips) {
                DrawLineV(parseCoord(ball->pos, BOUNDS), Vector2Add(parseCoord(ball->pos, BOUNDS), Vector2Multiply(ball->vel, (Vector2){50, -50})), BLACK);
                intPair gridPos = gridPosFromBall(ball, BOUNDS);
                DrawText(TextFormat("%i\n%i:%i", ball->id, gridPos.a, gridPos.b), ball->pos.x,
                         parseCoord(ball->pos, BOUNDS).y - 20, 20, BLACK);
            }
            pthread_mutex_unlock(&mutex);
        }

        if (grid) {
            for (int i = 0; (int) (i*cellDimX) < BOUNDS.a; i++) {
                DrawLine(i*cellDimX, 0, i*cellDimX, BOUNDS.b, bg ? BLACK : RAYWHITE);
            }

            for (int j = 0; (int) (j*cellDimY) < BOUNDS.b; j++) {
                DrawLine(0, j*cellDimY, BOUNDS.a, j*cellDimY, bg ? BLACK : RAYWHITE);
            }
        }

        time_t diff = clock() - renderClock;
        // title
        const char *text = TextFormat("Balls: %i, Indices: %i", linearGrid->size, startIndices->size);
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
        DrawText(text, BOUNDS.a / 2 - text_size.x / 2, 5, 20, bg ? BLACK : RAYWHITE);
        // info
        const float frameTime = GetFrameTime() * 1000;
        DrawText(TextFormat("ms: %02.02f", frameTime), 5, 5, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("expected: %02.02f",FRAME_TIME), 5, 25, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("diff: %02.02f", frameTime-FRAME_TIME), 5, 45, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("sim time: %02.02f", (float)physTime), 5, 65, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("render time: %02.02f", (float)diff), 5, 85, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("FPS: %i", GetFPS()), 5, 105, 20, bg ? BLACK : RAYWHITE);
        DrawText(TextFormat("%sthreaded", threaded ? "" : "not "), 5, 125, 20, bg ? BLACK : RAYWHITE);
        if (threaded) DrawText(TextFormat("%d threads", ((pool*)tpool)->nthreads), 5, 145, 20, bg ? BLACK : RAYWHITE);
//        DrawText(TextFormat("PHYSICS FPS: %02.02f", 1000/phys_time), 200, 5, 20, bg ? BLACK : RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}