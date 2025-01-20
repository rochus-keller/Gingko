/*
 * 2024 modified by me@rochus-keller.ch for keyboard layout independence
 * */

#include "version.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sdldefs.h"
#include "byteswapdefs.h"
#include "lispemul.h"
#include "lsptypes.h"
#include "keyboard.h"
#include "lspglob.h"  // for IOPage
#include "display.h"  // for CURSORHEIGHT, DisplayRegion68k

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_surface.h>

static SDL_Window *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_RendererInfo sdl_rendererinfo = {0};
static SDL_Texture *sdl_texture = NULL;
static Uint32 sdl_foreground_color;
static Uint32 sdl_background_color;
static Uint32 sdl_foreground;
static Uint32 sdl_background;
static int sdl_bytesperpixel;
static SDL_PixelFormat *sdl_pixelformat;
static int sdl_window_focusp = 0;
extern void kb_trans(uint16_t keycode, uint16_t upflg);
extern int error(const char *s);

/* clang-format off */

static struct { int lispcode; int scancode; } sdlScanCodeToLisp[] =
{
    // these are all keys usually not used for a printable character
    {13, SDL_SCANCODE_DELETE},
    {14, SDL_SCANCODE_SCROLLLOCK},
    {15, SDL_SCANCODE_BACKSPACE},
    {31, SDL_SCANCODE_LALT},
    {33, SDL_SCANCODE_ESCAPE},
    {34, SDL_SCANCODE_TAB},
    {36, SDL_SCANCODE_LCTRL},
    {41, SDL_SCANCODE_LSHIFT},
    {44, SDL_SCANCODE_RETURN},
    {76, SDL_SCANCODE_KP_ENTER},
    {46, SDL_SCANCODE_F20},
    {47, SDL_SCANCODE_RCTRL},
    {56, SDL_SCANCODE_CAPSLOCK},
    {57, SDL_SCANCODE_SPACE},
    {60, SDL_SCANCODE_RSHIFT},
    {61, SDL_SCANCODE_PAUSE},
    {62, SDL_SCANCODE_F18},
    {62, SDL_SCANCODE_HOME},
    {63, SDL_SCANCODE_PAGEUP},
    {66, SDL_SCANCODE_F7},
    {67, SDL_SCANCODE_F4},
    {68, SDL_SCANCODE_F5},
    {69, SDL_SCANCODE_DOWN},
    {73, SDL_SCANCODE_NUMLOCKCLEAR},
    {80, SDL_SCANCODE_F9},
    {82, SDL_SCANCODE_UP},
    {84, SDL_SCANCODE_LEFT},
    {86, SDL_SCANCODE_LGUI},
    {87, SDL_SCANCODE_RIGHT},
    {88, SDL_SCANCODE_RGUI},
    {89, SDL_SCANCODE_INSERT},
    {90, SDL_SCANCODE_END},
    {92, SDL_SCANCODE_PRINTSCREEN},
    // {93, SDL_SCANCODE_RALT}, // this interferes with some Medley feature, so we don't send it
    {99, SDL_SCANCODE_F2},
    {100, SDL_SCANCODE_F3},
    {101, SDL_SCANCODE_F6},
    {104, SDL_SCANCODE_F8},
    {106, SDL_SCANCODE_F10},
    {107, SDL_SCANCODE_F11},
    {108, SDL_SCANCODE_F12},
    {109, SDL_SCANCODE_F13},
    {110, SDL_SCANCODE_F22},
    {111, SDL_SCANCODE_F17},
    {0,0}
};

static int shortcutScanCodeToLisp[] =
{
    0, 0, 0, 0,
    21, // SDL_SCANCODE_A = 4
    39, // SDL_SCANCODE_B
    37, // SDL_SCANCODE_C
    5, // SDL_SCANCODE_D
    3, // SDL_SCANCODE_E
    35, // SDL_SCANCODE_F
    50, // SDL_SCANCODE_G
    52, // SDL_SCANCODE_H
    23, // SDL_SCANCODE_I
    38, // SDL_SCANCODE_J
    9, // SDL_SCANCODE_K
    26, // SDL_SCANCODE_L
    55, // SDL_SCANCODE_M
    54, // SDL_SCANCODE_N
    25, // SDL_SCANCODE_O
    11, // SDL_SCANCODE_P
    19, // SDL_SCANCODE_Q
    48, // SDL_SCANCODE_R
    20, // SDL_SCANCODE_S
    49, // SDL_SCANCODE_T
    6, // SDL_SCANCODE_U
    7, // SDL_SCANCODE_V
    18, // SDL_SCANCODE_W
    24, // SDL_SCANCODE_X
    51, // SDL_SCANCODE_Y
    40, // SDL_SCANCODE_Z = 29
};

static uint8_t lshift = 0;
static uint8_t rshift = 0;
static uint8_t lctrl = 0;
static uint8_t rctrl = 0;
static SDL_Keycode lastKey = SDLK_UNKNOWN;

static struct { char ch; uint8_t code; uint8_t shift; } charToLisp[] =
{
    {'5', 0, 0},
    {'%', 0, 1},
    {'4', 1, 0},
    {'$', 1, 1},
    {'6', 2, 0},
    {'^', 2, 1}, // orig {'~', 2, 1},
    {'e', 3, 0},
    {'E', 3, 1},
    {'7', 4, 0},
    {'&', 4, 1},
    {'d', 5, 0},
    {'D', 5, 1},
    {'u', 6, 0},
    {'U', 6, 1},
    {'v', 7, 0},
    {'V', 7, 1},
    {'0', 8, 0},
    {')', 8, 1},
    {'k', 9, 0},
    {'K', 9, 1},
    {'-', 10, 0},
    {'_', 10, 1}, // prints left arrow instead of underscore, which is expected
    {'p', 11, 0},
    {'P', 11, 1},
    {'/', 12, 0},
    {'?', 12, 1},
    {'\\', 105, 0}, // orig {'\\', 13, 0},
    {'|', 105, 1},   // orig {'|', 13, 1},
    {'3', 16, 0},
    {'#', 16, 1},
    {'2', 17, 0},
    {'@', 17, 1},
    {'w', 18, 0},
    {'W', 18, 1},
    {'q', 19, 0},
    {'Q', 19, 1},
    {'s', 20, 0},
    {'S', 20, 1},
    {'a', 21, 0},
    {'A', 21, 1},
    {'9', 22, 0},
    {'(', 22, 1},
    {'i', 23, 0},
    {'I', 23, 1},
    {'x', 24, 0},
    {'X', 24, 1},
    {'o', 25, 0},
    {'O', 25, 1},
    {'l', 26, 0},
    {'L', 26, 1},
    {',', 27, 0},
    {'<', 27, 1},
    {'\'', 28, 0},
    {'"', 28, 1},
    {']', 29, 0},
    {'}', 29, 1},
    {'1', 32, 0},
    {'!', 32, 1},
    {'f', 35, 0},
    {'F', 35, 1},
    {'c', 37, 0},
    {'C', 37, 1},
    {'j', 38, 0},
    {'J', 38, 1},
    {'b', 39, 0},
    {'B', 39, 1},
    {'z', 40, 0},
    {'Z', 40, 1},
    {'.', 42, 0},
    {'>', 42, 1},
    {';', 43, 0},
    {':', 43, 1},
    {'`', 45, 0},
    {'~', 45, 1}, // orig {'^', 45, 1},
    {'r', 48, 0},
    {'R', 48, 1},
    {'t', 49, 0},
    {'T', 49, 1},
    {'g', 50, 0},
    {'G', 50, 1},
    {'y', 51, 0},
    {'Y', 51, 1},
    {'h', 52, 0},
    {'H', 52, 1},
    {'8', 53, 0},
    {'*', 53, 1},
    {'n', 54, 0},
    {'N', 54, 1},
    {'m', 55, 0},
    {'M', 55, 1},
    {'[', 58, 0},
    {'{', 58, 1},
    {'=', 59, 0},
    {'+', 59, 1},
    // space: scancode
    {0,0,0}
};

const struct ColorNameToRGB {
  const char * name; uint8_t red; uint8_t green; uint8_t blue;
} colornames[] = {
{"black",  0, 0, 0},
{"white",  255, 255, 255},
{0,  0, 0, 0},

};

/* clang-format on */
static const DLword bitmask[16] = {1 << 15, 1 << 14, 1 << 13, 1 << 12, 1 << 11, 1 << 10,
                                   1 << 9,  1 << 8,  1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                   1 << 3,  1 << 2,  1 << 1,  1 << 0};
// all of the following are overwritten, the values here are irrelevant defaults!
// actual size of the lisp display in pixels.
int sdl_displaywidth = 0;
int sdl_displayheight = 0;
// current size of the window, in pixels
int sdl_windowwidth = 0;
int sdl_windowheight = 0;
// each pixel is shown as this many pixels
int sdl_pixelscale = 0;
extern DLword *EmCursorBitMap68K;
extern char foregroundColorName[64];
extern char backgroundColorName[64];

/*
 * sdl_MapColorName approximates the X11 color parsing,
 * taking either a #RRGGBB hex value, or a name that is mapped
 * through the X11 color names table, returning an SDL pixel
 * according to the given pixel format
 */
static Uint32 sdl_MapColorName(const SDL_PixelFormat * format, char *name) {
  /* check for #RRBBGG format */
  if (name[0]=='#' && strlen(name) == 7 && strspn(&name[1], "0123456789abcdefABCDEF") == 6) {
    unsigned long pixval = strtoul(&name[1], NULL, 16);
    return SDL_MapRGB(format, (pixval >> 16) & 0xFF, (pixval >> 8) & 0xFF, pixval & 0xFF);
  }
  /* then try for a named color */
  for (int i = 0; colornames[i].name; i++) {
    if (0 == strcasecmp(name, colornames[i].name)) {
      return SDL_MapRGB(format, colornames[i].red, colornames[i].green, colornames[i].blue);
    }
  }
  /* fail */
  return(0);
}

static int min(int a, int b) {
  if (a < b) return a;
  return b;
}

static int display_update_needed = 0;

static int min_x = INT_MAX;
static int min_y = INT_MAX;
static int max_x = 0;
static int max_y = 0;
void sdl_notify_damage(int x, int y, int w, int h) {
  if (x < min_x) min_x = x;
  if (y < min_y) min_y = y;
  if (x + w > max_x) max_x = min(x + w, sdl_displaywidth - 1);
  if (y + h > max_y) max_y = min(y + h, sdl_displayheight - 1);
  display_update_needed = 1;
}

/* a simple linked list to remember generated cursors
 * because cursors don't have any identifying information
 * except for the actual bitmap in Lisp, just cache that.
 * 16 DLwords, to give a 16x16 bitmap cursor.
 */
struct CachedCursor {
  struct CachedCursor *next;
  DLword EmCursorBitMap[CURSORHEIGHT];
  SDL_Cursor *cursor;
} *sdl_cursorlist = NULL;

/*
 * given a 16-bit value and a repeat count modify an array
 * of bytes to contain the same bit pattern with each bit
 * repeated "reps" times consecutively in the output
 */
static void replicate_bits(int bits, int reps, Uint8 *out) {
  int dbyte = 0;
  int dbit = 7;
  for (int ibit = 15; ibit >= 0; --ibit) {
    for (int r = 0; r < reps; r++) {
      if (bits & (1 << ibit))
        out[dbyte] |= 1 << dbit;
      if (--dbit < 0) {
        dbyte++;
        dbit = 7;
      }
    }
  }
}

static int cursor_equal_p(const DLword *a, const DLword *b) {
  for (int i = 0; i < CURSORHEIGHT; i++)
    if (a[i] != b[i]) return FALSE;
  return TRUE;
}

/*
 * Try to find cursor CURSOR on the sdl_cursorlist, if it isn't there, add it.
 * Return an SDL_Cursor that can be used directly.
 */
static SDL_Cursor *sdl_getOrAllocateCursor(DLword cursor[16], int hot_x, int hot_y) {
  hot_x = 0;
  hot_y = 0;
  /* try to find the cursor by checking the full bitmap */
  struct CachedCursor *pclp = NULL;
  struct CachedCursor *clp = sdl_cursorlist;
  SDL_Cursor *c;
  while (clp != NULL) {
    if (cursor_equal_p(clp->EmCursorBitMap, cursor) == TRUE) {
      /* if it's in the first two elements of the list, leave the order alone.
       * There is a high probability of flipping back and forth between two
       */
      if (clp == sdl_cursorlist || pclp == sdl_cursorlist) {
        return clp->cursor;
      }
      /* otherwise unlink the found item and reinsert at the front */
      pclp->next = clp->next;
      clp->next = sdl_cursorlist;
      sdl_cursorlist = clp;
      return clp->cursor;
    }
    pclp = clp;
    clp = clp->next;
  }
  /* It isn't there, so build a new one */
  clp = (struct CachedCursor *)malloc(sizeof(struct CachedCursor));
  memcpy(clp->EmCursorBitMap, cursor, sizeof(clp->EmCursorBitMap));
  /* no scaling is an easy case, scale > 1 is harder */
  if (sdl_pixelscale == 1) {
    Uint8 sdl_cursor_data[32];
    for (int i = 0; i < 32; i++) sdl_cursor_data[i] = GETBYTE(((Uint8 *)cursor) + i);
    c = SDL_CreateCursor(sdl_cursor_data, sdl_cursor_data, 16, 16, hot_x, hot_y);
  } else {
    Uint8 *sdl_cursor_data = (Uint8*)calloc(sdl_pixelscale * sdl_pixelscale, 32);
    /* fill in the cursor data expanded */
    for (int i = 0; i < 32; i += 2) {
      int v = GETBYTE(((Uint8 *)cursor) + i) << 8 | GETBYTE(((Uint8 *)cursor) + i + 1);
      int db = i * sdl_pixelscale * sdl_pixelscale;
      /* spread the bits out for the first copy of the row */
      replicate_bits(v, sdl_pixelscale, &sdl_cursor_data[db]);
      /* and then copy the replicated bits for the copies of the row */
      for (int j = 1; j < sdl_pixelscale; j++) {
        memcpy(&sdl_cursor_data[db + (j * 2 * sdl_pixelscale)], &sdl_cursor_data[db], 2 * sdl_pixelscale);
      }
    }
    c = SDL_CreateCursor(sdl_cursor_data, sdl_cursor_data, 16 * sdl_pixelscale, 16 * sdl_pixelscale, hot_x, hot_y);
  }
  if (c == NULL) printf("ERROR creating cursor: %s\n", SDL_GetError());
  clp->cursor = c;
  clp->next = sdl_cursorlist;
  sdl_cursorlist = clp;
  return clp->cursor;
}

/*
 * Read a cursor bitmap from lisp. Try to find a cached cursor, then use that.
 * Use HOT_X and HOT_Y as the cursor hotspot.
 * XXX: needs to deal with sdl_pixelscale > 1, and where is the hotspot?
 */
void sdl_setCursor(int hot_x, int hot_y) {
  SDL_Cursor *c = sdl_getOrAllocateCursor(EmCursorBitMap68K, hot_x, hot_y);
  SDL_SetCursor(c);
}
void sdl_bitblt_to_texture(int _x, int _y, int _w, int _h) {
  DLword *src = DisplayRegion68k;
  void *dst;
  int dstpitchbytes;
  int dstpitchpixels;
  const int bitsperword = 8 * sizeof(DLword);
  int sourcepitchwords = sdl_displaywidth / bitsperword;
  int xstart = _x / bitsperword;
  int xlimit = (_x + _w + bitsperword - 1) / bitsperword;
  int ystart = _y * sourcepitchwords;
  int ylimit = (_y + _h) * sourcepitchwords;
  SDL_Rect dstrect;
  // Avoid dealing with partial words in the update by stretching the source rectangle
  // left and right to cover complete units and lock the corresponding
  // region in the texture
  dstrect.x = xstart * bitsperword;
  dstrect.w = (xlimit * bitsperword) - dstrect.x;
  dstrect.y = _y;
  dstrect.h = _h;
  SDL_LockTexture(sdl_texture, &dstrect, &dst, &dstpitchbytes);
  dstpitchpixels = dstpitchbytes / sdl_bytesperpixel;
  int dy = 0;
  // for each line in the source image
  for (int sy = ystart; sy < ylimit; sy += sourcepitchwords, dy += dstpitchpixels) {
    // for each word in the line
    int dx = 0;
    for (int sx = xstart; sx < xlimit; sx++, dx += bitsperword) {
      int srcw = GETBASEWORD(src, sy + sx);
      // for each bit in the word
      for (int b = 0; b < bitsperword; b++) {
        ((Uint32 *)dst)[dy + dx + b] = (srcw & bitmask[b]) ? sdl_foreground : sdl_background;
      }
    }
  }
  SDL_UnlockTexture(sdl_texture);
}

static int map_key(SDL_Scancode k) {
  for (int i = 0; sdlScanCodeToLisp[i].scancode != 0; i++ ) {
    if (sdlScanCodeToLisp[i].scancode == k)
        return sdlScanCodeToLisp[i].lispcode;
  }
  return -1;
}

static void sendLispCode(int code, int up)
{
    kb_trans(code, up);
    display_notify_lisp();
}

static void handle_keydown(SDL_Scancode k) {
  int lk = map_key(k);
  if (lk == -1) {
    if( (lctrl || rctrl) && k >= SDL_SCANCODE_A && k <= SDL_SCANCODE_Z )
        sendLispCode(shortcutScanCodeToLisp[k], FALSE);
  } else {
    // printf("dn %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
    if( k == SDL_SCANCODE_LSHIFT )
        lshift = 1;
    else if( k == SDL_SCANCODE_RSHIFT )
        rshift = 1;
    else if( k == SDL_SCANCODE_LCTRL )
        lctrl = 1;
    else if( k == SDL_SCANCODE_RCTRL )
        rctrl = 1;
    sendLispCode(lk, FALSE);
  }
}
static void handle_keyup(SDL_Scancode k) {
  int lk = map_key(k);
  if (lk == -1) {
      if( (lctrl || rctrl) && k >= SDL_SCANCODE_A && k <= SDL_SCANCODE_Z )
          sendLispCode(shortcutScanCodeToLisp[k], TRUE);
  } else {
    // printf("up %s -> lisp keycode %d (0x%x)\n", SDL_GetKeyName(k), lk, mod);
      if( k == SDL_SCANCODE_LSHIFT )
          lshift = 0;
      else if( k == SDL_SCANCODE_RSHIFT )
          rshift = 0;
      else if( k == SDL_SCANCODE_LCTRL )
          lctrl = 0;
      else if( k == SDL_SCANCODE_RCTRL )
          rctrl = 0;
      sendLispCode(lk, TRUE);
  }
}

static void sdl_update_viewport(int width, int height) {
  /* XXX: needs work */
  int w = width / 32 * 32;
  if (w > sdl_displaywidth * sdl_pixelscale) w = sdl_displaywidth * sdl_pixelscale;
  int h = height / 32 * 32;
  if (h > sdl_displayheight * sdl_pixelscale) h = sdl_displayheight * sdl_pixelscale;
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = w;
  r.h = h;
  SDL_RenderSetViewport(sdl_renderer, &r);
  printf("new viewport: %d / %d\n", w, h);
}

void sdl_set_invert(int flag) {
  if (flag) {
    sdl_foreground = sdl_background_color;
    sdl_background = sdl_foreground_color;
  } else {
    sdl_foreground = sdl_foreground_color;
    sdl_background = sdl_background_color;
  }
  sdl_notify_damage(0, 0, sdl_displaywidth, sdl_displayheight);
}
void sdl_setMousePosition(int x, int y) {
  SDL_WarpMouseInWindow(sdl_window, x * sdl_pixelscale, y * sdl_pixelscale);
}
void sdl_update_display() {
  sdl_bitblt_to_texture(min_x, min_y, max_x - min_x, max_y - min_y);
  SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
  SDL_RenderPresent(sdl_renderer);
}

void process_SDLevents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        printf("quitting\n");
        exit(0);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            /* XXX: what about integer multiple of 32 requirements here? */
            sdl_windowwidth = event.window.data1;
            sdl_windowheight = event.window.data2;
            sdl_update_viewport(sdl_windowwidth, sdl_windowheight);
            break;
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            sdl_window_focusp = 1;
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            sdl_window_focusp = 0;
            break;
        default:
          break;
        }
        break;
      case SDL_TEXTINPUT: {
        const char ch = (uint8_t)event.text.text[0];
        if( ch == ' ' )
            break; // handled by scan code
        if( lctrl || rctrl )
            break; // just pro forma, since SDL_TEXTINPUT is apparently not sent anyway if CTRL pressed
        int code = -1;
        int shift = 0;
        for(int i = 0; charToLisp[i].ch; i++ )
        {
            if( charToLisp[i].ch == ch )
            {
                code = charToLisp[i].code;
                shift = charToLisp[i].shift;
                break;
            }
        }
        if( code != -1 )
        {
            const uint8_t old_lshift = lshift;
            const uint8_t old_rshift = rshift;
            if( shift )
            {
                if( !old_lshift && !old_rshift )
                    handle_keydown(SDL_SCANCODE_LSHIFT);
            }else
            {
                if( old_lshift )
                    handle_keyup(SDL_SCANCODE_LSHIFT);
                if( old_rshift )
                    handle_keyup(SDL_SCANCODE_RSHIFT);
            }
            sendLispCode(code,FALSE);
            sendLispCode(code,TRUE);
            if( shift )
            {
                if( !old_lshift && !old_rshift )
                    handle_keyup(SDL_SCANCODE_LSHIFT);
            }else
            {
                if( old_lshift )
                    handle_keydown(SDL_SCANCODE_LSHIFT);
                if( old_rshift )
                    handle_keydown(SDL_SCANCODE_RSHIFT);
            }
        }else
        {
            printf("No mapping for key '%s'\n", SDL_GetKeyName(lastKey));
            fflush(stdout);
        }
        break;
        }

      case SDL_KEYDOWN:
        if (event.key.repeat) {
          /* Lisp needs to see the UP transition before the DOWN transition */
          handle_keyup(event.key.keysym.scancode);
        }
        lastKey = event.key.keysym.sym;
        handle_keydown(event.key.keysym.scancode);
        break;
      case SDL_KEYUP:
        handle_keyup(event.key.keysym.scancode);
        break;
      case SDL_MOUSEMOTION: {
        int x, y;
        if (!sdl_window_focusp) break;
        SDL_GetMouseState(&x, &y);
        x /= sdl_pixelscale;
        y /= sdl_pixelscale;
        display_notify_mouse_pos(x,y);
        display_notify_lisp();
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        switch (event.button.button) {
          case SDL_BUTTON_LEFT: display_left_mouse_button(TRUE); break;
          case SDL_BUTTON_MIDDLE: display_mid_mouse_button(TRUE); break;
          case SDL_BUTTON_RIGHT: display_right_mouse_button(TRUE); break;
        }
        display_notify_lisp();
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        switch (event.button.button) {
          case SDL_BUTTON_LEFT: display_left_mouse_button(FALSE); break;
          case SDL_BUTTON_MIDDLE: display_mid_mouse_button(FALSE); break;
          case SDL_BUTTON_RIGHT: display_right_mouse_button(FALSE); break;
        }
        display_notify_lisp();
        break;
      }
    default: /* printf("other event type: %d\n", event.type); */ break;
    }
  }
  if (display_update_needed) {
    sdl_update_display();
    display_update_needed = 0;
    min_x = min_y = INT_MAX;
    max_x = max_y = 0;
  }
}

int init_SDL(const char *windowtitle, int w, int h, int s) {
  sdl_pixelscale = s;
  // width must be multiple of 32
  w = (w + 31) / 32 * 32;
  sdl_displaywidth = w;
  sdl_displayheight = h;
  sdl_windowwidth = w * s;
  sdl_windowheight = h * s;
  int width = sdl_displaywidth;
  int height = sdl_displayheight;
  printf("requested width: %d, height: %d\n", width, height);
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not be initialized. SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  printf("initialised\n");
  sdl_window = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                sdl_windowwidth, sdl_windowheight, 0);
  printf("Window created\n");
  if (sdl_window == NULL) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    return 2;
  }
  printf("Creating renderer...\n");
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (NULL == sdl_renderer) {
    printf("SDL Error: %s\n", SDL_GetError());
    return 3;
  }
  SDL_GetRendererInfo(sdl_renderer, &sdl_rendererinfo);
  SDL_SetRenderDrawColor(sdl_renderer, 127, 127, 127, 255);
  SDL_RenderClear(sdl_renderer);
  SDL_RenderPresent(sdl_renderer);
  SDL_RenderSetScale(sdl_renderer, 1.0, 1.0);
  sdl_pixelformat = SDL_AllocFormat(sdl_rendererinfo.texture_formats[0]);
  printf("Creating texture...\n");
  sdl_texture = SDL_CreateTexture(sdl_renderer, sdl_pixelformat->format,
                                  SDL_TEXTUREACCESS_STREAMING, width, height);
  char tmp[10] = "black";
  sdl_foreground_color = sdl_MapColorName(sdl_pixelformat,
                                          foregroundColorName[0] ? foregroundColorName : tmp);
  strcpy(tmp,"white");
  sdl_background_color = sdl_MapColorName(sdl_pixelformat,
                                          backgroundColorName[0] ? backgroundColorName : tmp);
  sdl_foreground = sdl_foreground_color;
  sdl_background = sdl_background_color;
  sdl_bytesperpixel = sdl_pixelformat->BytesPerPixel;
  printf("SDL initialised\n");
  fflush(stdout);
  return 0;
}
