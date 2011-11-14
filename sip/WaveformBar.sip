class WaveformBar: QGraphicsLineItem
{
%TypeHeaderCode
#include "QWave4/WaveformBar.h"
using namespace QWave4;
%End

public:
  WaveformBar(Waveform*,double=0.0);
  void adjust();
  void setX(const double&);
  int type() const;
  double getBeginSeconds() const;
};


