#ifndef _Events_h_
#define _Events_h_

#include <QEvent>
#include <QWave4/qwavedefs.h>

namespace QWave4 {

enum EventId {
  PlayerPosition = QEvent::User
};

/// PlayerPositionEvent is posted to notify the position of the player.
/**
   Especially, SndPlayerTicker posts this event every .25 seconds
   when it is enabled.
 */
class DLLEXPORT PlayerPositionEvent: public QEvent
{
public:
  /**
     @param t The initial position of the player cursor.
   */
  PlayerPositionEvent(double t);

  /**
     @return The current position of the player cursor in seconds.
   */
  double time() const;

private:
  double _t;
};

}

#endif
