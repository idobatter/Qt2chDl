/*
  qt2chdl.cpp
*/

#include "qt2chdl.h"

Qt2chdl::Qt2chdl(int n, QObject *parent) : QObject(parent), num(n), stat(0)
{
  qDebug("[Qt2chdl created: %08x]",
    (uint)QApplication::instance()->thread()->currentThreadId());
  nam = new QNetworkAccessManager(this);
  connect(nam, SIGNAL(finished(QNetworkReply *)),
    this, SLOT(fin(QNetworkReply *)));
  connect(nam, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
    this, SLOT(err(QNetworkReply *, const QList<QSslError> &)));
}

Qt2chdl::~Qt2chdl()
{
  qDebug("[Qt2chdl deleted: %08x]",
    (uint)QApplication::instance()->thread()->currentThreadId());
}

void Qt2chdl::get(const QUrl &url)
{
  qDebug("[Qt2chdl get: %08x]",
    (uint)QApplication::instance()->thread()->currentThreadId());

  QNetworkRequest request(url);
  request.setRawHeader("User-Agent", "Qt4");
  nam->get(request);
}

void Qt2chdl::fin(QNetworkReply *reply)
{
  qDebug("[Qt2chdl fin: %08x]",
    (uint)QApplication::instance()->thread()->currentThreadId());

  if(reply->error() != QNetworkReply::NoError){
    // エラー時はとりあえず utf-8 でエンコードして QByteArray にして保存
    QByteArray u8err = reply->errorString().toUtf8();
    qDebug("Error: %s", u8err.constData());
    dat = u8err;
    stat = 0;
  }else{
    dat = reply->readAll();
    stat = 1;
  }
  // reply->deleteLater(); // done() の接続先の方で直接 delete
  emit done();
}
