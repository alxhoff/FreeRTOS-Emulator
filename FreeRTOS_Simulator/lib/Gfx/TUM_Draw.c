#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL2_gfxPrimitives.h"

#include "TUM_Draw.h"
#include "queue.h"

typedef enum {
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
	DRAW_IMAGE
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

typedef struct circle_date {
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

typedef struct text_data {
	char *str;
	unsigned short x;
	unsigned short y;
	unsigned int colour;
} text_data_t;

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
	text_data_t text;
};

typedef struct draw_job {
	draw_job_type_t type;
	union data_u *data;
} draw_job_t;

const int screen_height = SCREEN_HEIGHT;
const int screen_width = SCREEN_WIDTH;
const int screen_x = SCREEN_X;
const int screen_y = SCREEN_Y;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
xQueueHandle drawJobQueue = NULL;

xSemaphoreHandle DisplayReady = NULL;

void vDrawUpdateScreen(void);

uint32_t SwapBytes(uint x) {
	return ((x & 0x000000ff) << 24) + ((x & 0x0000ff00) << 8)
			+ ((x & 0x00ff0000) >> 8) + ((x & 0xff000000) >> 24);
}

void logSDLError(char *msg) {
	if (msg)
		printf("[ERROR] %s, %s\n", msg, SDL_GetError());
}

void vClearDisplay(unsigned int colour) {
	SDL_SetRenderDrawColor(renderer, (colour >> 16) & 0xFF,
			(colour >> 8) & 0xFF, colour & 0xFF, 255);
	SDL_RenderClear(renderer);
}

void vDrawRectangle(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour) {

	rectangleColor(renderer, x, y, x + w, y + h,
			SwapBytes((colour << 8) | 0xFF));
}

void vDrawFilledRectangle(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour) {
	boxColor(renderer, x, y, x + w, y + h, SwapBytes((colour << 8) | 0xFF));
}

void vDrawArc(signed short x, signed short y, signed short radius,
		signed short start, signed short end, unsigned int colour) {
	arcColor(renderer, x, y, radius, start, end,
			SwapBytes((colour << 8) | 0xFF));
}

void vDrawEllipse(signed short x, signed short y, signed short rx,
		signed short ry, unsigned int colour) {
	ellipseColor(renderer, x, y, rx, ry, SwapBytes((colour << 8) | 0xFF));
}

void vDrawCircle(signed short x, signed short y, signed short radius,
		unsigned int colour) {
	filledCircleColor(renderer, x, y, radius, SwapBytes((colour << 8) | 0xFF));
}

void vDrawLine(signed short x1, signed short y1, signed short x2,
		signed short y2, unsigned int colour) {
	lineColor(renderer, x1, y1, x2, y2, SwapBytes((colour << 8) | 0xFF));
}

void vDrawPoly(coord_t *points, unsigned int n, signed short colour) {
	signed short *x_coords = calloc(1, sizeof(signed short) * n);
	signed short *y_coords = calloc(1, sizeof(signed short) * n);

	for (unsigned int i = 0; i < n; i++) {
		x_coords[i] = points[i].x;
		y_coords[i] = points[i].y;
	}

	polygonColor(renderer, x_coords, y_coords, n,
			SwapBytes((colour << 8) | 0xFF));

	free(x_coords);
	free(y_coords);

}

void vDrawTriangle(coord_t *points, unsigned int colour) {
	filledTrigonColor(renderer, points[0].x, points[0].y, points[1].x,
			points[1].y, points[2].x, points[2].y,
			SwapBytes((colour << 8) | 0xFF));
}

void vInitDrawing(void) {
	int ret = 0;

	SDL_Init( SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("FreeRTOS Simulator", SDL_WINDOWPOS_CENTERED,
	SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

	if (!window) {
		logSDLError("vInitDrawing->CreateWindow");
		SDL_Quit();
		exit(-1);
	}

	renderer = SDL_CreateRenderer(window, -1,0);
	/** renderer = SDL_CreateRenderer(window, -1, */
	/**         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); */

	if (!renderer) {
		logSDLError("vInitDrawing->CreateRenderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(-1);
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_RenderClear(renderer);

	drawJobQueue = xQueueCreate(100, sizeof(draw_job_t));
	if (!drawJobQueue) {
		printf("drawJobQueue init failed\n");
		exit(-1);
	}

	DisplayReady = xSemaphoreCreateMutex();

	if (!DisplayReady) {
		printf("DisplayReady semaphore not created\n");
		exit(-1);
	}

	atexit(SDL_Quit);
}

void vExitDrawing(void) {
	if (window)
		SDL_DestroyWindow(window);

	if (renderer)
		SDL_DestroyRenderer(renderer);

	SDL_Quit();
}

void tumDrawDelay(int delay) {
	SDL_Delay(delay);
}

SDL_Texture *loadImage(char *filename, SDL_Renderer *ren) {
	SDL_Texture *tex = NULL;

	tex = IMG_LoadTexture(ren, filename);

	if (!tex) {
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(window);
		logSDLError("loadImage->LoadBMP");
		SDL_Quit();
		exit(0);
	}

	return tex;
}

void vDrawScaledImage(SDL_Texture *tex, SDL_Renderer *ren, unsigned short x,
		unsigned short y, unsigned short w, unsigned short h) {
	SDL_Rect dst;
	dst.w = w;
	dst.h = h;
	dst.x = x;
	dst.y = y;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

void vDrawImage(SDL_Texture *tex, SDL_Renderer *ren, unsigned short x,
		unsigned short y) {
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h); //Get texture dimensions
	vDrawScaledImage(tex, ren, x, y, w, h);
}

void vDrawLoadAndDrawImage(char *filename, SDL_Renderer *ren, unsigned short x,
		unsigned short y) {
	SDL_Texture *tex = loadImage(filename, ren);

	vDrawImage(tex, ren, x, y);
}

void vDrawRectImage(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst,
		SDL_Rect *clip) {
	SDL_RenderCopy(ren, tex, clip, &dst);
}

void vDrawClippedImage(SDL_Texture *tex, SDL_Renderer *ren, unsigned short x,
		unsigned short y, SDL_Rect *clip) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;

	if (!clip) {
		dst.w = clip->w;
		dst.h = clip->h;
	} else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}

	vDrawRectImage(tex, ren, dst, clip);
}

void vDrawText(char *string, unsigned short x, unsigned short y,
		unsigned int colour) {
	stringColor(renderer, x, y, string, SwapBytes((colour << 8) | 0xFF));
}

void vHandleDrawJob(draw_job_t *job) {
	if (!job)
		return;

	switch (job->type) {
	case DRAW_CLEAR:
		vClearDisplay(job->data->clear.colour);
		break;
	case DRAW_ARC:
		vDrawArc(job->data->arc.x, job->data->arc.y, job->data->arc.radius,
				job->data->arc.start, job->data->arc.end,
				job->data->arc.colour);
		break;
	case DRAW_ELLIPSE:
		vDrawEllipse(job->data->ellipse.x, job->data->ellipse.y,
				job->data->ellipse.rx, job->data->ellipse.ry,
				job->data->ellipse.colour);
		break;
	case DRAW_TEXT:
		vDrawText(job->data->text.str, job->data->text.x, job->data->text.y,
				job->data->text.colour);
		free(job->data->text.str);
		break;
	case DRAW_RECT:
		vDrawRectangle(job->data->rect.x, job->data->rect.y, job->data->rect.w,
				job->data->rect.h, job->data->rect.colour);
		break;
	case DRAW_FILLED_RECT:
		vDrawFilledRectangle(job->data->rect.x, job->data->rect.y,
				job->data->rect.w, job->data->rect.h, job->data->rect.colour);
		break;
	case DRAW_CIRCLE:
		vDrawCircle(job->data->circle.x, job->data->circle.y,
				job->data->circle.radius, job->data->circle.colour);
		break;
	case DRAW_LINE:
		vDrawLine(job->data->line.x1, job->data->line.y1, job->data->line.x2,
				job->data->line.y2, job->data->line.colour);
		break;
	case DRAW_POLY:
		vDrawPoly(job->data->poly.points, job->data->poly.n,
				job->data->poly.colour);
		break;
	case DRAW_TRIANGLE:
		vDrawTriangle(job->data->triangle.points, job->data->triangle.colour);
		break;
	case DRAW_IMAGE:
		job->data->image.tex = loadImage(job->data->image.filename, renderer);
		vDrawImage(job->data->image.tex, renderer, job->data->image.x,
				job->data->image.y);
		break;
	default:
		break;
	}
	free(job->data);
}

void vDrawUpdateScreen(void) {
	draw_job_t tmp_job = { 0 };

	while (xQueueReceive(drawJobQueue, &tmp_job, 0) == pdTRUE)
		vHandleDrawJob(&tmp_job);

    SDL_RenderPresent(renderer);

	xSemaphoreGive(DisplayReady);
}

void vSetupScreen(void) {
}

#define CREATE_JOB(TYPE) \
    union data_u *data = calloc(1, sizeof(union data_u));\
    if(!data) \
        logCriticalError("#TYPE data alloc");\
    job.data = data;

void logCriticalError(char *msg) {
	printf("[ERROR] %s\n", msg);
	exit(-1);
}

signed char tumDrawText(char *str, signed short x, signed short y,
		unsigned int colour) {
	draw_job_t job = { .type = DRAW_TEXT };

	CREATE_JOB(text);

	job.data->text.str = malloc(sizeof(char) * (strlen(str) + 1));
	//TODO checking
	strcpy(job.data->text.str, str);
	job.data->text.x = x;
	job.data->text.y = y;
	job.data->text.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawEllipse(signed short x, signed short y, signed short rx,
		signed short ry, unsigned int colour) {
	draw_job_t job = { .type = DRAW_ELLIPSE };

	CREATE_JOB(ellipse);

	job.data->ellipse.x = x;
	job.data->ellipse.y = y;
	job.data->ellipse.rx = rx;
	job.data->ellipse.ry = ry;
	job.data->ellipse.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;

}

signed char tumDrawArc(signed short x, signed short y, signed short radius,
		signed short start, signed short end, unsigned int colour) {
	draw_job_t job = { .type = DRAW_ARC };

	CREATE_JOB(arc);

	job.data->arc.x = x;
	job.data->arc.y = y;
	job.data->arc.radius = radius;
	job.data->arc.start = start;
	job.data->arc.end = end;
	job.data->arc.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;

}

signed char tumDrawFilledBox(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour) {
	draw_job_t job = { .type = DRAW_FILLED_RECT };

	CREATE_JOB(rect);

	job.data->rect.x = x;
	job.data->rect.y = y;
	job.data->rect.w = w;
	job.data->rect.h = h;
	job.data->rect.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawBox(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour) {
	draw_job_t job = { .type = DRAW_RECT };

	CREATE_JOB(rect);

	job.data->rect.x = x;
	job.data->rect.y = y;
	job.data->rect.w = w;
	job.data->rect.h = h;
	job.data->rect.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawClear(unsigned int colour) {
	draw_job_t job = { .type = DRAW_CLEAR };

	CREATE_JOB(clear);

	job.data->clear.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawCircle(signed short x, signed short y, signed short radius,
		unsigned int colour) {
	draw_job_t job = { .type = DRAW_CIRCLE };

	CREATE_JOB(circle);

	job.data->circle.x = x;
	job.data->circle.y = y;
	job.data->circle.radius = radius;
	job.data->circle.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawLine(signed short x1, signed short y1, signed short x2,
		signed short y2, unsigned int colour) {
	draw_job_t job = { .type = DRAW_LINE };

	CREATE_JOB(line);

	job.data->line.x1 = x1;
	job.data->line.y1 = y1;
	job.data->line.x2 = x2;
	job.data->line.y2 = y2;
	job.data->line.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawPoly(coord_t *points, int n, unsigned int colour) {
	draw_job_t job = { .type = DRAW_POLY };

	CREATE_JOB(poly);

	job.data->poly.points = points;
	job.data->poly.n = n;
	job.data->poly.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawTriangle(coord_t *points, unsigned int colour) {
	draw_job_t job = { .type = DRAW_TRIANGLE };

	CREATE_JOB(triangle);

	job.data->triangle.points = points;
	job.data->triangle.colour = colour;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;
}

signed char tumDrawImage(char *filename, signed short x, signed short y) {
	draw_job_t job = { .type = DRAW_IMAGE };

	CREATE_JOB(image);

	job.data->image.filename = filename;
	job.data->image.x = x;
	job.data->image.y = y;

	if (xQueueSend(drawJobQueue, &job, portMAX_DELAY) != pdTRUE)
		return -1;

	return 0;

}
