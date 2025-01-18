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

#include "qtgui.h"
#include <QBackingStore>
#include <QResizeEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QWindow>
#include <stdint.h>
#include <QBitmap>

typedef unsigned short DLword;
extern DLword *DisplayRegion68k;
extern DLword *EmCursorBitMap68K;
extern DLword *EmCursorX68K, *EmCursorY68K;
extern DLword *EmMouseX68K, *EmMouseY68K;
extern int displaywidth, displayheight;
static const int pixPerWord = 16;
extern void display_notify_lisp();
extern void display_notify_mouse_pos(int x, int y);
extern void display_set_keyboard_event_flag();
extern void display_left_mouse_button(bool on);
extern void display_mid_mouse_button(bool on);
extern void display_right_mouse_button(bool on);
#define GETBASEWORD(base, offset) GETWORDBASEWORD((base),(offset))
#define GETWORDBASEWORD(base, offset) (* (DLword *) (2^(uintptr_t)((base)+(offset))))
#define GETBYTE(base) (* (unsigned char *) (3^(uintptr_t)(base)))

static inline void PUTBASEBIT68K(DLword* base68k, int offset, bool bitvalue)
{
  do {
    uintptr_t real68kbase;
    real68kbase = 2 ^ ((uintptr_t)(base68k));
    if (bitvalue)
      (*(DLword *)(2 ^ (uintptr_t)((DLword *)(real68kbase) + (((uint16_t)(offset)) >> 4)))) |=
          1 << (15 - ((uint16_t)(offset)) % pixPerWord);
    else
      (*(DLword *)(2 ^ (uintptr_t)((DLword *)(real68kbase) + (((uint16_t)(offset)) >> 4)))) &=
          ~(1 << (15 - ((uint16_t)(offset)) % pixPerWord));
  } while (0);
}

class GingkoWindow : public QWindow
{
public:
    explicit GingkoWindow(QWindow *parent = 0);
    ~GingkoWindow();

    virtual void render(QPainter *painter);
    void damage(int x, int y, int w, int h);
    void update();

protected:
    bool event(QEvent *event);

    void resizeEvent(QResizeEvent *event);
    void exposeEvent(QExposeEvent *event);
    void timerEvent(QTimerEvent*);
    void keyPressEvent(QKeyEvent * ev);
    void keyReleaseEvent(QKeyEvent * ev);
    void mouseMoveEvent(QMouseEvent * ev);
    void mousePressEvent(QMouseEvent * ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void renderLater();
    void renderNow();

private:
    QBackingStore *m_backingStore;
    QImage img;
    QRgb foreground, background;
    QList<QRect> patches;
    bool m_update_pending;
};

static QGuiApplication* app = 0;
static GingkoWindow* wnd = 0;
static int argc = 1;
static char * argv = "Gingko";
static const DLword bitmask[pixPerWord] = {1 << 15, 1 << 14, 1 << 13, 1 << 12, 1 << 11, 1 << 10,
                                   1 << 9,  1 << 8,  1 << 7,  1 << 6,  1 << 5,  1 << 4,
                                   1 << 3,  1 << 2,  1 << 1,  1 << 0};

GingkoWindow::GingkoWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false)
{
    create();
    m_backingStore = new QBackingStore(this);
    foreground = qRgb(0,0,0);
    background = qRgb(255,255,255);
    startTimer(30);
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
    //img = QImage( resizeEvent->size(), QImage::Format_Mono );
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

}

void GingkoWindow::keyReleaseEvent(QKeyEvent* ev)
{

}

void GingkoWindow::mouseMoveEvent(QMouseEvent* ev)
{
    display_notify_mouse_pos(ev->x(),ev->y());
    display_notify_lisp();
    display_set_keyboard_event_flag();
}

void GingkoWindow::mousePressEvent(QMouseEvent* ev)
{
    display_notify_mouse_pos(ev->x(),ev->y());
    switch(ev->button())
    {
    case Qt::LeftButton:
        display_left_mouse_button(true);
        break;
    case Qt::MidButton:
        display_mid_mouse_button(true);
        break;
    case Qt::RightButton:
        display_right_mouse_button(true);
        break;
    }
    display_notify_lisp();
    display_set_keyboard_event_flag();
}

void GingkoWindow::mouseReleaseEvent(QMouseEvent* ev)
{
    display_notify_mouse_pos(ev->x(),ev->y());
    switch(ev->button())
    {
    case Qt::LeftButton:
        display_left_mouse_button(false);
        break;
    case Qt::MidButton:
        display_mid_mouse_button(false);
        break;
    case Qt::RightButton:
        display_right_mouse_button(false);
        break;
    }
    display_notify_lisp();
    display_set_keyboard_event_flag();
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

void GingkoWindow::damage(int x, int y, int w, int h)
{
    patches.append(QRect(x,y,w,h));
}

void GingkoWindow::update()
{
    if( !patches.isEmpty() )
        renderLater();
}

void GingkoWindow::render(QPainter *painter)
{
    const int w = displaywidth;
    const int h = displayheight;
    const int ww = w / pixPerWord;
    const quint16* raster = (quint16*)DisplayRegion68k;
    for( int line = 0; line < h; line++) // for each screen line
    {
        const int line_start = line * ww;
        for( int col = 0; col < ww; col++ ) // for each word in a line
        {
            const int x = col * pixPerWord;
            const quint16 pixels = GETBASEWORD(raster,line_start + col);
            for( int b = 0; b < pixPerWord; b++ ) // for each pix in a word
                img.setPixel(x + b,line, (pixels & bitmask[b]) ? foreground : background );
        }
    }

    painter->drawImage(0,0,img);
}

void qt_notify_damage(int x, int y, int w, int h)
{
    wnd->damage(x,y,w,h);
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
    wnd->setCursor( QCursor( pix, pix, hot_x, hot_y ) );
}

void qt_set_invert(int flag)
{

}

void qt_setMousePosition(int x, int y)
{

}

void qt_process_events()
{
    //wnd->update();
    QGuiApplication::processEvents(QEventLoop::WaitForMoreEvents | QEventLoop::AllEvents);
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
    wnd->show();
    return 0;
}
