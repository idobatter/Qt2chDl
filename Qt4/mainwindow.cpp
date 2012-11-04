/*
  mainwindow.cpp
*/

#include "mainwindow.h"

MainWindow::MainWindow(QQueue<QString> &q,
  QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags),
  dt(0), th(0), quelst(q)
{
  QIcon ico = QIcon(trUtf8(APP_ICON));
  setWindowIcon(ico);
  setWindowTitle(trUtf8(APP_NAME));
  resize(960, 480);

  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanupcode()));
  connect(this, SIGNAL(toBeAbort()), qApp, SLOT(quit()));

  home = QString(trUtf8("%1/.%2").arg(QDir::homePath()).arg(trUtf8(APP_NAME)));
  qDebug("home: %s", home.toUtf8().constData());
  if(!QDir().exists(home)){
    if(!QDir().mkdir(home)){
      QString msg(trUtf8("ホームディレクトリを作成出来ません\n%1").arg(home));
      QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
      emit toBeAbort();
    }
  }

  createActions();
  createMenus();
  createToolBar();
  createStatusBar();
  createDockWindows();
  createTrayIcon();
  mTrayIcon->setIcon(ico);
  mTrayIcon->setToolTip(trUtf8(APP_NAME));

  mModel = new QDirModel;
  mModel->setReadOnly(true);
  // mModel->setRootPath(home);
  // mModel->setRootPath(QDir::currentPath());
  // QModelIndex idx = mModel->index(QDir::currentPath());
  QModelIndex idx = mModel->index(home);
  // ここで sort すると起動が極端に遅くなる(固まる)のでスキップ
  // (特にネットワークドライブがツリーに含まれていると危険＝起動後でも固まる)
  // mModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
  mTree = new QTreeView;
  mTree->setModel(mModel);
  mTree->header()->setStretchLastSection(true);
  mTree->header()->setSortIndicator(0, Qt::AscendingOrder);
  mTree->header()->setSortIndicatorShown(true);
  mTree->header()->setClickable(true);
  // ※header に Size 情報他が含まれているようなので除くと速くなるかも？
  // mTree->setRootIndex(idx);
  mTree->setCurrentIndex(idx);
  mTree->expand(idx);
  mTree->scrollTo(idx);
  mTree->setColumnHidden(1, true); // Size
  mTree->setColumnHidden(2, true); // Type
  mTree->resizeColumnToContents(0);
  setCentralWidget(mTree);

  loadLayout();
  mTrayIcon->show();
  show();

  QString fname(trUtf8("%1/%2").arg(home).arg(trUtf8(APP_DATA)));
  db = QSqlDatabase::addDatabase(trUtf8(APP_DB));
  db.setDatabaseName(fname);
  if(!QFileInfo(fname).exists()){
    if(!db.open()){
      QString msg(trUtf8("データベースを初期化出来ません\n%1").arg(fname));
      QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
    }else{
      QString msg(trUtf8("データベースが無いため初期化します\n%1").arg(fname));
      QMessageBox::warning(this, trUtf8(APP_NAME), msg);
      QSqlQuery q;
      q.exec("drop table if exists testtable;");
      q.exec("create table testtable (id integer primary key,"
        " c1 varchar(255), c2 varchar(255), c3 integer);");
      db.close();
    }
  }

  dt = new DlThread(th = new QThread(this));
  connect(this, SIGNAL(quit()), th, SLOT(quit()));
  connect(this, SIGNAL(stop()), dt, SLOT(stop()));
  connect(dt, SIGNAL(proc()), this, SLOT(proc()));
  th->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::saveLayout()
{
  QString fname(trUtf8("%1/%2").arg(home).arg(trUtf8(APP_CONF)));
  QFile file(fname);
  if(!file.open(QFile::WriteOnly)){
    QString msg(trUtf8("レイアウトファイルをオープン出来ません(W) %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
  QByteArray geo = saveGeometry();
  QByteArray lay = saveState();
  bool ok = file.putChar((uchar)geo.size());
  if(ok) ok = file.write(geo) == geo.size();
  if(ok) ok = file.write(lay) == lay.size();
  if(!ok){
    QString msg(trUtf8("レイアウトファイルの書き込みに失敗しました %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
}

void MainWindow::loadLayout()
{
  QString fname(trUtf8("%1/%2").arg(home).arg(trUtf8(APP_CONF)));
  if(!QFileInfo(fname).exists()) return;
  QFile file(fname);
  if(!file.open(QFile::ReadOnly)){
    QString msg(trUtf8("レイアウトファイルをオープン出来ません(R) %1\n%2")
      .arg(fname).arg(file.errorString()));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
  uchar geo_size;
  QByteArray geo;
  QByteArray lay;
  bool ok = file.getChar((char *)&geo_size);
  if(ok){
    geo = file.read(geo_size);
    ok = geo.size() == geo_size;
  }
  if(ok){
    lay = file.readAll();
    ok = lay.size() > 0;
  }
  if(ok) ok = restoreGeometry(geo);
  if(ok) ok = restoreState(lay);
  if(!ok){
    QString msg(trUtf8("レイアウトファイルの読み出しに失敗しました %1")
      .arg(fname));
    QMessageBox::warning(this, trUtf8(APP_NAME), msg);
    return;
  }
}

void MainWindow::closeEvent(QCloseEvent *ce)
{
  if(mTrayIcon->isVisible()){
    hide();
    ce->ignore();
  }
}

void MainWindow::createActions()
{
  mDownloadAction = new QAction(QIcon(":/qrc/icon_download"),
    trUtf8("ダウンロード(&G)"), this);
  mDownloadAction->setShortcut(tr("Ctrl+G"));
  mDownloadAction->setStatusTip(trUtf8("Download from url"));
  connect(mDownloadAction, SIGNAL(triggered()), this, SLOT(download()));

  mMinimizeAction = new QAction(QIcon(":/qrc/icon_minimize"),
    trUtf8("最小化(&N)"), this);
  mMinimizeAction->setShortcut(tr("Ctrl+N"));
  mMinimizeAction->setStatusTip(trUtf8("Minimize window"));
  connect(mMinimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

  mMaximizeAction = new QAction(QIcon(":/qrc/icon_maximize"),
    trUtf8("最大化(&X)"), this);
  mMaximizeAction->setShortcut(tr("Ctrl+X"));
  mMaximizeAction->setStatusTip(trUtf8("Maximize window"));
  connect(mMaximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

  mRestoreAction = new QAction(QIcon(":/qrc/icon_restore"),
    trUtf8("復元(&R)"), this);
  mRestoreAction->setShortcut(tr("Ctrl+R"));
  mRestoreAction->setStatusTip(trUtf8("Restore window"));
  connect(mRestoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  mQuitAction = new QAction(QIcon(":/qrc/icon_exit"),
    trUtf8("終了(&Q)"), this);
  mQuitAction->setShortcut(tr("Ctrl+Q"));
  mQuitAction->setStatusTip(trUtf8("Quit the application"));
  connect(mQuitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createMenus()
{
  mFileMenu = menuBar()->addMenu(trUtf8("ファイル(&F)"));
  mFileMenu->addAction(mDownloadAction);
  mFileMenu->addSeparator();
  mFileMenu->addAction(mQuitAction);
  mViewMenu = menuBar()->addMenu(trUtf8("表示(&V)"));
}

void MainWindow::createToolBar()
{
  mFileToolBar = addToolBar(trUtf8("ファイル"));
  mFileToolBar->setObjectName(tr("File"));
  mFileToolBar->addAction(mDownloadAction);
  mFileToolBar->addSeparator();
  mFileToolBar->addAction(mQuitAction);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(trUtf8("完了"));
}

void MainWindow::createDockWindows()
{
  QDockWidget *dockT1 = new QDockWidget(trUtf8("URL"), this);
  dockT1->setObjectName(tr("URL"));
  dockT1->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  QWidget *wT1 = new QWidget();
  QHBoxLayout *hbT1 = new QHBoxLayout();
  mUrl = new QLineEdit(trUtf8(SAMPLE_URL), dockT1);
  hbT1->addWidget(mUrl);
  QPushButton *btnDl = new QPushButton(trUtf8("ダウンロード(&G)"), dockT1);
  connect(btnDl, SIGNAL(clicked()), this, SLOT(download()));
  hbT1->addWidget(btnDl);
  wT1->setLayout(hbT1);
  dockT1->setWidget(wT1);
  addDockWidget(Qt::TopDockWidgetArea, dockT1);
  mViewMenu->addAction(dockT1->toggleViewAction());

  QDockWidget *dockL1 = new QDockWidget(trUtf8("データ"), this);
  dockL1->setObjectName(tr("Data"));
  dockL1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  QWidget *wL1 = new QWidget();
  QVBoxLayout *vbL1 = new QVBoxLayout();
  mText = new QTextEdit(trUtf8("データ"), dockL1);
  vbL1->addWidget(mText);
  wL1->setLayout(vbL1);
  dockL1->setWidget(wL1);
  addDockWidget(Qt::LeftDockWidgetArea, dockL1);
  mViewMenu->addAction(dockL1->toggleViewAction());

  QDockWidget *dockR1 = new QDockWidget(trUtf8("操作"), this);
  dockR1->setObjectName(tr("Buttons"));
  dockR1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  QWidget *wR1 = new QWidget();
  QVBoxLayout *vbR1 = new QVBoxLayout();
  QPushButton *btnTray = new QPushButton(trUtf8("トレイへ(&T)"), dockR1);
  connect(btnTray, SIGNAL(clicked()), this, SLOT(hide()));
  vbR1->addWidget(btnTray);
  QPushButton *btnQuit = new QPushButton(trUtf8("終了(&Q)"), dockR1);
  connect(btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()));
  vbR1->addWidget(btnQuit);
  wR1->setLayout(vbR1);
  dockR1->setWidget(wR1);
  addDockWidget(Qt::RightDockWidgetArea, dockR1);
  mViewMenu->addAction(dockR1->toggleViewAction());
}

void MainWindow::createTrayIcon()
{
  mTrayIconMenu = new QMenu(this);
  mTrayIconMenu->addAction(mMinimizeAction);
  mTrayIconMenu->addAction(mMaximizeAction);
  mTrayIconMenu->addAction(mRestoreAction);
  mTrayIconMenu->addSeparator();
  mTrayIconMenu->addAction(mQuitAction);

  mTrayIcon = new QSystemTrayIcon(this);
  mTrayIcon->setContextMenu(mTrayIconMenu);
  connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch(reason){
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
    ;
    break;
  case QSystemTrayIcon::MiddleClick:
    ;
    break;
  default:
    ;
  }
}

void MainWindow::download()
{
  QUrl url(mUrl->text());

  qDebug("url: %s", mUrl->text().toUtf8().constData());
  Qt2chdl *dl123 = new Qt2chdl(123);
  connect(dl123, SIGNAL(done()), this, SLOT(fin()));
  dl123->get(url);

  qDebug("url: %s", mUrl->text().toUtf8().constData());
  Qt2chdl *dl456 = new Qt2chdl(456);
  connect(dl456, SIGNAL(done()), this, SLOT(fin()));
  dl456->get(url);
}

void MainWindow::fin()
{
  Qt2chdl *dl = qobject_cast<Qt2chdl *>(sender());
  qDebug("num: %d", dl->getnum());

  if(dl->getstat() == 0){
    // エラー時はとりあえず utf-8 でデコード
    QTextCodec *jp = QTextCodec::codecForName("utf-8");
    QString txt = jp->toUnicode(dl->getdat());
    qDebug("error: %s", txt.toUtf8().constData());
    mText->setPlainText(txt);
  }else{
    // sjis 決め打ちは拙いが 2ch 専用ならこれでよい
    QTextCodec *jp = QTextCodec::codecForName("sjis");
    QString txt = jp->toUnicode(dl->getdat());
    qDebug("result: %s", txt.toUtf8().constData());
    mText->setPlainText(txt);

    QStringList lst = txt.split(QChar('\n'));
    QRegExp re(QString("ttp:[^\\s\"'<>]+"), Qt::CaseInsensitive);
    for(QStringList::Iterator i = lst.begin(); i != lst.end(); ++i){
      // if((*i).contains("ＥＲＲＯＲ")) continue;
      int p = 0;
      while((p = re.indexIn(*i, p)) >= 0){
        quelst.enqueue((*i).mid(p, re.matchedLength()));
        p += re.matchedLength();
      }
    }
  }
  delete dl;
}

void MainWindow::proc()
{
#if 1
  static int i = 0;
  if(++i > 200){
    qDebug("[MainWindow proc: %08x]", (uint)th->currentThreadId());
    i = 0;
  }
#endif
  // 再入禁止?
  if(quelst.isEmpty()) return;
  if(!db.open()){
    QString msg(trUtf8("データベースを開けません"));
    QMessageBox::critical(this, trUtf8(APP_NAME), msg, QMessageBox::Cancel);
  }else{
    db.transaction();
    QSqlQuery q;
    while(!quelst.isEmpty()){
      QString link(quelst.dequeue());
      QString v(trUtf8("'%1', '%2', %3").arg(link).arg(link).arg(0));
      // sqlite は複数の values を連結出来ないらしい
      q.exec(trUtf8("insert into testtable (c1, c2, c3) values (%1);").arg(v));
    }
    q.clear();
    if(!db.commit()) db.rollback();
    db.close();
  }
}

void MainWindow::cleanupcode()
{
  qDebug("running clean up code...");
  Qt::HANDLE id = QApplication::instance()->thread()->currentThreadId();
  qDebug("[main thread: %08x]", (uint)id);
  emit stop(); emit quit();
  qDebug("waiting for sub thread finalize...");
  if(th){
    while(!th->isFinished()){
      std::cerr << ".";
      /* mutable */ QMutex mutex;
      QMutexLocker locker(&mutex);
      QWaitCondition cond;
      cond.wait(&mutex, 5);
    }
    delete th; th = 0; // must be deleted before delete dt
  }
  if(dt){ delete dt; dt = 0; } // must be deleted after delete th
  qDebug("done.");
  qDebug("saving layout...");
  saveLayout();
  qDebug("application is cleaned up.");
}
