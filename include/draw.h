#ifndef __DRAW_H__
#define __DRAW_H__

extern image_handle_t logo_image;

void vCreateWalls(wall_t **left_wall, wall_t **right_wall, wall_t **top_wall, wall_t **bottom_wall);
void vDrawWalls(wall_t *left_wall, wall_t *right_wall, wall_t *top_wall, wall_t *bottom_wall);
void vDrawBall(ball_t *ball);
void vDrawClearScreen(void);
void vDrawCaveBoundingBox(void);
void vDrawCave(unsigned char ball_color_inverted);
void vDrawHelpText(void);
void vDrawFPS(void);
void vDrawLogo(void);
void vDrawStaticItems(void);
void vDrawButtonText(void);
void vDrawSpriteAnnimations(TickType_t xLastFrameTime);
void vDrawInitAnnimations(void);

#endif //__DRAW_H__