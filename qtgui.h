#ifndef QTGUI_H
#define QTGUI_H

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

void qt_notify_damage(int x, int y, int w, int h); // w/h==-1 ... all
void qt_setCursor(int hot_x, int hot_y);
void qt_set_invert(int flag);
void qt_setMousePosition(int x, int y);
void qt_process_events();
int qt_init(const char *windowtitle, int w, int h, int s);


#endif // QTGUI_H
