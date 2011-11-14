%ModuleHeaderCode
#include "QWave4/Events.h"
%End

enum EventId {
  PlayerPosition = QEvent::User
};


class PlayerPositionEvent: QEvent
{
%TypeHeaderCode
#include "QWave4/Events.h"
using namespace QWave4;
%End

public:
  PlayerPositionEvent(double);
  double time() const;
};

