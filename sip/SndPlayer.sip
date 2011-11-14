class SndPlayer: QThread /Abstract/
{
%If (WS_X11)
%TypeHeaderCode
#include "QWave4/SndPlayer.h"
#include "QWave4/SndPlayerAlsa.h"
using namespace QWave4;
%End
%End

%If (WS_WIN)
%TypeHeaderCode
#include "QWave4/SndPlayer.h"
#include "QWave4/SndPlayerDirectSound.h"
using namespace QWave4;
%End
%End

public:
  static const double MAX_PLAYBACK_SPEED;
  static const double MIN_PLAYBACK_SPEED;
  SndPlayer(int=2, int=8000, int=0, int=2);
  void initialize();
  void closePlayer();
  void addSndFile(SndFile*, double=0.0);
  void removeSndFile(SndFile*);
  void enableTicker();
  void disableTicker();
  SndPlayerTicker* getPlayerTicker();
  bool setOutputChannel(SndFile*, int, int);
  bool setWeight(SndFile*, int, double);
  void setSpeed(double);
  void play(double, double);
  void repeat(double, double);
  void pause();
  void resume();
  void stop();
  double playerPosition();
  double secondsPlayed();
  double maxPlaybackDelay();
  virtual bool isDevicePlaying() = 0;
/*
protected:
  virtual void openDevice() throw(AudioDeviceError) = 0;
  virtual void closeDevice() = 0;
  virtual unsigned long bytesPlayed() throw(AudioDeviceError) = 0;
  virtual unsigned getBufferSize() = 0;
  virtual unsigned getBufferFragmentSize() = 0;
  virtual void stopDevice() = 0;
  virtual int writeDevice(short*,int) = 0;
  virtual void run();
  SndBuffer* outBuffer;
  int outBufferSize;  
  int outBufferSizeMax;
  int outBufferFragSize;
  int outBufferFragBytes;
  int outBufferFrameBytes;
  double outBufferDuration;
  int outChannels;
  int outSampleRate;
  int resampleQuality;
  int numBuffer;
  bool repeatedPlay;
  bool stopSignaled;
  bool pauseSignaled;
  double beginTime;
  double playDuration;
  double pausedTime;
*/
};

%If (WS_X11)
SndPlayer* getPlayer(int=2, int=8000, int=0, int=2) /Factory/;
%MethodCode
    sipRes = (QWave4::SndPlayer*) new QWave4::SndPlayerAlsa(a0,a1,a2,a3);
%End
%End

%If (WS_WIN)
SndPlayer* getPlayer(WId, int=2, int=8000, int=0, int=2) /Factory/;
%MethodCode
    sipRes = (QWave4::SndPlayer*) new QWave4::SndPlayerDirectSound(a0,a1,a2,a3,a4);
%End
%End

/*
class SndPlayerOSS: SndPlayer /PyName=SndPlayer/
{
%TypeHeaderCode
#include "QWave4/SndPlayerOSS.h"
using namespace QWave4;
%End

public:
  static const double MAX_PLAYBACK_SPEED;
  static const double MIN_PLAYBACK_SPEED;
  SndPlayerOSS(int=2, int=8000, int=0, int=2);
  void initialize();
  void closePlayer();
  void addSndFile(SndFile*, double=0.0);
  void removeSndFile(SndFile*);
  void enableTicker();
  void disableTicker();
  SndPlayerTicker* getPlayerTicker();
  bool setOutputChannel(SndFile*, int, int);
  bool setWeight(SndFile*, int, double);
  void setSpeed(double);
  void play(double, double);
  void repeat(double, double);
  void pause();
  void resume();
  void stop();
  double playerPosition();
  double secondsPlayed();
  double maxPlaybackError();
};
*/

