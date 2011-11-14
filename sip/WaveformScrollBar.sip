class WaveformScrollBar: QScrollBar
{
%TypeHeaderCode
#include "QWave4/WaveformScrollBar.h"
using namespace QWave4;
%End

public:
  WaveformScrollBar(QWidget* /TransferThis/ =0);
  void registerWaveform(Waveform*);
  void unregisterWaveform(Waveform*);
signals:
  void waveformScrollBarScrolled(double);
protected:
  void sliderChange(SliderChange);
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);
  void customEvent(QEvent*);
protected slots:
  void parameterUpdate(double, double);
  void moveWaveform();
};


