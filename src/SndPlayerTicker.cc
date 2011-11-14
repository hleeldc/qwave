#include "QWave4/SndPlayerTicker.h"
#include "QWave4/SndPlayer.h"
#include "QWave4/Events.h"
#include <qapplication.h>

#if __GNUC__ < 3
#define not !
#define or ||
#define and &&
#endif

namespace QWave4 {

SndPlayerTicker::SndPlayerTicker(SndPlayer* player)
  : QThread(),
    _player(player),
    _stop(false)
{
}

SndPlayerTicker::~SndPlayerTicker()
{
  stop();
}

void
SndPlayerTicker::registerReceiver(QObject* obj)
{
  _receivers.insert(obj);
}

void
SndPlayerTicker::unregisterReceiver(QObject* obj)
{
  _receivers.erase(obj);
}

void
SndPlayerTicker::run()
{
  set<QObject*>::iterator pos;
  double t;
  _stop = false;
  while (_stop == false) {
    t = _player->playerPosition();
    //qDebug("%20.4f", t);
    for (pos=_receivers.begin(); pos != _receivers.end(); ++pos)
      QApplication::postEvent(*pos, new PlayerPositionEvent(t));
    _mutex.lock();
    if ((_player->stopSignaled or _player->pauseSignaled) and !_stop) {
      _waitCond.wait(&_mutex);
      _mutex.unlock();
    }
    else {
      _mutex.unlock();
      msleep(25);
    }
  }
  for (pos=_receivers.begin(); pos != _receivers.end(); ++pos)
    QApplication::postEvent(*pos, new PlayerPositionEvent(_player->playerPosition()));
}

void
SndPlayerTicker::resume()
{
  _waitCond.wakeOne();
}

void
SndPlayerTicker::stop()
{
  _mutex.lock();
  _stop = true;
  _mutex.unlock();
  _waitCond.wakeOne();
  wait();
}

}
