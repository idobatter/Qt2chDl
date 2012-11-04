/*
  dlthread.cpp
*/

#include "dlthread.h"

DlThread::DlThread(QThread *thread) : QObject(), // fource set parent = 0
  timer(this), th(thread)
{
  qDebug("[DlThread created: %08x]", (uint)th->currentThreadId());
  this->moveToThread(th);
  connect(th, SIGNAL(started()), this, SLOT(started()));
}

DlThread::~DlThread()
{
  qDebug("[DlThread deleted: %08x]", (uint)th->currentThreadId());
}

void DlThread::stop()
{
  timer.stop();
  qDebug("[DlThread timer stop: %08x]", (uint)th->currentThreadId());
}

void DlThread::started()
{
  qDebug("[DlThread timer start: %08x]", (uint)th->currentThreadId());
  connect(&timer, SIGNAL(timeout()), this, SLOT(dl()));
  timer.start(99); // QTimer can only be used with threads started with QThread
}

void DlThread::dl()
{
#if 1
  static int i = 0;
  if(++i > 200){
    qDebug("[DlThread dl: %08x]", (uint)th->currentThreadId());
    i = 0;
  }
#endif
  // 今のところ二重に呼ばれる訳ではない
  {
    QMutexLocker locker(&mutex);
    emit proc();
  }
}

void DlThread::active()
{
  cond.wakeOne();
  qDebug("actived");
}
