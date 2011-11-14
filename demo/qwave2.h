#ifndef _qwave2_h_
#define _qwave2_h_

#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWave4/SndFile.h>
#include <QWave4/Waveform.h>
#include <QWave4/WaveformScrollBar.h>
#include <QWave4/WaveformRuler.h>
#include <QWave4/WaveformCursorProxy.h>
#include <QWave4/WaveformSelectionProxy.h>
#include <QWave4/TimeLabel.h>
#include <QWave4/SndPlayer.h>
#include <vector>
#include <map>
using namespace QWave4;

class MyWidget: public QWidget
{
  Q_OBJECT

public:
  MyWidget();
  ~MyWidget();
  bool has_error_occurred() { return error_occurred; }

public slots:  
  void addSndFile();
  void browseSndFile();
  void play();
  void repeat();
  void pauseResume();
  void stop();

  void changeSelection(double beg, double dur, Waveform*);
  void setSpeed(int);

private slots:
  void setTime(Waveform*,double);

private:
  QVBoxLayout* layout;         // main layout
  SndPlayer* player;

  QLineEdit* fileEntry;
  QPushButton* fileBrowseBtn;
  QPushButton* fileAddBtn;
  QGridLayout* grid;     // 4 columns to contain waveforms
  int gridCurRow;
  WaveformRuler* ruler;
  WaveformScrollBar* sb;
  WaveformCursorProxy* cursor;
  WaveformSelectionProxy* selection;

  TimeLabel* tm;
  TimeLabel* tb;
  TimeLabel* te;
  TimeLabel* td;

  QPushButton* playBtn;
  QPushButton* repeatBtn;
  QPushButton* pauseBtn;
  QPushButton* stopBtn;

  QSlider* speedSlider;

  bool paused;
  bool hasRuler;

  vector<SndFile*> sndfiles;
  map<int,map<int,Waveform*> > waveforms;

  bool error_occurred;
};

#endif
