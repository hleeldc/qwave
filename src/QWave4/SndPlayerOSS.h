#ifndef _SndPlayerOSS_h_
#define _SndPlayerOSS_h_

#include <QWave4/SndPlayer.h>
#include <QWave4/Exceptions.h>

namespace QWave4 {

class SndPlayerOSS: public SndPlayer
{
private:
  static const char* dsp;
  static const int format;
  static const char* format_str;
  int fd;
  bool _trigger;
  bool playing;
  unsigned long long counter;
  unsigned _bufferBytes;
  unsigned _bufferFragBytes;
  QMutex _mutex;

  virtual void openDevice();
  virtual void closeDevice();
  virtual unsigned getBufferSize();
  virtual unsigned getBufferFragmentSize();
  virtual unsigned long long bytesPlayed();
  virtual void stopDevice();
  virtual int writeDevice(short*,int);
  
public:
  SndPlayerOSS(int channels=2, int samplerate=8000, int resampleQuality=0, int numBuffer=2);
  virtual ~SndPlayerOSS();
  virtual bool isDevicePlaying() { return playing; }
};

typedef SndPlayerOSS PLAYERIMPLEMENTATION;

}

#endif
