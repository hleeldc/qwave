class Waveform: QGraphicsView
{
%TypeHeaderCode
#include "QWave4/Waveform.h"
using namespace QWave4;
%End

public:
  Waveform(SndFile*,int,double=0.0,double=60.0,QWidget* /TransferThis/ =0);

  int getChannel() const;
  double getAmplitudeRatio() const;
  QPaintDevice* getPaintDevice();
  WaveformBar* addBar(const double&) /Factory/;
  WaveformRegion* addRegion(const double&, const double&) /Factory/;
  QGraphicsScene* getCanvas();
  SndFile* getSndFile() const;
  double getBeginFrames() const;
  double getWidthFrames() const;
  double getBeginSeconds() const;
  double getWidthSeconds() const;
  double getBeginPixels() const;
  int getWidthPixels() const;
  int getHeightPixels() const;
  double getSecondsPerPixel() const;
  double getPixelsPerSecond() const;
  double getFramesPerPixel() const;
  double getPixelsPerFrame() const;
  double getSecondsPerFrame() const;
  double getFramesPerSecond() const;
  double px2time(const double&) const;
  double time2px(const double&) const;
  double frm2px(const double&) const;
  double px2frm(const double&) const;
  double frm2time(const double&) const;
  double time2frm(const double&) const;
public slots:
  void display(double, double);
  void display(double);
  void setAmplitudeRatio(double);
signals:
  void waveformHorizontallyChanged(double, double);
  void waveformWindowResized();
  void waveformMousePressed(Waveform*,double);
  void waveformMouseMoved(Waveform*,double);
  void waveformMouseReleased(Waveform*,double);
protected:
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
};


