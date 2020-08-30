#include <SDL.h>
#include <math.h>

const float PI = 3.1415916;
const float DTR = PI / 180.f;

const int W = 400;
const int H = 200;
const int pxs = 2;

int tw, th;

struct IPoint {
	int x, y;
};

struct ILine {
	IPoint a, b;
};

struct Point {
	float x, y;
};

struct Line {
	Point a, b;
	float getDist() {
		float x = a.x - b.x;
		float y = a.y - b.y;
		return sqrt(x*x + y*y);
	}
};


struct Camera {
private:
	Point transformPoint(Point& p) {
		Point ret;
		ret.x = pos.x + p.x * cosf(dir * DTR) - p.y * sinf(dir * DTR);
		ret.y = pos.y + p.y * cosf(dir * DTR) + p.x * sinf(dir * DTR);
		return ret;
	}
public:
	Point pos;
	float dir;
	float znear, zfar;
	Line near, far;
	float h;

	void update() {
		znear = h;
		near.a.x = -znear;
		near.a.y = znear;
		near.a = transformPoint(near.a);

		near.b.x = znear;
		near.b.y = znear;
		near.b = transformPoint(near.b);

		zfar = h * 20;

		far.a.x = -zfar;
		far.a.y = zfar;
		far.a = transformPoint(far.a);

		far.b.x = zfar;
		far.b.y = zfar;
		far.b = transformPoint(far.b);
	}
};

Line getProjected(Camera& c, float v) {
	Line ret;
	float in = 1.f / c.znear;
	float ifar = 1.f / c.zfar;
	float iz = in + v * (ifar - in);
	ret.a.x = c.near.a.x * in + v * (c.far.a.x * ifar - c.near.a.x * in);
	ret.a.y = c.near.a.y * in + v * (c.far.a.y * ifar - c.near.a.y * in);

	ret.b.x = c.near.b.x * in + v * (c.far.b.x * ifar - c.near.b.x * in);
	ret.b.y = c.near.b.y * in + v * (c.far.b.y * ifar - c.near.b.y * in);

	ret.a.x /= iz;
	ret.b.x /= iz;
	ret.a.y /= iz;
	ret.b.y /= iz;
	return ret;
}

IPoint getTexCoords(float u, Line& line) {
	Point inter;
	inter.x = line.a.x + u * (line.b.x - line.a.x);
	inter.y = line.a.y + u * (line.b.y - line.a.y);

	IPoint ret;
	ret.x = inter.x;
	ret.y = -inter.y;

	ret.x = (tw + (ret.x % tw)) % tw;
	ret.y = (th + (ret.y % th)) % th;

	return ret;
}

int main(int argc, char** argv) {
	SDL_Window* win = SDL_CreateWindow("mode7", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W * pxs, 2 * H * pxs, SDL_WINDOW_SHOWN);
	SDL_Renderer* rnd = SDL_CreateRenderer(win, 0, 0);

	int buff[H][W];
	SDL_Surface* sur = SDL_LoadBMP("back.bmp");
	SDL_Surface* tex = SDL_ConvertSurfaceFormat(sur, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(sur);
	SDL_Texture* scr = SDL_CreateTexture(rnd, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, W, H);
	Camera c;
	c.pos.x = 150;
	c.pos.y = 150;
	c.h = 16;
	c.dir = 0;

	tw = tex->w;
	th = tex->h;
	bool running = true;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	while (running) {
		SDL_Event ev;

		while(SDL_PollEvent(&ev)) {
			if(ev.type == SDL_QUIT || keys[SDL_SCANCODE_ESCAPE]) running = false;
		}

		if (keys[SDL_SCANCODE_W]) {
			c.h += 0.5f;
		}
		if (keys[SDL_SCANCODE_S]) {
			c.h -= 0.5f;
		}
		if (keys[SDL_SCANCODE_LEFT]) c.dir += 1.f;
		if (keys[SDL_SCANCODE_RIGHT]) c.dir -= 1.f;

		if (keys[SDL_SCANCODE_UP]) {
			c.pos.y += cosf(c.dir * DTR);
			c.pos.x -= sinf(c.dir * DTR);
		}

		if (keys[SDL_SCANCODE_DOWN]) {
			c.pos.y -= cosf(c.dir * DTR);
			c.pos.x += sinf(c.dir * DTR);
		}

		c.update();

		SDL_SetRenderDrawColor(rnd, 100, 100, 255, 0);
		SDL_RenderClear(rnd);

		int* tdata = (int*) tex->pixels;
		for(int y = 0; y < H; y++) {
			float v = 1.f - ((float) y / (float) H);
			Line inter = getProjected(c, v);
			for(int x = 0; x < W; x++) {
				float u = (float) x / (float) W;

				IPoint coord = getTexCoords(u, inter);

				buff[y][x] = tdata[coord.y * (tex->pitch / 4) + coord.x];
			}
		}
		SDL_UpdateTexture(scr, NULL, buff, W * 4);
		SDL_Rect r;
		r.x = 0;
		r.y = H * pxs;
		r.w = W * pxs;
		r.h = H * pxs;
		SDL_RenderCopy(rnd, scr, NULL, &r);

		SDL_RenderPresent(rnd);
	}
	
	SDL_FreeSurface(tex);
	SDL_DestroyTexture(scr);
	SDL_DestroyRenderer(rnd);
	SDL_DestroyWindow(win);
	return 0;
}
