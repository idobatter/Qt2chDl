/*
  qt2chdl.h
*/

#ifndef __QT2CHDL_H__
#define __QT2CHDL_H__

#include <QtCore>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkReply>

class QUrl;

class Qt2chdl : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Qt2chdl)

public:
  Qt2chdl(int n, QObject *parent=0);
  virtual ~Qt2chdl();
  int getnum() { return num; }
  int getstat() { return stat; }
  QByteArray& getdat() { return dat; }
  void get(const QUrl &url);

signals:
  void done();

private slots:
  void fin(QNetworkReply *reply);
  void err(QNetworkReply *reply, const QList<QSslError> &errors) {}

private:
  QNetworkAccessManager *nam;
  int num;
  int stat;
  QByteArray dat;
};

#endif // __QT2CHDL_H__
