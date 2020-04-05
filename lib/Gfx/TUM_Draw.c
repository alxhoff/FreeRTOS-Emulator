/**
 * @file TUM_Draw.c
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief A SDL2 based library to implement work queue based drawing of graphical
 * elements. Allows for drawing using SDL2 from multiple threads.
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2019
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <pthread.h>
#include <assert.h>

#include "TUM_Draw.h"
#include "TUM_Utils.h"

typedef enum {
    DRAW_NONE = 0,
    DRAW_CLEAR,
    DRAW_ARC,
    DRAW_ELLIPSE,
    DRAW_TEXT,
    DRAW_RECT,
    DRAW_FILLED_RECT,
    DRAW_CIRCLE,
    DRAW_LINE,
    DRAW_POLY,
    DRAW_TRIANGLE,
    DRAW_IMAGE,
    DRAW_SCALED_IMAGE,
    DRAW_ARROW,
} draw_job_type_t;

typedef struct clear_data {
    unsigned int colour;
} clear_data_t;

typedef struct arc_data {
    signed short x;
    signed short y;
    signed short radius;
    signed short start;
    signed short end;
    unsigned int colour;
} arc_data_t;

typedef struct ellipse_data {
    signed short x;
    signed short y;
    signed short rx;
    signed short ry;
    unsigned int colour;
} ellipse_data_t;

typedef struct rect_data {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned int colour;
} rect_data_t;

typedef struct circle_data {
    unsigned short x;
    unsigned short y;
    unsigned short radius;
    unsigned int colour;
} circle_data_t;

typedef struct line_data {
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned char thickness;
    unsigned int colour;
} line_data_t;

typedef struct poly_data {
    coord_t *points;
    unsigned int n;
    unsigned int colour;
} poly_data_t;

typedef struct triangle_data {
    coord_t *points;
    unsigned int colour;
} triangle_data_t;

typedef struct image_data {
    char *filename;
    SDL_Texture *tex;
    unsigned short x;
    unsigned short y;
} image_data_t;

typedef struct scaled_image_data {
    image_data_t image;
    float scale;
} scaled_image_data_t;

typedef struct text_data {
    char *str;
    unsigned short x;
    unsigned short y;
    unsigned int colour;
} text_data_t;

typedef struct arrow_data {
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short head_length;
    unsigned char thickness;
    unsigned short colour;
} arrow_data_t;

union data_u {
    clear_data_t clear;
    arc_data_t arc;
    ellipse_data_t ellipse;
    rect_data_t rect;
    circle_data_t circle;
    line_data_t line;
    poly_data_t poly;
    triangle_data_t triangle;
    image_data_t image;
    scaled_image_data_t scaled_image;
    text_data_t text;
    arrow_data_t arrow;
};

typedef struct draw_job {
    draw_job_type_t type;
    union data_u *data;

    struct draw_job *next;
} draw_job_t;

draw_job_t job_list_head = { 0 };

const int screen_height = SCREEN_HEIGHT;
const int screen_width = SCREEN_WIDTH;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext context = NULL;
TTF_Font *font = NULL;

char *error_message = NULL;

#define SHIFT_BYTE 8
#define SHIFT_THREE_BYTE 24
#define FIRST_BYTE 0x000000ff 
#define SECOND_BYTE 0x0000ff00 
#define THIRD_BYTE 0x00ff0000 
#define FOURTH_BYTE 0xff000000 

uint32_t SwapBytes(unsigned int x)
{
    return ((x & FIRST_BYTE) << SHIFT_THREE_BYTE) + ((x & SECOND_BYTE) << SHIFT_BYTE) +
           ((x & THIRD_BYTE) >> SHIFT_BYTE) + ((x & FOURTH_BYTE) >> SHIFT_THREE_BYTE);
}

void setErrorMessage(char *msg)
{
    if (error_message) {
        free(error_message);
    }

    error_message = malloc(sizeof(char) * (strlen(msg) + 1));
    if (!error_message) {
        return;
    }
    strcpy(error_message, msg);
}

#define PRINT_TTF_ERROR(msg, ...)                                              \
    PRINT_ERROR("[TTF Error] %s\n" #msg, (char *)TTF_GetError(),           \
                ##__VA_ARGS__)
#define PRINT_SDL_ERROR(msg, ...)                                              \
    PRINT_ERROR("[SDL Error] %s\n" #msg, (char *)SDL_GetError(),           \
                ##__VA_ARGS__)

static draw_job_t *pushDrawJob(void)
{
    draw_job_t *iterator;
    draw_job_t *job = calloc(1, sizeof(draw_job_t));

    assert(job);

    for (iterator = &job_list_head; iterator->next;
         iterator = iterator->next)
        ;

    iterator->next = job;

    return job;
}

static draw_job_t *popDrawJob(void)
{
    draw_job_t *ret = job_list_head.next;

    if (ret) {
        if (ret->next) {
            job_list_head.next = ret->next;
        }
        else {
            job_list_head.next = NULL;
        }
    }

    return ret;
}

static int vClearDisplay(unsigned int colour)
{
    SDL_SetRenderDrawColor(renderer, (colour >> 16) & 0xFF,
                           (colour >> 8) & 0xFF, colour & 0xFF, 255);
    SDL_RenderClear(renderer);

    return 0;
}

static int vDrawRectangle(signed short x, signed short y, signed short w,
                          signed short h, unsigned int colour)
{
    rectangleColor(renderer, x + w, y, x, y + h,
                   SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawFilledRectangle(signed short x, signed short y, signed short w,
                                signed short h, unsigned int colour)
{
    boxColor(renderer, x + w, y, x, y + h, SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawArc(signed short x, signed short y, signed short radius,
                    signed short start, signed short end, unsigned int colour)
{
    arcColor(renderer, x, y, radius, start, end,
             SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawEllipse(signed short x, signed short y, signed short rx,
                        signed short ry, unsigned int colour)
{
    ellipseColor(renderer, x, y, rx, ry, SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawCircle(signed short x, signed short y, signed short radius,
                       unsigned int colour)
{
    filledCircleColor(renderer, x, y, radius,
                      SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawLine(signed short x1, signed short y1, signed short x2,
                     signed short y2, unsigned char thickness,
                     unsigned int colour)
{
    thickLineColor(renderer, x1, y1, x2, y2, thickness,
                   SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static int vDrawPoly(coord_t *points, unsigned int n, signed short colour)
{
    signed short *x_coords = calloc(1, sizeof(signed short) * n);
    signed short *y_coords = calloc(1, sizeof(signed short) * n);
    unsigned int i;

    for (i = 0; i < n; i++) {
        x_coords[i] = points[i].x;
        y_coords[i] = points[i].y;
    }

    polygonColor(renderer, x_coords, y_coords, n,
                 SwapBytes((colour << 8) | 0xFF));

    free(x_coords);
    free(y_coords);

    return 0;
}

static int vDrawTriangle(coord_t *points, unsigned int colour)
{
    filledTrigonColor(renderer, points[0].x, points[0].y, points[1].x,
                      points[1].y, points[2].x, points[2].y,
                      SwapBytes((colour << 8) | 0xFF));

    return 0;
}

static SDL_Texture *loadImage(char *filename, SDL_Renderer *ren)
{
    SDL_Texture *tex = NULL;

    tex = IMG_LoadTexture(ren, filename);

    if (!tex) {
        if (ren) {
            SDL_DestroyRenderer(ren);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        PRINT_SDL_ERROR("Failed to load image");
        SDL_Quit();
        exit(0);
    }

    return tex;
}

static void vRenderScaledImage(SDL_Texture *tex, SDL_Renderer *ren,
                               unsigned short x, unsigned short y,
                               unsigned short w, unsigned short h)
{
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

void vGetImageSize(char *filename, int *w, int *h)
{
    SDL_Texture *tex = loadImage(filename, renderer);
    SDL_QueryTexture(tex, NULL, NULL, w, h);
    SDL_DestroyTexture(tex);
}

static int vDrawScaledImage(SDL_Texture *tex, SDL_Renderer *ren,
                            unsigned short x, unsigned short y, float scale)
{
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    vRenderScaledImage(tex, ren, x, y, w * scale, h * scale);
    SDL_DestroyTexture(tex);

    return 0;
}

static int vDrawImage(SDL_Texture *tex, SDL_Renderer *ren, unsigned short x,
                      unsigned short y)
{
    vDrawScaledImage(tex, ren, x, y, 1);

    return 0;
}

static void vDrawLoadAndDrawImage(char *filename, SDL_Renderer *ren,
                                  unsigned short x, unsigned short y)
{
    SDL_Texture *tex = loadImage(filename, ren);

    vDrawImage(tex, ren, x, y);

    SDL_DestroyTexture(tex);
}

static void vDrawRectImage(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst,
                           SDL_Rect *clip)
{
    SDL_RenderCopy(ren, tex, clip, &dst);
}

static void vDrawClippedImage(SDL_Texture *tex, SDL_Renderer *ren,
                              unsigned short x, unsigned short y,
                              SDL_Rect *clip)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    if (!clip) {
        dst.w = clip->w;
        dst.h = clip->h;
    }
    else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }

    vDrawRectImage(tex, ren, dst, clip);
}

#define RED_PORTION(COLOUR) (COLOUR & 0xFF0000) >> 16
#define GREEN_PORTION(COLOUR) (COLOUR & 0x00FF00) >> 8
#define BLUE_PORTION(COLOUR) (COLOUR & 0x0000FF)
#define ZERO_ALPHA 0

static int vDrawText(char *string, unsigned short x, unsigned short y,
                     unsigned int colour)
{
    SDL_Color color = { RED_PORTION(colour), GREEN_PORTION(colour),
                        BLUE_PORTION(colour), ZERO_ALPHA
                      };
    SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { 0 };
    SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
    dst.x = x;
    dst.y = y;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    return 0;
}

static void vGetTextSize(char *string, int *width, int *height)
{
    SDL_Color color = { 0 };
    SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
    assert(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    assert(texture);
    SDL_QueryTexture(texture, NULL, NULL, width, height);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static int vDrawArrow(unsigned short x1, unsigned short y1, unsigned short x2,
                      unsigned short y2, unsigned short head_length,
                      unsigned char thickness, unsigned int colour)
{
    // Line vector
    unsigned short dx = x2 - x1;
    unsigned short dy = y2 - y1;

    // Normalize
    float length = sqrt(dx * dx + dy * dy);
    float unit_dx = dx / length;
    float unit_dy = dy / length;

    unsigned short head_x1 =
        round(x2 - unit_dx * head_length - unit_dy * head_length);
    unsigned short head_y1 =
        round(y2 - unit_dy * head_length + unit_dx * head_length);

    unsigned short head_x2 =
        round(x2 - unit_dx * head_length + unit_dy * head_length);
    unsigned short head_y2 =
        round(y2 - unit_dy * head_length - unit_dx * head_length);

    if (thickLineColor(renderer, x1, y1, x2, y2, thickness,
                       SwapBytes((colour << 8) | 0xFF))) {
        return -1;
    }
    if (thickLineColor(renderer, head_x1, head_y1, x2, y2, thickness,
                       SwapBytes((colour << 8) | 0xFF))) {
        return -1;
    }
    if (thickLineColor(renderer, head_x2, head_y2, x2, y2, thickness,
                       SwapBytes((colour << 8) | 0xFF))) {
        return -1;
    }

    return 0;
}

static int vHandleDrawJob(draw_job_t *job)
{
    int ret = 0;
    if (!job) {
        return -1;
    }

    assert(job->data);

    switch (job->type) {
        case DRAW_CLEAR:
            ret = vClearDisplay(job->data->clear.colour);
            break;
        case DRAW_ARC:
            ret = vDrawArc(job->data->arc.x, job->data->arc.y,
                           job->data->arc.radius, job->data->arc.start,
                           job->data->arc.end, job->data->arc.colour);
            break;
        case DRAW_ELLIPSE:
            ret = vDrawEllipse(job->data->ellipse.x, job->data->ellipse.y,
                               job->data->ellipse.rx, job->data->ellipse.ry,
                               job->data->ellipse.colour);
            break;
        case DRAW_TEXT:
            ret = vDrawText(job->data->text.str, job->data->text.x,
                            job->data->text.y, job->data->text.colour);
            free(job->data->text.str);
            break;
        case DRAW_RECT:
            ret = vDrawRectangle(job->data->rect.x, job->data->rect.y,
                                 job->data->rect.w, job->data->rect.h,
                                 job->data->rect.colour);
            break;
        case DRAW_FILLED_RECT:
            ret = vDrawFilledRectangle(job->data->rect.x, job->data->rect.y,
                                       job->data->rect.w, job->data->rect.h,
                                       job->data->rect.colour);
            break;
        case DRAW_CIRCLE:
            ret = vDrawCircle(job->data->circle.x, job->data->circle.y,
                              job->data->circle.radius,
                              job->data->circle.colour);
            break;
        case DRAW_LINE:
            ret = vDrawLine(job->data->line.x1, job->data->line.y1,
                            job->data->line.x2, job->data->line.y2,
                            job->data->line.thickness,
                            job->data->line.colour);
            break;
        case DRAW_POLY:
            ret = vDrawPoly(job->data->poly.points, job->data->poly.n,
                            job->data->poly.colour);
            break;
        case DRAW_TRIANGLE:
            ret = vDrawTriangle(job->data->triangle.points,
                                job->data->triangle.colour);
            break;
        case DRAW_IMAGE:
            job->data->image.tex =
                loadImage(job->data->image.filename, renderer);
            ret = vDrawImage(job->data->image.tex, renderer,
                             job->data->image.x, job->data->image.y);
            break;
        case DRAW_SCALED_IMAGE:
            job->data->scaled_image.image.tex = loadImage(
                                                    job->data->scaled_image.image.filename, renderer);
            ret = vDrawScaledImage(job->data->scaled_image.image.tex,
                                   renderer,
                                   job->data->scaled_image.image.x,
                                   job->data->scaled_image.image.y,
                                   job->data->scaled_image.scale);
            break;
        case DRAW_ARROW:
            ret = vDrawArrow(job->data->arrow.x1, job->data->arrow.y1,
                             job->data->arrow.x2, job->data->arrow.y2,
                             job->data->arrow.head_length,
                             job->data->arrow.thickness,
                             job->data->arrow.colour);
        default:
            break;
    }
    free(job->data);

    return ret;
}

#define INIT_JOB(JOB, TYPE)                                                    \
    draw_job_t *JOB = pushDrawJob();                                       \
    union data_u *data = calloc(1, sizeof(union data_u));                  \
    if (!data)                                                             \
        logCriticalError("job->data alloc");                           \
    JOB->data = data;                                                      \
    JOB->type = TYPE;

static void logCriticalError(char *msg)
{
    printf("[ERROR] %s\n", msg);
    exit(-1);
}

static float timespecDiffMilli(struct timespec *start, struct timespec *stop)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0)
        return (stop->tv_sec - start->tv_sec - 1) * 1000 +
               (stop->tv_nsec - start->tv_nsec + 1000000000) /
               1000000.0;
    else
        return (stop->tv_sec - start->tv_sec) * 1000 +
               (stop->tv_nsec - start->tv_nsec) / 1000000.0;
}

#define FRAMELIMIT 60.0
#define FRAMELIMIT_PERIOD 1000 / FRAMELIMIT

void vDrawUpdateScreen(void)
{
    static struct timespec last_time = { 0 }, cur_time = { 0 };

    if (clock_gettime(CLOCK_MONOTONIC, &cur_time)) {
        PRINT_ERROR("Failed to get monotonic clock");
        return;
    }

    if (timespecDiffMilli(&last_time, &cur_time) < (float)FRAMELIMIT_PERIOD) {
        return;
    }
    else {
        memcpy(&last_time, &cur_time, sizeof(struct timespec));
    }

    if (!job_list_head.next) {
        return;
    }

    draw_job_t *tmp_job;

    while ((tmp_job = popDrawJob()) != NULL) {
        assert(tmp_job->data);
        if (vHandleDrawJob(tmp_job) == -1) {
            goto draw_error;
        }
        free(tmp_job);
    }

    SDL_RenderPresent(renderer);

    return;

draw_error:
    free(tmp_job);
}

char *tumGetErrorMessage(void)
{
    return error_message;
}

int vInitDrawing(char *path) // Should be called from the Thread running main()
{
    /* Relevant for Docker-based toolchain */
#ifdef DOCKER
#ifndef HOST_OS
#warning "HOST_OS undefined! Assuming 'linux'..."
#elif HOST_OS != linux
    setenv("LIBGL_ALWAYS_INDIRECT", "1",
           1); // speed up drawings a little bit
    setenv("SDL_VIDEO_X11_VISUALID", "",
           1); // required on windows and macos
#elif HOST_OS == linux
    // nothing
#else
#error "Unexpected value of HOST_OS!"
#endif /* HOST_OS */
#endif /* DOCKER */

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        PRINT_SDL_ERROR("SDL_Init failed");
        goto err_sdl;
    }
    if (TTF_Init()) {
        PRINT_TTF_ERROR("TTF_Init failed");
        goto err_ttf;
    }

    char *buffer = prepend_path(path, FONT_LOCATION);
    if (!buffer) {
        PRINT_ERROR("Prepending font path failed");
        goto err_font_loc;
    }

    font = TTF_OpenFont(buffer, DEFAULT_FONT_SIZE);
    free(buffer);

    if (!font) {
        PRINT_TTF_ERROR("Opening font @ '%s' failed", FONT_LOCATION);
        goto err_open_font;
    }

    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, screen_width,
                              screen_height, SDL_WINDOW_OPENGL);

    if (!window) {
        PRINT_SDL_ERROR("Failed to create %d x %d window '%s'",
                        screen_width, screen_height, WINDOW_TITLE);
        goto err_window;
    }

    context = SDL_GL_CreateContext(window);

    if (!context) {
        PRINT_SDL_ERROR("Failed to create context");
        goto err_create_context;
    }

    if (SDL_GL_MakeCurrent(window, context) < 0) {
        PRINT_SDL_ERROR("Claiming current context failed");
        goto err_make_current;
    }

    if (SDL_GL_MakeCurrent(window, NULL) < 0) {
        PRINT_SDL_ERROR("Releasing current context failed");
        goto err_make_current;
    }

    atexit(SDL_Quit);

    return 0;

err_make_current:
    SDL_GL_DeleteContext(context);
err_create_context:
    SDL_DestroyWindow(window);
err_window:
    TTF_CloseFont(font);
err_open_font:
err_font_loc:
    TTF_Quit();
err_ttf:
    SDL_Quit();
err_sdl:
    return -1;
}

int vBindDrawing(void) // Should be called from the Drawing Thread
{
    if (SDL_GL_MakeCurrent(window, context) < 0) {
        PRINT_SDL_ERROR("Releasing current context failed");
        goto err_make_current;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_TARGETTEXTURE |
                                  SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        PRINT_SDL_ERROR("Failed to create renderer");
        goto err_renderer;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderClear(renderer);

    return 0;

err_renderer:
    SDL_DestroyWindow(window);
err_make_current:
    SDL_GL_DeleteContext(context);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return -1;
}

void vExitDrawing(void)
{
    if (window) {
        SDL_DestroyWindow(window);
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    TTF_Quit();
    SDL_Quit();

    exit(EXIT_SUCCESS);
}

signed char tumDrawText(char *str, signed short x, signed short y,
                        unsigned int colour)
{
    if (!strcmp(str, "")) {
        return -1;
    }

    INIT_JOB(job, DRAW_TEXT);

    job->data->text.str = malloc(sizeof(char) * (strlen(str) + 1));

    if (!job->data->text.str) {
        printf("Error allocating buffer in tumDrawText\n");
        exit(EXIT_FAILURE);
    }

    strcpy(job->data->text.str, str);
    job->data->text.x = x;
    job->data->text.y = y;
    job->data->text.colour = colour;

    return 0;
}

void tumGetTextSize(char *str, int *width, int *height)
{
    assert(str);
    vGetTextSize(str, width, height);
}

signed char tumDrawEllipse(signed short x, signed short y, signed short rx,
                           signed short ry, unsigned int colour)
{
    INIT_JOB(job, DRAW_ELLIPSE);

    job->data->ellipse.x = x;
    job->data->ellipse.y = y;
    job->data->ellipse.rx = rx;
    job->data->ellipse.ry = ry;
    job->data->ellipse.colour = colour;

    return 0;
}

signed char tumDrawArc(signed short x, signed short y, signed short radius,
                       signed short start, signed short end,
                       unsigned int colour)
{
    INIT_JOB(job, DRAW_ARC);

    job->data->arc.x = x;
    job->data->arc.y = y;
    job->data->arc.radius = radius;
    job->data->arc.start = start;
    job->data->arc.end = end;
    job->data->arc.colour = colour;

    return 0;
}

signed char tumDrawFilledBox(signed short x, signed short y, signed short w,
                             signed short h, unsigned int colour)
{
    INIT_JOB(job, DRAW_FILLED_RECT);

    job->data->rect.x = x;
    job->data->rect.y = y;
    job->data->rect.w = w;
    job->data->rect.h = h;
    job->data->rect.colour = colour;

    return 0;
}

signed char tumDrawBox(signed short x, signed short y, signed short w,
                       signed short h, unsigned int colour)
{
    INIT_JOB(job, DRAW_RECT);

    job->data->rect.x = x;
    job->data->rect.y = y;
    job->data->rect.w = w;
    job->data->rect.h = h;
    job->data->rect.colour = colour;

    return 0;
}

signed char tumDrawClear(unsigned int colour)
{
    /** INIT_JOB(job, DRAW_CLEAR); */
    draw_job_t *job = pushDrawJob();
    union data_u *data = calloc(1, sizeof(union data_u));
    if (!data) {
        logCriticalError("job->data alloc");
    }
    job->data = data;
    job->type = DRAW_CLEAR;

    job->data->clear.colour = colour;

    return 0;
}

signed char tumDrawCircle(signed short x, signed short y, signed short radius,
                          unsigned int colour)
{
    INIT_JOB(job, DRAW_CIRCLE);

    job->data->circle.x = x;
    job->data->circle.y = y;
    job->data->circle.radius = radius;
    job->data->circle.colour = colour;

    return 0;
}

signed char tumDrawLine(signed short x1, signed short y1, signed short x2,
                        signed short y2, unsigned char thickness,
                        unsigned int colour)
{
    INIT_JOB(job, DRAW_LINE);

    job->data->line.x1 = x1;
    job->data->line.y1 = y1;
    job->data->line.x2 = x2;
    job->data->line.y2 = y2;
    job->data->line.thickness = thickness;
    job->data->line.colour = colour;

    return 0;
}

signed char tumDrawPoly(coord_t *points, int n, unsigned int colour)
{
    INIT_JOB(job, DRAW_POLY);

    job->data->poly.points = points;
    job->data->poly.n = n;
    job->data->poly.colour = colour;

    return 0;
}

signed char tumDrawTriangle(coord_t *points, unsigned int colour)
{
    INIT_JOB(job, DRAW_TRIANGLE);

    job->data->triangle.points = points;
    job->data->triangle.colour = colour;

    return 0;
}

signed char tumDrawImage(char *filename, signed short x, signed short y)
{
    INIT_JOB(job, DRAW_IMAGE);

    char abs_path[PATH_MAX + 1];

    if (!realpath(filename, (char *)abs_path)) {
        return -1;
    }

    job->data->image.filename =
        malloc(sizeof(char) * (strlen(abs_path) + 1));
    strcpy(job->data->image.filename, abs_path);
    job->data->image.x = x;
    job->data->image.y = y;

    return 0;
}

void tumGetImageSize(char *filename, int *w, int *h)
{
    char full_filename[PATH_MAX + 1];
    realpath(filename, full_filename);
    vGetImageSize(full_filename, w, h);
}

signed char tumDrawScaledImage(char *filename, signed short x, signed short y,
                               float scale)
{
    // TODO

    INIT_JOB(job, DRAW_SCALED_IMAGE);

    char abs_path[PATH_MAX + 1];

    if (!realpath(filename, (char *)abs_path)) {
        return -1;
    }

    job->data->scaled_image.image.filename =
        malloc(sizeof(char) * (strlen(abs_path) + 1));
    strcpy(job->data->scaled_image.image.filename, abs_path);
    job->data->scaled_image.image.x = x;
    job->data->scaled_image.image.y = y;
    job->data->scaled_image.scale = scale;

    return 0;
}

signed char tumDrawArrow(unsigned short x1, unsigned short y1,
                         unsigned short x2, unsigned short y2,
                         unsigned short head_length, unsigned char thickness,
                         unsigned int colour)
{
    INIT_JOB(job, DRAW_ARROW);

    job->data->arrow.x1 = x1;
    job->data->arrow.y1 = y1;
    job->data->arrow.x2 = x2;
    job->data->arrow.y2 = y2;
    job->data->arrow.head_length = head_length;
    job->data->arrow.thickness = thickness;
    job->data->arrow.colour = colour;

    return 0;
}
