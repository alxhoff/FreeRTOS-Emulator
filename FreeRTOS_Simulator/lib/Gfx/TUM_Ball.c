#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "TUM_Ball.h"

typedef struct walls{
    wall_t **walls;

    unsigned int wall_count;
}walls_t;

walls_t walls = {0};

wall_t *create_wall(unsigned short x1, unsigned short y1, unsigned short x2,
        unsigned short y2, float dampening, unsigned int colour) {

    wall_t *ret = calloc(1, sizeof(wall_t));

    if(!ret) {
        fprintf(stderr, "Creating wall failed\n");
        exit(EXIT_FAILURE);
    }

    if(!(x1 <= x2) || !(y1 <= y2))
        goto error_dimensions;

    ret->x1 = x1;
    ret->x2 = x2;
    ret->y1 = y1;
    ret->y2 = y2;
    ret->colour = colour;
    ret->dampening = dampening;
    ret->angle = atan((y2-y1)/(x2-x1)) * 180 / M_PI;

    walls.wall_count++;
    walls.walls = realloc(walls.walls, sizeof(wall_t *) * walls.wall_count);
    if(!walls.walls){
        fprintf(stderr, "Increasing walls list to %d walls failed\n",
                walls.wall_count);
        exit(EXIT_FAILURE);
    }

error_dimensions:
    fprintf(stderr, "Wall dimensions not appropriate\n");
    exit(EXIT_FAILURE);
}

ball_t *createBall(unsigned short initial_x, unsigned short initial_y,
        unsigned int colour, unsigned short radius){
    ball_t *ret = calloc(1, sizeof(ball_t));

    if (!ret){
        fprintf(stderr, "Creating ball failed\n");
        exit(EXIT_FAILURE);
    }

    ret->x = initial_x;
    ret->y = initial_y;
    ret->f_x = initial_x;
    ret->f_y = initial_y;
    ret->colour = colour;
    ret->radius = radius;

    return ret;
}

void setBallSpeed(ball_t *ball, float dx, float dy) {
    if(dx)
        ball->dx = dx;

    if(dy)
        ball->dy = dy;
}

void updateBallPosition(ball_t *ball, unsigned int mili_seconds) {
    float update_interval = mili_seconds / 1000.0;
    ball->f_x += ball->dx * update_interval;
    ball->f_y += ball->dy * update_interval;
    ball->x = round(ball->f_x);
    ball->y = round(ball->f_y);
}

#define HORIZONTAL  0b1
#define VERTICAL    0b01

void changeBallDirection(ball_t *ball, unsigned char direction, 
        float dampening) {
    if(direction & 1){
        ball->dx *= -1;
        if(abs(ball->dx) > dampening)
            if(ball->dx > 0)
                ball->dx -= dampening;
            else
                ball->dx += dampening;
        else ball->dx = 0;
    }
    
    if((direction >> 1) & 1){
        ball->dy *= -1;
        if(abs(ball->dy) > dampening)
            if(ball->dy > 0)
                ball->dy -= dampening;
            else
                ball->dy += dampening;
        else ball->dy = 0;
    }
}


#define COLLIDE_WALL    1
#define COLLIDE_BALL    2

#define COLLIDE_WALL_TOP    1
#define COLLIDE_WALL_BOTTOM 2
#define COLLIDE_WALL_LEFT   3
#define COLLIDE_WALL_RIGHT  4

#define BALL_LEFT_POINT_X   ball->x - ball->radius
#define BALL_LEFT_POINT_Y   ball->y
#define BALL_RIGHT_POINT_X  ball->x + ball->radius
#define BALL_RIGHT_POINT_Y  ball->y + ball->radius
#define BALL_TOP_POINT_X    ball->x
#define BALL_TOP_POINT_Y    ball->y + ball->radius
#define BALL_BOTTOM_POINT_X ball->x
#define BALL_BOTTOM_POINT_Y ball->y - ball->radius

#define WALL    ((wall_t *)object)
#define BALL    ((ball_t *)object)

unsigned char collideWall(ball_t *ball, wall_t *wall, 
        unsigned char collision_type) {

    switch(collision_type){
    case COLLIDE_WALL_TOP:
        changeBallDirection(ball, VERTICAL, wall->dampening);
        break;
    case COLLIDE_WALL_BOTTOM:
        changeBallDirection(ball, VERTICAL, wall->dampening);
        break;
    case COLLIDE_WALL_RIGHT:
        changeBallDirection(ball, HORIZONTAL, wall->dampening);
        break;
    case COLLIDE_WALL_LEFT:
        changeBallDirection(ball, HORIZONTAL, wall->dampening);
        break;
    default:
        break;
    }

    return 0;
}

unsigned char handleCollition(ball_t *ball, void *object, unsigned char flag) {
    switch(flag){
    case COLLIDE_WALL:
            //Above wall
            if(BALL_BOTTOM_POINT_Y >= WALL->y1 
                    && BALL_TOP_POINT_Y < WALL->y1 
                    && BALL_BOTTOM_POINT_X >= WALL->x1 
                    && BALL_BOTTOM_POINT_X <= WALL->x2)
                collideWall(ball, WALL, COLLIDE_WALL_TOP);
            //Below wall
            else if(BALL_TOP_POINT_Y <= WALL->y2
                    && BALL_BOTTOM_POINT_Y > WALL->y2
                    && BALL_TOP_POINT_X >= WALL->x1
                    && BALL_TOP_POINT_X <= WALL->x2)
                collideWall(ball, WALL, COLLIDE_WALL_BOTTOM);
            //Left of wall
            else if(BALL_RIGHT_POINT_X >= WALL->x1
                    && BALL_LEFT_POINT_X < WALL->x1
                    && BALL_RIGHT_POINT_Y >= WALL->y1
                    && BALL_RIGHT_POINT_Y <= WALL->y2)
                collideWall(ball, WALL, COLLIDE_WALL_RIGHT);
            //Right of wall
            else if(BALL_LEFT_POINT_X <= WALL->x2
                    && BALL_RIGHT_POINT_X > WALL->x2
                    && BALL_LEFT_POINT_Y >= WALL->y1
                    && BALL_RIGHT_POINT_Y <= WALL->y2)
                collideWall(ball, WALL, COLLIDE_WALL_LEFT);
        break;
    case COLLIDE_BALL:
        //TODO
        break;
    default:
        break;
    }

    return 0;
}
