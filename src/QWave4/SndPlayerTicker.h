#ifndef _SndPlayerTicker_h_
#define _SndPlayerTicker_h_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <set>
#include <QWave4/qwavedefs.h>
using namespace std;

namespace QWave4 {

class SndPlayer;

/// SndPlayerTicker generates synchronization signals every 0.025 seconds.
/**
   The signals indicate the location of the audio file where SndPlayer
   is currently playing.

   To recieve the ticks, the receiver class should inherit QObject and
   reimplement the customEvent method.  Then, the customEvent method should
   check if the type of the received event is PlayerPosition.  If it is,
   your class just received a tick.

   <pre>
   #include <QWave4/Events.h>
   ...

   void
   MyObject::customEvent(QCustomEvent* e)
   {
       if (e->type() == (QEvent::Type)PlayerPosition) {
           double tick = ((PlayerPositionEvent*)e)->time();
           ...
       }
   }
   </pre>

   Finally, your object needs to be registered with the ticker so that the
   ticker can post the tick events to your object.

   <pre>
   MyObject obj;
   theTicker.registerReceiver(&obj);
   </pre>
   
 */
class DLLEXPORT SndPlayerTicker: public QThread
{

  friend class SndPlayer;

public:
  ~SndPlayerTicker();

  /**
     Register the given object with the ticker.  The registered object
     will receive the tick events every 0.025 seconds if the ticker
     is running.
   */
  void registerReceiver(QObject* obj);
  /**
     Unregister the given object.  The unregistered object will not
     receive the tick events anymore.
   */
  void unregisterReceiver(QObject* obj);

  /**
     Reimplementation of QThread::run().  You should not use
     this method.  Instead, use start() to start the thread.
   */
  virtual void run();

  /**
     When the player is paused, the ticker is also paused together.
     This method resume the ticker.
   */
  void resume();

  /**
     Stop the ticker.
   */
  void stop();

private:
  /**
     Construct a ticker object.  This is used by the SndPlayer class.
   */
  SndPlayerTicker(SndPlayer* player);

  SndPlayer* _player;
  bool _stop;
  QWaitCondition _waitCond;
  QMutex _mutex;
  
  set<QObject*> _receivers;
};

}

#endif
