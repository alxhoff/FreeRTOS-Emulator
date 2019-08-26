#ifndef __TUM_BALL_H__
#define __TUM_BALL_H__

typedef struct ball{
    unsigned short x;
    unsigned short y;

    float f_x;
    float f_y;

    float dx;
    float dy;

    unsigned int colour;

    unsigned short radius;

    void (*callback)(void);
}ball_t;

typedef struct wall{
    unsigned short x1;
    unsigned short y1;

    unsigned short w;
    unsigned short h;

    unsigned short x2;
    unsigned short y2;

    float dampening;
    
    unsigned int colour;
    
    void (*callback)(void);
}wall_t;

ball_t *createBall(unsigned short initial_x, unsigned short initial_y,
        unsigned int colour, unsigned short radius, void (*callback)());
wall_t *createWall(unsigned short x1, unsigned short y1, unsigned short w,
        unsigned short h, float dampening, unsigned int colour, 
        void (*callback)());
void setBallSpeed(ball_t *ball, float dx, float dy);
void checkBallCollisions(ball_t *ball, void (*callback)());
void updateBallPosition(ball_t *ball, unsigned int mili_seconds);

#endif
