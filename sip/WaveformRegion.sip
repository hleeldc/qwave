class WaveformRegion: QGraphicsRectItem
{
%TypeHeaderCode
#include "QWave4/WaveformRegion.h"
using namespace QWave4;
%End

public:
  WaveformRegion(Waveform*,double=0.0,double=0.0);
  void adjust();
  void setRegion(const double&, const double&);
  int type() const;
};


