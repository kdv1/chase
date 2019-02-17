
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <p3.h>
#include "blitter.h"

#define LINEAR_FILTER 1
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

extern void chase_main(void);
extern void load_music();
extern void unload_music();
extern void neslib_nmi_handler();

static int is_exit;
static SDL_Window *window;
static SDL_Renderer* renderer;
static SDL_Texture *texture;
static unsigned char tileset[P3_CHR_SIZE_8];
static int test_mmc;

static void load_tileset() {
	FILE *file = fopen("gfx/tileset.chr", "rb");
	if (file) {
		fread(tileset, 1, P3_CHR_SIZE_8, file);
		fclose(file);
	}
}

static void initialize() {
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_Init(MIX_INIT_OGG);

	window = SDL_CreateWindow("Chase", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, LINEAR_FILTER ? "linear" : "nearest");
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

	load_tileset();
	p3_create_object(tileset, P3_CHR_SIZE_8);
	blitter_init();
	load_music();
}

static void uninitialize() {
	unload_music();
	p3_destroy_object(NULL);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_Quit();
	Mix_CloseAudio();
	SDL_Quit();
}

static struct wave_state {
	int x;
	int frame;
} wave_state;

static void wave_raster_effect(int x, int y, void *param) {
	struct wave_state *wave = (struct wave_state*) param;
	switch (y) {
	case P3_CALLBACK_BEGIN:
		--wave->frame;
		if (wave->frame < 0)
			wave->frame = 239;
		p3_save_register(P3_REGISTER_T);
		wave->x = p3_get_scroll_x();
		break;
	case P3_CALLBACK_END:
		p3_restore_register(P3_REGISTER_T);
		break;

	default:
		p3_set_scroll_x(((int) (cos((y + wave->frame) / 100.0 * 6.2831)
		                   * wave->frame/5 + wave->x)) & 0xff);
	}
}

static void sdl_process_events() {
	static SDL_Event event;
	SDL_PumpEvents();
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_CLOSE)
				is_exit = 1;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_R:
				p3_set_tint(p3_get_tint() ^ P3_TINT_RED);
				break;
			case SDL_SCANCODE_G:
				p3_set_tint(p3_get_tint() ^ P3_TINT_GREEN);
				break;
			case SDL_SCANCODE_B:
				p3_set_tint(p3_get_tint() ^ P3_TINT_BLUE);
				break;
			case SDL_SCANCODE_C:
				p3_enable_grayscale(p3_is_grayscale() ^ 1);
				break;
			case SDL_SCANCODE_E:
				wave_state.frame = 239;
				p3_set_callback(wave_raster_effect, P3_CALLBACK_SCANLINE, 0, 0, &wave_state);
				p3_enable_callback(p3_is_callback_enabled() ^ 1);
				break;
			case SDL_SCANCODE_M:
				test_mmc ^= 1;
				if (test_mmc) {
					p3_set_mmc_mode(P3_CHR_TABLE_LEFT, P3_MMC_MODE_1X4);
					p3_set_mmc_mode(P3_CHR_TABLE_RIGHT, P3_MMC_MODE_1X4);
				} else {
					p3_set_bank(P3_CHR_TABLE_LEFT, P3_BANK_0K, 0);
					p3_set_bank(P3_CHR_TABLE_RIGHT, P3_BANK_0K, 1);
					p3_set_mmc_mode(P3_CHR_TABLE_LEFT, P3_MMC_MODE_4X1);
					p3_set_mmc_mode(P3_CHR_TABLE_RIGHT, P3_MMC_MODE_4X1);
				}
			case SDL_SCANCODE_Y:
				p3_fix_obj_y(p3_is_fix_obj_y() ^ 1);
				break;
			}
			break;
		}
	}
}

void sdl_render_frame() {
	static unsigned char frame;

	if (!is_exit) {
		sdl_process_events();
		neslib_nmi_handler();
		p3_render();
		blitter_blit(texture, (const uint16_t *) p3_get_frame_pointer());
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	} else {
		uninitialize();
		exit(0);
	}

	if (test_mmc && (++frame % 60 == 0)) {
		static int bank = 0;

		p3_set_bank(P3_CHR_TABLE_LEFT, P3_BANK_0K, (bank + 7) & 7);
		p3_set_bank(P3_CHR_TABLE_LEFT, P3_BANK_1K, (bank + 6) & 7);
		p3_set_bank(P3_CHR_TABLE_LEFT, P3_BANK_2K, (bank + 5) & 7);
		p3_set_bank(P3_CHR_TABLE_LEFT, P3_BANK_3K, (bank + 4) & 7);

		p3_set_bank(P3_CHR_TABLE_RIGHT, P3_BANK_0K, (bank + 3) & 7);
		p3_set_bank(P3_CHR_TABLE_RIGHT, P3_BANK_1K, (bank + 2) & 7);
		p3_set_bank(P3_CHR_TABLE_RIGHT, P3_BANK_2K, (bank + 1) & 7);
		p3_set_bank(P3_CHR_TABLE_RIGHT, P3_BANK_3K, (bank) & 7);

		bank = (bank + 1) & 7;
	}
}

/* noreturn */
int main(int argc, char* argv[])
{
	initialize();	
	p3_enable(0);
	p3_show_bg(0);
	p3_show_obj(0);
	chase_main();
	return 0;
}
