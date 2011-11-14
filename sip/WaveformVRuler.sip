class WaveformVRuler: QWidget
{
%TypeHeaderCode
#include "QWave4/WaveformVRuler.h"
using namespace QWave4;
%End

public:
  WaveformVRuler(QWidget* /TransferThis/ =0);
  void connectToWaveform(Waveform*);
public slots:
  void redraw();
  //void setMutt(bool);
protected:
  virtual void paintEvent(QPaintEvent*);
  virtual void resizeEvent(QResizeEvent*);
};


