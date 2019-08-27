#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "TUM_Draw.h"
#include "TUM_Ball.h"
#include "TUM_Sound.h"

typedef struct walls{
    wall_t **walls;

    unsigned int wall_count;
}walls_t;

walls_t walls = {0};

wall_t *createWall(unsigned short x1, unsigned short y1, unsigned short w,
        unsigned short h, float dampening, unsigned int colour, 
        void (*callback)()) {

    wall_t *ret = calloc(1, sizeof(wall_t));

    if(!ret) {
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

    walls.wall_count++;
    walls.walls = realloc(walls.walls, sizeof(wall_t *) * walls.wall_count);

    if(!walls.walls){
        fprintf(stderr, "Increasing walls list to %d walls failed\n",
                walls.wall_count);
        exit(EXIT_FAILURE);
    }

    walls.walls[walls.wall_count - 1] = ret;

    return ret;
}

ball_t *createBall(unsigned short initial_x, unsigned short initial_y,
        unsigned int colour, unsigned short radius, float max_speed, 
        void (*callback)()){

    ball_t *ret = calloc(1, sizeof(ball_t));

    if (!ret){
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

    return ret;
}

void setBallSpeed(ball_t *ball, float dx, float dy, float max_speed, 
        unsigned char flags) {
    if(flags & 1) //Set X
        if(dx <= ball->max_speed)
            ball->dx = dx;
    if((flags >> 1) & 1) //Set y
        if(dy <= ball->max_speed)
            ball->dy = dy;
    if((flags >> 2) & 1) //Set max speed
        ball->max_speed = max_speed;
}

#define SET_BALL_COORD(AXIS, VAL) \
            ball->AXIS = VAL; \
            ball->f_##AXIS = VAL;

void setBallLocation(ball_t *ball, unsigned short x, unsigned short y) {
    if (x < ball->radius){
        SET_BALL_COORD(x, ball->radius);
    }else if(x > SCREEN_WIDTH - ball->radius){
        SET_BALL_COORD(x, SCREEN_WIDTH - ball->radius);
    }else{
        SET_BALL_COORD(x, x)
    }

    if (y < ball->radius){
        SET_BALL_COORD(x, ball->radius);
    }else if(x > SCREEN_HEIGHT - ball->radius){
        SET_BALL_COORD(x, SCREEN_HEIGHT - ball->radius);
    }else{
        SET_BALL_COORD(x, x)
    }
}

void updateBallPosition(ball_t *ball, unsigned int milli_seconds) {
    float update_interval = milli_seconds / 1000.0;
    ball->f_x += ball->dx * update_interval;
    ball->f_y += ball->dy * update_interval;
    ball->x = round(ball->f_x);
    ball->y = round(ball->f_y);
}

#define HORIZONTAL  0b1
#define VERTICAL    0b10

void changeBallDirection(ball_t *ball, unsigned char direction, 
        float dampening) {
    if(direction & 1){
        ball->dx *= -1;
        setBallSpeed(ball, ball->dx * (1 + dampening), 0, 0, SET_BALL_SPEED_X);
    }
    
    if((direction >> 1) & 1){
        ball->dy *= -1;
        setBallSpeed(ball, 0, ball->dy * (1 + dampening), 0, SET_BALL_SPEED_Y);
    }
}


#define COLLIDE_WALL    1
#define COLLIDE_BALL    2

#define COLLIDE_WALL_TOP    1
#define COLLIDE_WALL_BOTTOM 2
#define COLLIDE_WALL_LEFT   3
#define COLLIDE_WALL_RIGHT  4

#define BALL_LEFT_POINT_X   ball->f_x - ball->radius
#define BALL_LEFT_POINT_Y   ball->f_y
#define BALL_RIGHT_POINT_X  ball->f_x + ball->radius
#define BALL_RIGHT_POINT_Y  ball->f_y 
#define BALL_TOP_POINT_X    ball->f_x
#define BALL_TOP_POINT_Y    ball->f_y - ball->radius
#define BALL_BOTTOM_POINT_X ball->f_x
#define BALL_BOTTOM_POINT_Y ball->f_y + ball->radius

#define WALL    ((wall_t *)object)
#define BALL    ((ball_t *)object)

unsigned char collideWall(ball_t *ball, wall_t *wall, 
        unsigned char collision_type, void (*callback)()) {

    if (callback)
        callback();
    if(wall->callback)
        wall->callback();
    if(ball->callback)
        ball->callback();
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

unsigned char handleCollision(ball_t *ball, void *object, unsigned char flag,
        void (*callback)()) {
    switch(flag){
    case COLLIDE_WALL:
            //Coming from:
            //Above wall
            if(BALL_BOTTOM_POINT_Y >= WALL->y1 
                    && BALL_TOP_POINT_Y < WALL->y1 
                    && BALL_BOTTOM_POINT_X >= WALL->x1 
                    && BALL_BOTTOM_POINT_X <= WALL->x2){
                collideWall(ball, WALL, COLLIDE_WALL_TOP, callback);
                //Place ball next to wall to prevent ball getting stuck in wall
                ball->f_y = WALL->y1 - ball->radius;
            }
            //Below wall
            if(BALL_TOP_POINT_Y <= WALL->y2
                    && BALL_BOTTOM_POINT_Y > WALL->y2
                    && BALL_TOP_POINT_X >= WALL->x1
                    && BALL_TOP_POINT_X <= WALL->x2){
                collideWall(ball, WALL, COLLIDE_WALL_BOTTOM, callback);
                ball->f_y = WALL->y2 + ball->radius;
            }
            //Left of wall
            if(BALL_RIGHT_POINT_X >= WALL->x1
                    && BALL_LEFT_POINT_X < WALL->x1
                    && BALL_RIGHT_POINT_Y >= WALL->y1
                    && BALL_RIGHT_POINT_Y <= WALL->y2){
                collideWall(ball, WALL, COLLIDE_WALL_RIGHT, callback);
                ball->f_x = WALL->x1 - ball->radius;
            }
            //Right of wall
            if(BALL_LEFT_POINT_X <= WALL->x2
                    && BALL_RIGHT_POINT_X > WALL->x2
                    && BALL_LEFT_POINT_Y >= WALL->y1
                    && BALL_RIGHT_POINT_Y <= WALL->y2){
                collideWall(ball, WALL, COLLIDE_WALL_LEFT, callback);
                ball->f_x = WALL->x2 + ball->radius;
            }
        break;
    case COLLIDE_BALL:
        //TODO
        break;
    default:
        break;
    }

    return 0;
}

void checkBallCollisionsWithWalls(ball_t *ball, void (*callback)()) {
    for(unsigned int i = 0; i < walls.wall_count; i++)
        handleCollision(ball, walls.walls[i], COLLIDE_WALL, callback);
}

void checkBallCollisionsWithBalls(ball_t *ball) {
    //TODO
}

void checkBallCollisions(ball_t *ball, void (*callback)()) {
    checkBallCollisionsWithWalls(ball, callback);
    checkBallCollisionsWithBalls(ball);
}
