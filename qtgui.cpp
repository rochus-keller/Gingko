/*
* Copyright 2025 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Gingko project.
*
* The following is the license that applies to this copy of the
* file. For a license to use the file under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include "qtgui_imp.h"
#include <QMutex>
#include <stdint.h>

extern "C" {
#include "qtgui.h"
typedef unsigned short DLword;
extern DLword *DisplayRegion68k;
extern DLword *EmCursorBitMap68K;
extern DLword *EmCursorX68K, *EmCursorY68K;
extern DLword *EmMouseX68K, *EmMouseY68K;
extern int displaywidth, displayheight;
static const int pixPerWord = 16;
static const int FrameRate = 30; // milliseconds
extern void display_notify_lisp();
extern void display_notify_mouse_pos(int x, int y);
extern void display_mouse_state(bool left, bool mid, bool right);
extern void kb_trans(uint16_t keycode, uint16_t upflg);
#define GETBASEWORD(base, offset) GETWORDBASEWORD((base),(offset))
#define GETWORDBASEWORD(base, offset) (* (DLword *) (2^(uintptr_t)((base)+(offset))))
}

static bool shift = false, ctrl = false;

static struct { int lispcode; int scancode; } sdlScanCodeToLisp[] =
{
    // these are all keys usually not used for a printable character
    {13, Qt::Key_Delete},
    {14, Qt::Key_ScrollLock},
    {15, Qt::Key_Backspace},
    {31, Qt::Key_Alt},
    {33, Qt::Key_Escape},
    {34, Qt::Key_Tab},
    {36, Qt::Key_Control},
    {41, Qt::Key_Shift},
    {44, Qt::Key_Return},
    {76, Qt::Key_Enter},
    {46, Qt::Key_F20},
    //{47, SDL_SCANCODE_RCTRL},
    {56, Qt::Key_CapsLock},
    {57, Qt::Key_Space},
    //{60, SDL_SCANCODE_RSHIFT},
    {61, Qt::Key_Pause},
    {62, Qt::Key_F18},
    {62, Qt::Key_Home},
    {63, Qt::Key_PageUp},
    {66, Qt::Key_F7},
    {67, Qt::Key_F4},
    {68, Qt::Key_F5},
    {69, Qt::Key_Down},
    //{73, SDL_SCANCODE_NUMLOCKCLEAR},
    {80, Qt::Key_F9},
    {82, Qt::Key_Up},
    {84, Qt::Key_Left},
    {86, Qt::Key_Meta},
    {87, Qt::Key_Right},
    //{88, SDL_SCANCODE_RGUI},
    {89, Qt::Key_Insert},
    {90, Qt::Key_End},
    {92, Qt::Key_Print},
    // {93, SDL_SCANCODE_RALT}, // this interferes with some Medley feature, so we don't send it
    {99, Qt::Key_F2},
    {100, Qt::Key_F3},
    {101, Qt::Key_F6},
    {104, Qt::Key_F8},
    {106, Qt::Key_F10},
    {107, Qt::Key_F11},
    {108, Qt::Key_F12},
    {109, Qt::Key_F13},
    {110, Qt::Key_F22},
    {111, Qt::Key_F17},
    {0,0}
};

static int shortcutScanCodeToLisp[] =
{
    21, // Qt::Key_A
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
    40, // Qt::Key_Z
};

static struct CharToLisp { char ch; uint8_t code; bool shift; } charToLisp[] =
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

static CharToLisp* map_char(char ch) {
    for(int i = 0; charToLisp[i].ch; i++ )
    {
        if( charToLisp[i].ch == ch )
            return &charToLisp[i];
    }
    return 0;
}

static int map_key(int k) {
  for (int i = 0; sdlScanCodeToLisp[i].scancode != 0; i++ ) {
    if (sdlScanCodeToLisp[i].scancode == k)
        return sdlScanCodeToLisp[i].lispcode;
  }
  return -1;
}

static void sendLispCode(int code, bool up)
{
    kb_trans(code, up);
    display_notify_lisp();
}

static void handle_keydown(int k) {
  int lk = map_key(k);
  if (lk == -1) {
    if( ctrl && k >= Qt::Key_A && k <= Qt::Key_Z )
        sendLispCode(shortcutScanCodeToLisp[k-Qt::Key_A], false);
  } else {
    if( k == Qt::Key_Shift )
        shift = true;
    else if( k == Qt::Key_Control )
        ctrl = true;
    sendLispCode(lk, false);
  }
}
static void handle_keyup(int k) {
  int lk = map_key(k);
  if (lk == -1) {
      if( ctrl && k >= Qt::Key_A && k <= Qt::Key_Z )
          sendLispCode(shortcutScanCodeToLisp[k-Qt::Key_A], true);
  } else {
      if( k == Qt::Key_Shift )
          shift = false;
      else if( k == Qt::Key_Control )
          ctrl = false;
      sendLispCode(lk, true);
  }
}

static GingkoWindow* wnd = 0;
static QGuiApplication* app = 0;
static LispRunner* runner = 0;
static int argc = 1;
static char * argv = "Gingko";
static const DLword bitmask[pixPerWord] = {1 << 15, 1 << 14, 1 << 13, 1 << 12, 1 << 11, 1 << 10,
                                   1 << 9,  1 << 8,  1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                   1 << 3,  1 << 2,  1 << 1,  1 << 0};

struct MouseEvent
{
    QPoint pos;
    bool left, mid, right;
    MouseEvent(const QPoint& pos, bool left, bool mid, bool right):
        pos(pos),left(left),mid(mid),right(right) {}
    bool equals(const MouseEvent& rhs) const
    {
        return rhs.pos == pos && rhs.left == left && rhs.mid == mid && rhs.right == right;
    }
};
struct KeyEvent
{
    QByteArray text;
    int code;
    bool down;
    KeyEvent(int code, const QByteArray& text, bool down):
        code(code),text(text),down(down) {}
};
static QMutex eventLock;
static QList<MouseEvent> mouseEvents;
static QList<KeyEvent> keyEvents;

GingkoWindow::GingkoWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false), inverted(false)
{
    create();
    m_backingStore = new QBackingStore(this);
    foreground = qRgb(0,0,0);
    background = qRgb(255,255,255);
    startTimer(FrameRate);
}

GingkoWindow::~GingkoWindow()
{
    delete m_backingStore;
}

bool GingkoWindow::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        m_update_pending = false;
        renderNow();
        return true;
    }
    return QWindow::event(event);
}

void GingkoWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void GingkoWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    m_backingStore->resize(resizeEvent->size());
    img = QImage( resizeEvent->size(), QImage::Format_RGB888 );
    if (isExposed())
        renderNow();
}

void GingkoWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed()) {
        renderNow();
    }
}

void GingkoWindow::timerEvent(QTimerEvent*)
{
    if (isExposed()) {
        renderNow();
    }
}

void GingkoWindow::keyPressEvent(QKeyEvent* ev)
{
    const QByteArray text = ev->text().simplified().toUtf8();
    notifyKeyStat(ev->key(), text, true);
}

void GingkoWindow::keyReleaseEvent(QKeyEvent* ev)
{
    const QByteArray text = ev->text().simplified().toUtf8();
    notifyKeyStat(ev->key(), text, false);
}

void GingkoWindow::mouseMoveEvent(QMouseEvent* ev)
{
    emit notifyMouseStat(ev->pos(), ev->buttons() & Qt::LeftButton, ev->buttons() &
                         Qt::MidButton, ev->buttons() & Qt::RightButton);
}

void GingkoWindow::mousePressEvent(QMouseEvent* ev)
{
    emit notifyMouseStat(ev->pos(), ev->buttons() & Qt::LeftButton, ev->buttons() &
                         Qt::MidButton, ev->buttons() & Qt::RightButton);
}

void GingkoWindow::mouseReleaseEvent(QMouseEvent* ev)
{
    emit notifyMouseStat(ev->pos(), ev->buttons() & Qt::LeftButton, ev->buttons() &
                         Qt::MidButton, ev->buttons() & Qt::RightButton);
}

void GingkoWindow::hideEvent(QHideEvent*)
{
    printf("quitting\n");
    exit(0);
}

void GingkoWindow::renderNow()
{
    if (!isExposed())
        return;

    QRect rect(0, 0, width(), height());
    m_backingStore->beginPaint(rect);

    QPaintDevice *device = m_backingStore->paintDevice();
    QPainter painter(device);

    // TODO: only updated the damaged region if too slow
    patches.clear();
    render(&painter);

    m_backingStore->endPaint();
    m_backingStore->flush(rect);
}

void GingkoWindow::notifyMouseStat(QPoint pos, bool left, bool mid, bool right)
{
    QMutexLocker lock(&eventLock);
    MouseEvent e(pos, left, mid, right);
    if( !mouseEvents.isEmpty() && mouseEvents.front().equals(e) )
        return;
    mouseEvents.push_front(e);
}

void GingkoWindow::notifyKeyStat(int code, const QByteArray& text, bool down)
{
    QMutexLocker lock(&eventLock);
    KeyEvent e(code, text, down);
    keyEvents.push_front(e);
}

void GingkoWindow::onSetMousePosition(int x, int y)
{
    const QPoint p0 = wnd->mapToGlobal(QPoint(x,y));
    QCursor::setPos(p0);
}

void GingkoWindow::onSetInvert(bool on)
{
    inverted = on;
    renderNow();
}

void GingkoWindow::onDamage(int x, int y, int w, int h)
{
    patches.append(QRect(x,y,w,h));
}

void GingkoWindow::onSetCursor(QCursor cur)
{
    setCursor(cur);
}

void GingkoWindow::render(QPainter *painter)
{
    const int w = displaywidth;
    const int h = displayheight;
    const int ww = w / pixPerWord;
    const quint16* raster = (quint16*)DisplayRegion68k;
    QRgb fg = inverted ? background : foreground, bg = inverted ? foreground : background;
    for( int line = 0; line < h; line++) // for each screen line
    {
        const int line_start = line * ww;
        for( int col = 0; col < ww; col++ ) // for each word in a line
        {
            const int x = col * pixPerWord;
            const quint16 pixels = GETBASEWORD(raster,line_start + col);
            for( int b = 0; b < pixPerWord; b++ ) // for each pix in a word
                img.setPixel(x + b,line, (pixels & bitmask[b]) ? fg : bg );
        }
    }

    painter->drawImage(0,0,img);
}

void LispRunner::run()
{
    proc();
}

extern "C" {
void qt_notify_damage(int x, int y, int w, int h)
{
    runner->damage(x,y,w,h);
}

void qt_setCursor(int hot_x, int hot_y)
{
    QImage cursor( 16, 16, QImage::Format_Mono );
    const quint16* raster = (quint16*)EmCursorBitMap68K;
    for( int line = 0; line < 16; line++ )
    {
        const quint16 pixels = GETBASEWORD(raster,line);
        for( int b = 0; b < pixPerWord; b++ ) // for each pix in a word
            cursor.setPixel(b,line, !(pixels & bitmask[b]) );
    }
    QBitmap pix = QPixmap::fromImage( cursor );
    runner->setCursor( QCursor( pix, pix, hot_x, hot_y ) );
}

void qt_set_invert(int flag)
{
    runner->setInvert(flag);
}

void qt_setMousePosition(int x, int y)
{
    // TODO runner->setMousePosition(x,y);
}

void qt_process_events()
{
    QMutexLocker lock(&eventLock);
    while( !mouseEvents.isEmpty() )
    {
        MouseEvent e = mouseEvents.takeLast();
        display_notify_mouse_pos(e.pos.x(),e.pos.y());
        display_mouse_state(e.left, e.mid, e.right);
        display_notify_lisp();
    }
    while( !keyEvents.isEmpty() )
    {
        KeyEvent e = keyEvents.takeLast();
        CharToLisp* ctl = 0;
        if( e.down )
        {
            if( e.text.size() == 1 && (ctl = map_char(e.text[0]) ) )
            {
                const bool old_shift = shift;
                if( shift )
                {
                    if( !old_shift )
                        handle_keydown(Qt::Key_Shift);
                }else if( old_shift )
                    handle_keyup(Qt::Key_Shift);
                sendLispCode(ctl->code,false);
                sendLispCode(ctl->code,true);
                if( shift )
                {
                    if( !old_shift )
                        handle_keyup(Qt::Key_Shift);
                }else if( old_shift )
                    handle_keydown(Qt::Key_Shift);
                continue;
            }
#if 0
            if( ev->isAutoRepeat() )
            {
                /* Lisp needs to see the UP transition before the DOWN transition */
                handle_keyup(ev->key());
            }
#endif
            handle_keydown(e.code);
        }else
        {
            if( e.text.size() == 1 && map_char(e.text[0]) )
                continue;
            handle_keyup(e.code);
        }
    }
}

int qt_init(const char* windowtitle, int w, int h, int s)
{
    displaywidth = w;
    displayheight = h;
    app = new QGuiApplication(argc,&argv);
    wnd = new GingkoWindow();
    wnd->setTitle(QString::fromUtf8(windowtitle));
    const QSize size(w,h);
    wnd->setMinimumSize(size);
    wnd->setMaximumSize(size);
    wnd->setFlags(Qt::WindowCloseButtonHint);
    wnd->show();
    printf("Qt initialised\n");
    fflush(stdout);
    return 0;
}

void qt_lisp_thread(void (*proc)())
{
    Q_ASSERT(proc);
    runner = new LispRunner(proc);

    QObject::connect(runner,SIGNAL(damage(int,int,int,int)), wnd, SLOT(onDamage(int,int,int,int)));
    QObject::connect(runner,SIGNAL(setCursor(QCursor)),wnd,SLOT(onSetCursor(QCursor)));
    QObject::connect(runner,SIGNAL(setInvert(bool)),wnd, SLOT(onSetInvert(bool)));
    QObject::connect(runner,SIGNAL(setMousePosition(int,int)), wnd, SLOT(onSetMousePosition(int,int)));

    runner->start();

    app->exec();

    delete runner;
    delete wnd;
    delete app;
}
}
