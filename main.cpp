#include<SDL2/SDL.h>
#include<math.h>

const float PI = 3.1415916;
const float DTR = PI / 180.f;

const int W = 800;
const int H = 300;
const int pxs = 1;

int tw, th;

const float fov = 90 * (3.1415916f / 180.f);
float zn = 20.f;
float zf = 500.f;
float ndist, fdist;

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


struct Player {
	Point pos;
	float dir;

	Line getPlane(float dist) {
		Line line;

		line.a.x = pos.x + cosf((dir * DTR) - fov * 0.5f) * dist;
		line.a.y = pos.y + sinf((dir * DTR) - fov * 0.5f) * dist;

		line.b.x = pos.x + cosf((dir * DTR) + fov * 0.5f) * dist;
		line.b.y = pos.y + sinf((dir * DTR) + fov * 0.5f) * dist;

		return line;
	}
};

Line getProjected(float v, Line& near, Line& far) {
	Line ret;
	float in = 1.f / zn;
	float ifar = 1.f / zf;
	float iz = in + v * (ifar - in);
	ret.a.x = near.a.x * in + v * (far.a.x * ifar - near.a.x * in);
	ret.a.y = near.a.y * in + v * (far.a.y * ifar - near.a.y * in);

	ret.b.x = near.b.x * in + v * (far.b.x * ifar - near.b.x * in);
	ret.b.y = near.b.y * in + v * (far.b.y * ifar - near.b.y * in);

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
	Player p;
	p.pos.x = 150;
	p.pos.y = 150;

	p.dir = 0;

	tw = tex->w;
	th = tex->h;
	bool running = true;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	while (running) {
		SDL_Event ev;

		while(SDL_PollEvent(&ev)) {
			if(ev.type == SDL_QUIT) running = false;
		}

		if (keys[SDL_SCANCODE_W]) {
			zn += 0.5f;
			zf += 0.5f;
		}
		if (keys[SDL_SCANCODE_S]) {
			zn -= 0.5f;
			zf -= 0.5f;
		}
		if (keys[SDL_SCANCODE_LEFT]) p.dir -= 1.f;
		if (keys[SDL_SCANCODE_RIGHT]) p.dir += 1.f;

		if (keys[SDL_SCANCODE_UP]) {
			p.pos.x += cosf(p.dir * DTR);
			p.pos.y += sinf(p.dir * DTR);
		}

		if (keys[SDL_SCANCODE_DOWN]) {
			p.pos.x -= cosf(p.dir * 3.1415916f / 180.f);
			p.pos.y -= sinf(p.dir * 3.1415916f / 180.f);
		}

		//p.pos.x += 1.f;
		//p.dir += 1.f;

		Line near = p.getPlane(zn);
		Line far = p.getPlane(zf);
		ndist = near.getDist();
		fdist = far.getDist();

		SDL_SetRenderDrawColor(rnd, 100, 100, 255, 0);
		SDL_RenderClear(rnd);

		int* tdata = (int*) tex->pixels;
		for(int y = 0; y < H; y++) {
			float v = 1.f - ((float) y / (float) H);
			Line inter = getProjected(v, near, far);
			for(int x = 0; x < W; x++) {
				float u = (float) x / (float) W;

				IPoint coord = getTexCoords(u, inter);

				buff[y][x] = tdata[coord.y * (tex->pitch / 4) + coord.x];
			}
		}
		SDL_UpdateTexture(scr, NULL, buff, W * 4);
		SDL_Rect r;
		r.x = 0;
		r.y = H;
		r.w = W;
		r.h = H;
		SDL_RenderCopy(rnd, scr, NULL, &r);

		SDL_RenderPresent(rnd);
	}
	
	SDL_FreeSurface(tex);
	SDL_DestroyTexture(scr);
	SDL_DestroyRenderer(rnd);
	SDL_DestroyWindow(win);
	return 0;
}
