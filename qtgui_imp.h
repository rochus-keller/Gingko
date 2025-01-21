#ifndef QTGUI_IMP
#define QTGUI_IMP

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

#include <QResizeEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QWindow>
#include <QBitmap>
#include <QtDebug>
#include <QThread>
#include <QBackingStore>

class GingkoWindow : public QWindow
{
    Q_OBJECT
public:
    explicit GingkoWindow(QWindow *parent = 0);
    ~GingkoWindow();

    virtual void render(QPainter *painter);

public slots:
    void onSetMousePosition(int x, int y);
    void onSetInvert(bool flag);
    void onDamage(int x, int y, int w, int h);
    void onSetCursor(QCursor cur);

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
    void hideEvent(QHideEvent *);
    void renderLater();
    void renderNow();
    void notifyMouseStat( QPoint pos, bool left, bool mid, bool right );
    void notifyKeyStat(int code, const QByteArray& text, bool down);

private:
    QBackingStore *m_backingStore;
    QImage img;
    QRgb foreground, background;
    QList<QRect> patches;
    bool m_update_pending;
    bool inverted;
};

class LispRunner : public QThread
{
    Q_OBJECT
    void (*proc)();
public:
    LispRunner(void (*proc)()):proc(proc) {}
    void run();

signals:
    void setMousePosition(int x, int y);
    void setInvert(bool flag);
    void damage(int x, int y, int w, int h);
    void setCursor(QCursor);

};


#endif // QTGUI_IMP

