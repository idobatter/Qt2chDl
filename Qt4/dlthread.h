/*
  dlthread.h
*/

#ifndef __DLTHREAD_H__
#define __DLTHREAD_H__

#include <QtCore>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QTimer>

class DlThread : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(DlThread)

public:
  DlThread(QThread *thread);
  virtual ~DlThread();

signals:
  void proc();

public slots:
  void stop();

private slots:
  void started();
  void dl();
  void active();

private:
  QTimer timer;
  QThread *th;
  mutable QMutex mutex;
  QWaitCondition cond;
};

#endif // __DLTHREAD_H__
