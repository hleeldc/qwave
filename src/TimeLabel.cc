#include "QWave4/Utils.h"
#include "QWave4/Events.h"
#include "QWave4/TimeLabel.h"

namespace QWave4 {

TimeLabel::TimeLabel(QWidget* parent)
  : QLabel(parent)
{
  QFont f;
  f.setStyleHint(QFont::TypeWriter);
  f.setFixedPitch(true);
  setFont(f);
}

TimeLabel::~TimeLabel()
{
}

void
TimeLabel::setTime(double f)
{
  setText(time2str(f,4));
}

void
TimeLabel::customEvent(QEvent* e)
{
  if (e->type() == (QEvent::Type)PlayerPosition)
    setText(time2str(((PlayerPositionEvent*)e)->time(),4));
}

}
