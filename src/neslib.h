
/* Minimal port of neslib for chase game */

#define NAMETABLE_A		P3_TOP_LEFT_NAMETABLE
#define NAMETABLE_B		P3_TOP_RIGHT_NAMETABLE
#define NAMETABLE_C		P3_BOTTOM_LEFT_NAMETABLE
#define NAMETABLE_D		P3_BOTTOM_RIGHT_NAMETABLE

#define TRUE			1
#define FALSE			0

#define NT_UPD_HORZ		0x40
#define NT_UPD_VERT		0x80
#define NT_UPD_EOF		0xff

#define PAD_A			0x01
#define PAD_B			0x02
#define PAD_SELECT		0x04
#define PAD_START		0x08
#define PAD_UP			0x10
#define PAD_DOWN		0x20
#define PAD_LEFT		0x40
#define PAD_RIGHT		0x80

extern void sdl_render_frame();

static unsigned char palette_buffer[32];
static int is_update_palette;

//set bg palette only, data is 16 bytes array
static void pal_bg(const char *data) {
	memcpy(palette_buffer, data, 16);
	is_update_palette = 1;
}

//set spr palette only, data is 16 bytes array
static void pal_spr(const char *data) {
	memcpy(palette_buffer+16, data, 16);
	is_update_palette = 1;
}

//set a palette entry, index is 0..31
static void pal_col(unsigned char index,unsigned char color) {
	palette_buffer[index & 0x1f] = color;
	is_update_palette = 1;
}

static unsigned char palBrightTable0[] = {
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f}; //black
static unsigned char palBrightTable1[] = {
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f};
static unsigned char palBrightTable2[] = {
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f};
static unsigned char palBrightTable3[] = {
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f};
static unsigned char palBrightTable4[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0f,0x0f,0x0f};	//;normal colors
static unsigned char palBrightTable5[] = {
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x00,0x00,0x00};
static unsigned char palBrightTable6[] = {
	0x10,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x10,0x10,0x10};	//;0x10 because 0x20 is the same as 0x30
static unsigned char palBrightTable7[] = {
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x20,0x20,0x20};
static unsigned char palBrightTable8[] = {
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,	//;white
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};

static unsigned char *palBrightTable[] = {
	(unsigned char*) &palBrightTable0, 
	(unsigned char*) &palBrightTable1, 
	(unsigned char*) &palBrightTable2,
	(unsigned char*) &palBrightTable3,
	(unsigned char*) &palBrightTable4,
	(unsigned char*) &palBrightTable5,
	(unsigned char*) &palBrightTable6,
	(unsigned char*) &palBrightTable7,
	(unsigned char*) &palBrightTable8 };

//set virtual bright both for sprites and background, 0 is black, 4 is normal, 8 is white
static unsigned char *pal_spr_ptr, *pal_bg_ptr;
static void pal_bright(unsigned char bright) {
	pal_spr_ptr = palBrightTable[bright];
	pal_bg_ptr = palBrightTable[bright];
	is_update_palette = 1;
}

static void update_palette() {
	int i;
	is_update_palette = 0;

	p3_set_color(0, pal_bg_ptr[palette_buffer[0]]);
	for (i = 1; i < 16; ++i)
		if (i % 4)
			p3_set_color(i, pal_bg_ptr[palette_buffer[i]]);

	for (i = 17; i < 32; ++i)
		if (i % 4)
			p3_set_color(i, pal_spr_ptr[palette_buffer[i]]);
}

//wait virtual frame, it is always 50hz, frame-to-frame in PAL, frameskip in NTSC
static void ppu_wait_frame(void) {
	static unsigned char frame_cnt = 0;
	
	// wait using vsync
	sdl_render_frame();
	++frame_cnt;

	// skip frame
	if (frame_cnt % 5 == 0)
		sdl_render_frame();
}

//turn off rendering, nmi still enabled when rendering is disabled
static void ppu_off(void) {
	p3_show_bg(0);
	p3_show_obj(0);
}

//turn on bg, spr
static void ppu_on_all(void) {
	p3_enable(1);
	p3_show_bg(1);
	p3_show_obj(1);
}

//turn on bg only
static void ppu_on_bg(void) { p3_show_bg(1); }

static unsigned char oam_buf[256];

static void transfer_oam_buf() {
	P3_SPRITE spr;
	int i;

	spr.flip_horizontal = 0;
	spr.flip_vertical = 0;
	spr.priority = P3_SPRITE_FRONT;
	for (i = 0; i < 64; ++i) {
		spr.y = oam_buf[i * 4];
		spr.tile = oam_buf[i * 4 +1];
		spr.palette = oam_buf[i * 4 + 2];
		spr.x = oam_buf[i * 4 + 3];

		p3_put_sprite(i, &spr);
	}
}


//clear OAM buffer, all the sprites are hidden
static void oam_clear(void) { memset(oam_buf, 0xff, sizeof(oam_buf)); }

//set metasprite in OAM buffer
//meta sprite is a const unsigned char array, it contains four bytes per sprite
//in order x offset, y offset, tile, attribute
//x=128 is end of a meta sprite
//returns sprid+4, which is offset for a next sprite
static unsigned char oam_meta_spr(unsigned char x, unsigned char y,
                            unsigned char sprid,const unsigned char *data)
{
	const unsigned char *p = data;
	for (;;) {
		if (*p == 0x80)
			return sprid;
		*(oam_buf + 3 + sprid) = *p++ + x;   // x offset
		*(oam_buf + 0 + sprid) = *p++ + y;   // y offset
		*(oam_buf + 1 + sprid) = *p++;       // tile
		*(oam_buf + 2 + sprid) = *p++;       // attribute
		sprid += 4;
	}
}

static unsigned char pad_state_;
static unsigned char pad_state_p;
static unsigned char pad_state_t;

static void poll_pad() {
	unsigned char pad_buf = 0;
	const Uint8 *kb_state = SDL_GetKeyboardState(NULL);

	if (kb_state[SDL_SCANCODE_K])
		pad_buf |= PAD_B;
	if (kb_state[SDL_SCANCODE_L])
		pad_buf |= PAD_A;
	if (kb_state[SDL_SCANCODE_SPACE])
		pad_buf |= PAD_SELECT;
	if (kb_state[SDL_SCANCODE_RETURN])
		pad_buf |= PAD_START;
	if (kb_state[SDL_SCANCODE_W])
		pad_buf |= PAD_UP;
	if (kb_state[SDL_SCANCODE_S])
		pad_buf |= PAD_DOWN;
	if (kb_state[SDL_SCANCODE_A])
		pad_buf |= PAD_LEFT;
	if (kb_state[SDL_SCANCODE_D])
		pad_buf |= PAD_RIGHT;

	pad_state_ = pad_buf;
	pad_state_t = (pad_state_ ^ pad_state_p) & pad_state_;
	pad_state_p = pad_state_;
}

//poll controller in trigger mode, a flag is set only on button down, not hold
//if you need to poll the pad in both normal and trigger mode, poll it in the
//trigger mode for first, then use pad_state
static unsigned char pad_trigger(unsigned char pad) {
	poll_pad();
	return pad_state_t;
}

//get previous pad state without polling ports
static unsigned char pad_state(unsigned char pad) { return pad_state_; }

//set scroll, including rhe top bits
static void scroll(unsigned int x,unsigned int y) {
	int page = 0;
	
	if (y >= 240) {
		y -= 240;
		page = 2;
	}

	page |= (x >> 8) & 1;
	
	p3_set_page(page);
	p3_set_scroll(x & 0xff, y & 0xff);
}

//select current chr bank for sprites, 0..1
static void bank_spr(unsigned char n) { p3_set_obj_chr_table(n); }

//select current chr bank for background, 0..1
static void bank_bg(unsigned char n) { p3_set_bg_chr_table(n); }

//set random seed
static unsigned short rand_seed;
static unsigned char rand_seed_c;
static void set_rand(unsigned short seed) {	rand_seed = seed; }

static void rand8_1(void) {
	unsigned char seed = rand_seed & 0xff;
	if (seed & 0x80) {
		seed = (seed << 1) ^ 0xcf;
	} else {
		seed = seed << 1;
	}
	rand_seed = (rand_seed & 0xff00) | seed;
}

static void rand8_2(void) {
	unsigned char seed = rand_seed >> 8;
	rand_seed_c = (seed & 0x80) >> 7;
	if (rand_seed_c) {
		seed = (seed << 1) ^ 0xd7;
	} else {
		seed = seed << 1;
	}
	rand_seed = (rand_seed & 0x00ff) | (((unsigned short) seed) << 8);
}

//get random number 0..255
static unsigned char rand8(void) {
	rand8_1();
	rand8_2();
	return (rand_seed >> 8) + (rand_seed & 0xff) + rand_seed_c;
}


//when display is enabled, vram access could only be done with this vram update system
//the function sets a pointer to the update buffer that contains data and addresses
//in a special format. It allows to write non-sequental bytes, as well as horizontal or
//vertical nametable sequences.
//buffer pointer could be changed during rendering, but it only takes effect on a new frame
//number of transferred bytes is limited by vblank time
//to disable updates, call this function with NULL pointer

//the update data format:
// MSB, LSB, byte for a non-sequental write
// MSB|NT_UPD_HORZ, LSB, LEN, [bytes] for a horizontal sequence
// MSB|NT_UPD_VERT, LSB, LEN, [bytes] for a vertical sequence
// NT_UPD_EOF to mark end of the buffer

//length of this data should be under 256 bytes
static unsigned char *vram_buf;
static void set_vram_update(unsigned char *buf) { vram_buf = buf; }

static void flush_vram_buf() {
	if (vram_buf) {
		unsigned char op, op2, *p = vram_buf;
		int prev_inc = p3_get_increment();
_updName:
		op = *p++;
		if (op >= 0x40) {
			op2 = op;
			if (op == 0xff) {
				goto _exit;
			} else if (op & 0x40) {
				p3_set_increment(P3_INCREMENT_HORIZONTAL);
			} else {
				p3_set_increment(P3_INCREMENT_VERTICAL);
			}
		} else {
			p3_set_address((((unsigned int) op << 8) | *p++) & 0xfff);
			p3_put_byte(*p++);
			goto _updName;
		}

		op = op2;
		op &= 0x3f;
		p3_set_address((((unsigned int) op << 8) | *p++) & 0xfff);
		op2 = *p++;
		do {
			op = *p++;
			p3_put_byte(op);
			--op2;
		} while (op2);
		p3_set_increment(prev_inc);
		goto _updName;
_exit:
		p3_set_increment(prev_inc);
	}
}

//set vram pointer to write operations if you need to write some data to vram
static void vram_adr(unsigned int adr) { p3_set_address(adr); }

//put a byte at current vram address, works only when rendering is turned off
static void vram_put(unsigned char n) { p3_put_byte(n); }

//fill a block with a byte at current vram address, works only when rendering is turned off
static void vram_fill(unsigned char n,unsigned int len) { p3_fill(n, len); }

//read a block from current address of vram, works only when rendering is turned off
static void vram_read(unsigned char *dst,unsigned int size) { p3_read(dst, size); }

//write a block to current address of vram, works only when rendering is turned off
static void vram_write(unsigned char *src,unsigned int size) { p3_write(src, size); }

//delay for N frames
static void delay(unsigned char frames) {
	int i;
	for (i = 0; i < frames; ++i) {
		sdl_render_frame();
	}
}

void neslib_nmi_handler() {
	transfer_oam_buf();
	update_palette();
	flush_vram_buf();
}
