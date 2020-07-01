#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Sound.h"

typedef struct walls {
    wall_t **walls;

    unsigned int wall_count;
} walls_t;

walls_t walls = { 0 };

wall_t *createWall(signed short x1, signed short y1, signed short w,
                   signed short h, float dampening, unsigned int colour,
                   void (*callback)(void *), void *args)
{
    wall_t *ret = calloc(1, sizeof(wall_t));

    if (!ret) {
        fprintf(stderr, "Creating wall failed\n");
        exit(EXIT_FAILURE);
    }

    ret->x1 = x1;
    ret->y1 = y1;
    ret->w = w;
    ret->h = h;
    ret->x2 = x1 + w;
    ret->y2 = y1 + h;
    ret->colour = colour;
    ret->dampening = dampening;
    ret->callback = callback;
    ret->args = args;

    walls.wall_count++;
    walls.walls = realloc(walls.walls, sizeof(wall_t *) * walls.wall_count);

    if (!walls.walls) {
        fprintf(stderr, "Increasing walls list to %u walls failed\n",
                walls.wall_count);
        exit(EXIT_FAILURE);
    }

    walls.walls[walls.wall_count - 1] = ret;

    return ret;
}

void setWallProperty(wall_t *wall, signed short x, signed short y,
                     signed short width, signed short height,
                     unsigned char flags)
{
    if (flags & 1) { // Set X
        wall->x1 = x;
        wall->x2 = x + wall->w;
    }
    if ((flags >> 1) & 1) { // Set Y
        wall->y1 = y;
        wall->y2 = y + wall->h;
    }
    if ((flags >> 2) & 1) { // Set width
        wall->w = width;
        wall->x2 = wall->x1 + width;
    }
    if ((flags >> 3) & 1) { // Set height
        wall->h = height;
        wall->y2 = wall->y1 + height;
    }
}

ball_t *createBall(signed short initial_x, signed short initial_y,
                   unsigned int colour, signed short radius, float max_speed,
                   void (*callback)(void *), void *args)
{
    ball_t *ret = calloc(1, sizeof(ball_t));

    if (!ret) {
        fprintf(stderr, "Creating ball failed\n");
        exit(EXIT_FAILURE);
    }

    ret->x = initial_x;
    ret->y = initial_y;
    ret->f_x = initial_x;
    ret->f_y = initial_y;
    ret->max_speed = max_speed;
    ret->colour = colour;
    ret->radius = radius;
    ret->callback = callback;
    ret->args = args;

    return ret;
}

void setBallSpeed(ball_t *ball, float dx, float dy, float max_speed,
                  unsigned char flags)
{
    if (flags & 1) // Set X
        if (abs(dx) <= ball->max_speed) {
            ball->dx = dx;
        }
    if ((flags >> 1) & 1) // Set y
        if (abs(dy) <= ball->max_speed) {
            ball->dy = dy;
        }
    if ((flags >> 2) & 1) { // Set max speed
        ball->max_speed = max_speed;
    }
}

#define SET_BALL_COORD(AXIS, VAL)                                              \
    ball->AXIS = VAL;                                                      \
    ball->f_##AXIS = VAL;

void setBallLocation(ball_t *ball, signed short x, signed short y)
{
    if (x < ball->radius) {
        SET_BALL_COORD(x, ball->radius);
    }
    else if (x > SCREEN_WIDTH - ball->radius) {
        SET_BALL_COORD(x, SCREEN_WIDTH - ball->radius);
    }
    else {
        SET_BALL_COORD(x, x)
    }

    if (y < ball->radius) {
        SET_BALL_COORD(y, ball->radius);
    }
    else if (y > SCREEN_HEIGHT - ball->radius) {
        SET_BALL_COORD(y, SCREEN_HEIGHT - ball->radius);
    }
    else {
        SET_BALL_COORD(y, y)
    }
}

void updateBallPosition(ball_t *ball, unsigned int milli_seconds)
{
    float update_interval = milli_seconds / 1000.0;
    ball->f_x += ball->dx * update_interval;
    ball->f_y += ball->dy * update_interval;
    ball->x = round(ball->f_x);
    ball->y = round(ball->f_y);
}

#define HORIZONTAL 0b1
#define VERTICAL 0b10

void changeBallDirection(ball_t *ball, signed char direction, float dampening)
{
    if (direction & 1) {
        ball->dx *= -1;
        setBallSpeed(ball, ball->dx * (1 + dampening), 0, 0,
                     SET_BALL_SPEED_X);
    }

    if ((direction >> 1) & 1) {
        ball->dy *= -1;
        setBallSpeed(ball, 0, ball->dy * (1 + dampening), 0,
                     SET_BALL_SPEED_Y);
    }
}

#define COLLIDE_WALL 1
#define COLLIDE_BALL 2

#define COLLIDE_WALL_TOP 1
#define COLLIDE_WALL_BOTTOM 2
#define COLLIDE_WALL_LEFT 3
#define COLLIDE_WALL_RIGHT 4

#define BALL_LEFT_POINT_X ball->f_x - ball->radius
#define BALL_LEFT_POINT_Y ball->f_y
#define BALL_RIGHT_POINT_X ball->f_x + ball->radius
#define BALL_RIGHT_POINT_Y ball->f_y
#define BALL_TOP_POINT_X ball->f_x
#define BALL_TOP_POINT_Y ball->f_y - ball->radius
#define BALL_BOTTOM_POINT_X ball->f_x
#define BALL_BOTTOM_POINT_Y ball->f_y + ball->radius

#define WALL ((wall_t *)object)
#define BALL ((ball_t *)object)

signed char collideWall(ball_t *ball, wall_t *wall,
                        unsigned char collision_type, void (*callback)(void *),
                        void *args)
{
    if (callback) {
        callback(args);
    }
    if (wall->callback) {
        wall->callback(wall->args);
    }
    if (ball->callback) {
        ball->callback(wall->args);
    }
    switch (collision_type) {
        case COLLIDE_WALL_TOP:
        case COLLIDE_WALL_BOTTOM:
            changeBallDirection(ball, VERTICAL, wall->dampening);
            break;
        case COLLIDE_WALL_RIGHT:
        case COLLIDE_WALL_LEFT:
            changeBallDirection(ball, HORIZONTAL, wall->dampening);
            break;
        default:
            break;
    }

    return 0;
}

signed char handleCollision(ball_t *ball, void *object, unsigned char flag,
                            void (*callback)(void *), void *args)
{
    unsigned char ret = 0;
    switch (flag) {
        case COLLIDE_WALL:
            // Coming from:
            // Above wall
            if (BALL_BOTTOM_POINT_Y >= WALL->y1 &&
                BALL_TOP_POINT_Y < WALL->y1 &&
                BALL_BOTTOM_POINT_X >= WALL->x1 &&
                BALL_BOTTOM_POINT_X <= WALL->x2 && ball->dy > 0) {
                // Place ball next to wall to prevent ball getting stuck in wall
                ball->f_y = WALL->y1 - ball->radius;
                collideWall(ball, WALL, COLLIDE_WALL_TOP, callback,
                            args);
                ret = 1;
            }
            // Below wall
            if (BALL_TOP_POINT_Y <= WALL->y2 &&
                BALL_BOTTOM_POINT_Y > WALL->y2 &&
                BALL_TOP_POINT_X >= WALL->x1 &&
                BALL_TOP_POINT_X <= WALL->x2 && ball->dy < 0) {
                ball->f_y = WALL->y2 + ball->radius;
                collideWall(ball, WALL, COLLIDE_WALL_BOTTOM, callback,
                            args);
                ret = 1;
            }
            // Left of wall
            if (BALL_RIGHT_POINT_X >= WALL->x1 &&
                BALL_LEFT_POINT_X < WALL->x1 &&
                BALL_RIGHT_POINT_Y >= WALL->y1 &&
                BALL_RIGHT_POINT_Y <= WALL->y2 && ball->dx > 0) {
                ball->f_x = WALL->x1 - ball->radius;
                collideWall(ball, WALL, COLLIDE_WALL_RIGHT, callback,
                            args);
                ret = 1;
            }
            // Right of wall
            if (BALL_LEFT_POINT_X <= WALL->x2 &&
                BALL_RIGHT_POINT_X > WALL->x2 &&
                BALL_LEFT_POINT_Y >= WALL->y1 &&
                BALL_RIGHT_POINT_Y <= WALL->y2 && ball->dx < 0) {
                ball->f_x = WALL->x2 + ball->radius;
                collideWall(ball, WALL, COLLIDE_WALL_LEFT, callback,
                            args);
                ret = 1;
            }
            break;
        case COLLIDE_BALL:
            // TODO
            break;
        default:
            break;
    }

    return ret;
}

signed char checkBallCollisionsWithWalls(ball_t *ball, void (*callback)(void *),
        void *args)
{
    unsigned char ret = 0;
    unsigned int i;
    for (i = 0; i < walls.wall_count; i++)
        if (handleCollision(ball, walls.walls[i], COLLIDE_WALL,
                            callback, args)) {
            ret = -1;
        }
    return ret;
}

unsigned char checkBallCollisionsWithBalls(ball_t *ball)
{
    // TODO
    return 0;
}

signed char checkBallCollisions(ball_t *ball, void (*callback)(void *),
                                void *args)
{
    unsigned char ret = 0;
    if (checkBallCollisionsWithWalls(ball, callback, args)) {
        ret = -1;
    }
    if (checkBallCollisionsWithBalls(ball)) {
        ret = -1;
    }
    return ret;
}
