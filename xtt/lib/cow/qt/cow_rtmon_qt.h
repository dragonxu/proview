/*
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2017 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of Proview (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 */

#ifndef cow_rtmon_qt_h
#define cow_rtmon_qt_h

/* cow_rtmon_qt.h -- Status Monitor */

#ifndef cow_rtmon_h
# include "cow_rtmon.h"
#endif

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class RtMonQtWidget;

class RtMonQt : public RtMon {
public:
  RtMonQt(void *rtmon_parent_ctx, QWidget *rtmon_parent_wid,
          const char *rtmon_name, const char *rtmon_display,
          pwr_tStatus *status);
  ~RtMonQt();

  QWidget *nodelistnav_widget;
  QPushButton *bbox_label;
  QWidget *bbox_start;
  QWidget *bbox_restart;
  QWidget *bbox_stop;
  QLabel *bbox_image;
  QLabel *bbox_image_gray;
  QAction *tools_xtt;
  QAction *tools_op;
  QAction *file_xtt;
  QAction *file_op;
  QTimer *timerid;
  pwr_tStatus old_status;

  void pop();
  void set_clock_cursor();
  void reset_cursor();
  void create_input_dialog();
  void open_input_dialog(char *text, char *title, char *init_text,
                         void (*ok_cb)(RtMon *, char *));

private:
  RtMonQtWidget *toplevel;
};

class RtMonQtWidget : public QWidget {
  Q_OBJECT

public:
  RtMonQtWidget(RtMonQt *parent_ctx, QWidget *parent)
      : QWidget(parent, Qt::Window), rtmon(parent_ctx) {}

protected:
  void focusInEvent(QFocusEvent *event);
  void closeEvent(QCloseEvent *event);

public slots:
  void activate_zoom_in();
  void activate_zoom_out();
  void activate_zoom_reset();
  void activate_xtt();
  void activate_op();
  void activate_help();

  void rtmon_scan();

private:
  RtMonQt *rtmon;
};

#endif