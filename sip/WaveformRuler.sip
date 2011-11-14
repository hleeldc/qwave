class WaveformRuler: QWidget
{
%TypeHeaderCode
#include "QWave4/WaveformRuler.h"
using namespace QWave4;
%End

protected:
  virtual void paintEvent(QPaintEvent*);
  virtual void resizeEvent(QResizeEvent*);
public:
  WaveformRuler(bool=false, QWidget* /TransferThis/ =0);
  void connectToWaveform(Waveform*);
  void disconnectWaveform();
public slots:
  void redraw(double,double);
  void redraw(bool=true);
};


