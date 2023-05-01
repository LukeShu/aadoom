#include "doomkeys.h"
#include "doomgeneric.h"
#include <aalib.h>

#include <ctype.h>  /* for tolower(3p) */
#include <error.h>  /* for error(3gnu) */
#include <stdio.h>  /* for puts(3p) */
#include <stdlib.h> /* for setenv(3p) and unsetenv(3p) */

static aa_context *context = NULL;

#define div_roundup(n, d) (((n) + (d) - 1) / (d))

int main(int argc, char **argv) {
	/* AAOPTS */
	if (!aa_parseoptions(NULL, NULL, NULL, NULL)) {
		puts(aa_help);
		return 2;
	}
	
	doomgeneric_Create(argc, argv);
	for (;;)
		doomgeneric_Tick();

	if (context != NULL)
		aa_close(context);

	return 0;
}

void DG_Init() {}

void DG_SetWindowTitle(const char *title) {}

void DG_DrawFrame() {
	if (context == NULL) {
		context = aa_autoinit(&aa_defparams);
		if (context == NULL)
			error(1, 0, "failed to initialize aalib graphics");
		if (!aa_autoinitkbd(context, 0)) {
			aa_close(context);
			error(1, 0, "failed to initialize aalib keyboard");
		}
		aa_hidecursor(context);
	}

	unsigned char *out_buffer = aa_image(context);

	int doomx_per_outx = div_roundup(DOOMGENERIC_RESX, aa_imgwidth(context));
	int doomy_per_outy = div_roundup(DOOMGENERIC_RESY, aa_imgheight(context)-1);
	int out_resx = DOOMGENERIC_RESX / doomx_per_outx;
	int out_resy = DOOMGENERIC_RESY / doomy_per_outy;
	int full_out_resx = aa_imgwidth(context);
	int out_xoff = (full_out_resx - out_resx) / 2;

	uint32_t v, r, g, b;
	for (int oy = 0; oy < out_resy; oy++) {
		for (int ox = 0; ox < out_resx; ox++) {
			v = 0;
			for (int dy = oy*doomy_per_outy; dy < (oy+1)*doomy_per_outy; dy++) {
				for (int dx = ox*doomx_per_outx; dx < (ox+1)*doomx_per_outx; dx++) {
					r = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 24) & 0xFF;
					g = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >> 16) & 0xFF;
					b = (DG_ScreenBuffer[dy*DOOMGENERIC_RESX+dx] >>  8) & 0xFF;
					v += (r*30 + g*59 + b*11) / 100;
				}
			}
			out_buffer[oy*full_out_resx+out_xoff+ox] = v/(doomx_per_outx*doomy_per_outy);
		}
	}

	aa_render(context, &aa_defrenderparams,
	          /* TTY X1 */ out_xoff,
	          /* TTY Y1 */ 0,
	          /* TTY X2 */ out_xoff + out_resx,
	          /* TTY Y2 */ out_resy);

	aa_flush(context);
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
	if (context == NULL)
		return 0;

	printf("aaa\r");
	int event = aa_getevent(context, 0);
	printf("bbb\r");
	if (!event)
		return 0;

	*pressed = event < AA_RELEASE;
	event &= ~AA_RELEASE;

	switch (event) {
	case AA_UP:
		*doomKey = KEY_UPARROW;
		break;
	case AA_DOWN:
		*doomKey = KEY_DOWNARROW;
		break;
	case AA_LEFT:
		*doomKey = KEY_LEFTARROW;
		break;
	case AA_RIGHT:
		*doomKey = KEY_RIGHTARROW;
		break;
	case AA_BACKSPACE:
		*doomKey = KEY_BACKSPACE;
		break;
	case AA_ESC:
		*doomKey = KEY_ESCAPE;
		break;
	default:
		*doomKey = (unsigned char)tolower(event);
		if (event >= 127)
			aa_printf(context, 0, aa_scrheight(context)-1, AA_NORMAL,
			          "unknown keycode: %d", event);
	}
	aa_printf(context, 0, aa_scrheight(context)-1, AA_NORMAL,
	          "doomkey=%d pressed=%d                    ", *doomKey, *pressed);
	return 1;
}