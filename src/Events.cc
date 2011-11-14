#include "QWave4/Events.h"

namespace QWave4 {

PlayerPositionEvent::PlayerPositionEvent(double t)
  : QEvent((QEvent::Type)PlayerPosition),
    _t(t)
{
}

double
PlayerPositionEvent::time() const
{
  return _t;
}

}
