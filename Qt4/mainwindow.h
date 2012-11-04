/*
  mainwindow.h
*/

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QtSql>
#include <iostream>

#include "qt2chdl.h"
#include "dlthread.h"

#define APP_NAME "Qt2chdl"
#define APP_ICON ":/qrc/icon_qt2chdl"
#define APP_CONF APP_NAME".qtd"
#define APP_DATA APP_NAME".sl3"
#define APP_DB "QSQLITE"
#define SAMPLE_URL "http://kohada.2ch.net/ana/dat/1351546378.dat"

class MainWindow : public QMainWindow{
  Q_OBJECT

public:
  MainWindow(QQueue<QString> &q, QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~MainWindow();

protected:
  void saveLayout();
  void loadLayout();
  void closeEvent(QCloseEvent *ce);

private:
  void createActions();
  void createMenus();
  void createToolBar();
  void createStatusBar();
  void createDockWindows();
  void createTrayIcon();

signals:
  void toBeAbort();
  void quit();
  void stop();

public slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void download();
  void fin();
  void proc();
  void cleanupcode();

private:
  DlThread *dt;
  QThread *th;
  QQueue<QString> &quelst;
  QSqlDatabase db;
  QString home;

  QAction *mDownloadAction;
  QMenu *mFileMenu;
  QMenu *mViewMenu;
  QToolBar *mFileToolBar;
  QLineEdit *mUrl;
  QTextEdit *mText;
  QDirModel *mModel;
  QTreeView *mTree;

  QAction *mMinimizeAction;
  QAction *mMaximizeAction;
  QAction *mRestoreAction;
  QAction *mQuitAction;
  QSystemTrayIcon *mTrayIcon;
  QMenu *mTrayIconMenu;
};

#endif // __MAINWINDOW_H__
