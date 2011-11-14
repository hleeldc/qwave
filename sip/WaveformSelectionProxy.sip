class WaveformSelectionProxy: QObject
{
%TypeHeaderCode
#include "QWave4/WaveformSelectionProxy.h"
using namespace QWave4;
%End

public:
  WaveformSelectionProxy(QObject* /TransferThis/ =0);
  void registerWaveform(Waveform*);
  void unregisterWaveform(Waveform*);
  void setColorScheme(const QColor&, const QColor&);
  double getBeginSeconds();
  double getWidthSeconds();
  Waveform* getSelectedWaveform();
  void select(const double&, const double&, Waveform*);
public slots:
  void beginSelection(Waveform*,double);
  void expandSelection(Waveform*,double);
  void expandSelectionBegin(Waveform*, double);
  void expandSelectionEnd(Waveform*, double);
  void freezeSelection(Waveform*,double);
signals:
  void waveformSelectionChanged(double, double, Waveform*);
};


