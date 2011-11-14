class WaveformCursorProxy: QObject
{
%TypeHeaderCode
#include "QWave4/WaveformCursorProxy.h"
using namespace QWave4;
%End

public:
  WaveformCursorProxy(QObject* /TransferThis/ =0);
  void registerWaveform(Waveform*);
  void unregisterWaveform(Waveform*);
  double getCursorPositionSeconds();
public slots:
  void updateCursorPosition(Waveform*,double);
  void setColorScheme(const QColor&, const QColor&);
protected:
  void customEvent(QEvent*);
};


