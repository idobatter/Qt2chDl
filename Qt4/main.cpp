/*
  main.cpp
*/

#include "mainwindow.h"

int main(int ac, char **av){
  // 起動インスタンスをひとつにするには QSingle(Core)Application を使う option
  QApplication app(ac, av);
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

/*
  QTranslator appTranslator;
  appTranslator.load(QLocale::system().name(), ":/translations");
  app.installTranslator(&appTranslator);

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
    qApp->applicationDirPath());
  app.installTranslator(&qtTranslator);
*/

  // TrayIcon を使うので DeleteOnClose も destroyed への接続も不要
  if(!QSystemTrayIcon::isSystemTrayAvailable()){
    QMessageBox::critical(0, QObject::trUtf8(APP_NAME),
      QObject::trUtf8("システムトレイがありません"));
      return 1;
  }
  QApplication::setQuitOnLastWindowClosed(false);

  QQueue<QString> quelst;
  MainWindow w(quelst);
  // w.setAttribute(Qt::WA_DeleteOnClose); // 下の connect と両方だと UAE 出る
  // connect(&w, SIGNAL(destroyed()), &app, SLOT(quit()));

  return app.exec();
}
