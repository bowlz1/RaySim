#include "utils.h"

const int STEPS_PER_FRAME = /*PHYSICS_FPS/FPS*/1;
const float PHYSICS_DURATION = (1000.0 / PHYSICS_FPS);
const float FRAME_TIME = 1000.0 / FPS;
const Vector2 GRAVITY = (Vector2){0/1E3, -9.80665 / 1E3};
const Vector2 R_RANGE = (Vector2){1, 5};
const int R_GRID_FACTOR = R_RANGE.y*8;
intPair BOUNDS = (intPair){1920, 960};

int max(int x, int y) {
    return x >= y ? x : y;
}

int min(int x, int y) {
    return x <= y ? x : y;
}

float maxf(float x, float y) {
    return x >= y ? x : y;
}

float minf(float x, float y) {
    return x <= y ? x : y;
}

void swap(BallToIntPair * a, BallToIntPair * b) {
    BallToIntPair temp = *a;
    *a = *b;
    *b = temp;
}

int partition(ArrayList* arrayList, int low, int high) {
    int pivot = ((BallToIntPair*)at(arrayList, low))->hash;
    int i = low;
    int j = high;

    while (i < j) {

        // condition 1: find the first element greater than
        // the pivot (from starting)
        while (((BallToIntPair*)at(arrayList, i))->hash <= pivot && i <= high - 1) {
            i++;
        }

        while (((BallToIntPair*)at(arrayList, j))->hash > pivot && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(((BallToIntPair*)at(arrayList, i)), ((BallToIntPair*)at(arrayList, j)));
        }
    }
    swap(((BallToIntPair*)at(arrayList, low)), ((BallToIntPair*)at(arrayList, j)));
    return j;
}

// QuickSort function
void quickSort(ArrayList* arrayList, int low, int high) {
    if (low < high) {
        // call Partition function to find Partition Index
        int partitionIndex = partition(arrayList, low, high);

        // Recursively call quickSort() for left and right
        // half based on partition Index
        quickSort(arrayList, low, partitionIndex - 1);
        quickSort(arrayList, partitionIndex + 1, high);
    }
}

void sortByCellHash(ArrayList* arrayList) {
    quickSort(arrayList, 0, arrayList->size-1);
}

int computeCellHash(intPair p) {
    //x and y must be >= 1 when the function is called
    //this pairing function goes around in a "square" starting from the origin.
    int m = max(p.a, p.b);
    return m * m + m + p.b - p.a;
}

intPair gridPosFromBall(Ball* ball, intPair bounds) {
    float numOfCellsX = floorf(((float) bounds.a)/(R_GRID_FACTOR));
    float numOfCellsY = floorf(((float) bounds.b)/(R_GRID_FACTOR));
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;

    float cellDimX = (float) bounds.a/numOfCellsX;
    float cellDimY = (float) bounds.b/numOfCellsY;

    intPair intPear = (intPair){(int) (ball->pos.x/cellDimX), (int) (ball->pos.y/cellDimY)};
    if (intPear.a >= (int) numOfCellsX) intPear.a = (int) numOfCellsX - 1;
    if (intPear.a < 0) intPear.a = 0;
    if (intPear.b >= (int) numOfCellsY) intPear.b = (int) numOfCellsY - 1;
    if (intPear.b < 0) intPear.b = 0;
    //clamping
    return intPear;
}

void computeStartIndices(ArrayList* ballPairList, ArrayList* indicesList, intPair bounds) {
//    printf("0\n");
    float numOfCellsX = floorf(((float) bounds.a)/(R_GRID_FACTOR));
    float numOfCellsY = floorf(((float) bounds.b)/(R_GRID_FACTOR));
    numOfCellsX = numOfCellsX < 1 ? 1 : numOfCellsX;
    numOfCellsY = numOfCellsY < 1 ? 1 : numOfCellsY;

    int indexCeil = max((int) numOfCellsX, (int) numOfCellsY) * max((int) numOfCellsX, (int) numOfCellsY);
//    printf("1\n");
    //reset indices list when recalculating
    clear(indicesList);
    int m1 = -1;
    for (int i = 0; i < indexCeil; i++) push(indicesList, (genericPtr)&m1);
//    printf("2\n");
    //assume ballPairList is sorted in ascending order by the cell hash
    int curCellHash = -1;

    for (int i = 0; i < ballPairList->size; i++) {
        BallToIntPair* buip = (BallToIntPair*)at(ballPairList, i);
        if (buip->hash != curCellHash) {
            setAt(indicesList, (genericPtr)&i, buip->hash);
            curCellHash = buip->hash;
        }
    }
//    printf("3\n");
//    for (int i = 0; i < indicesList->size; i++) printf("%i %i\n", i, *(int*)at(indicesList, i));
}

void updateBallsCellHashes(ArrayList* ballPairList, intPair bounds) {
    for (int i = 0; i < ballPairList->size; i++) {
        Ball* bptr = &(((BallToIntPair*)(at(ballPairList, i)))->ball);
        ((BallToIntPair*)(at(ballPairList, i)))->hash = computeCellHash(gridPosFromBall(bptr, bounds));
    }
}

Ball createBall(int id, Vector2 pos, Vector2 vel, Vector2 acc, float radius, float mass, Color color) {
    return (Ball){id, pos, vel, acc, radius, mass, color};
}

Ball createRandBallWithId(int id, intPair bounds) {
    float rad = GetRandomValue(R_RANGE.x, R_RANGE.y);
    return createBall(
            id,
            (Vector2){GetRandomValue(/*bounds.a/4*/0, bounds.a), GetRandomValue(0, bounds.b)},
            (Vector2){GetRandomValue(-2, 2), GetRandomValue(-2, 2)},
            Vector2Add((Vector2){0, 0}, GRAVITY),
//            (Vector2) {0,0},
            rad,
            rad*rad*PI*DENSITY,
            (Color){GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255}
    );
}

void initBallsDynamic(ArrayList *balls, intPair bounds) {
    int prevSize = balls->size;
    clear(balls);
    for (int i = 0; i < (prevSize > BALL_COUNT ? prevSize : BALL_COUNT); i++) {
        Ball b = createRandBallWithId(i, bounds);
        push(balls, (genericPtr) &b);
//        balls[i].vel = Vector2Zero();
    }
}

void initLinearGrid(ArrayList *grid, intPair bounds) {
    int prevSize = grid->size;
    clear(grid);
    for (int i = 0; i < (prevSize > BALL_COUNT ? prevSize : BALL_COUNT); i++) {
        Ball b = createRandBallWithId(i, bounds);
        BallToIntPair pear = {b, -1};//placeholder
        push(grid, (genericPtr) &pear);
    }
}

void initThreads(ArrayList* threads, int count) {
//    clear(threads);
    for (int i = 0; i < count; i++) {
        pthread_t* threadPtr = (pthread_t*)malloc(sizeof(pthread_t));
        push(threads, (genericPtr) threadPtr);
    }
}

Vector2 parseCoord(Vector2 v, intPair bounds) {
    return (Vector2){v.x, bounds.b - v.y};
}