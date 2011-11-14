#ifndef _SndPlayerAlsa_h_
#define _SndPlayerAlsa_h_

#include <QWave4/SndPlayer.h>
#include <QWave4/Exceptions.h>
#include <alsa/asoundlib.h>

namespace QWave4 {

class SndPlayerAlsa: public SndPlayer
{
private:
    unsigned long long _counter;
    snd_pcm_t* _handle;
    unsigned int _bufferBytes;
    unsigned int _bufferPeriodBytes;
    QMutex _mutex_counter;
    QMutex _mutex_buffer;

  virtual void openDevice() throw (AudioDeviceError);
  virtual void closeDevice();
  virtual unsigned getBufferSize();
  virtual unsigned getBufferFragmentSize();
  virtual unsigned long long bytesPlayed();
  virtual void stopDevice();
  virtual int writeDevice(short*,int);

public:
  SndPlayerAlsa(int channels=2, int samplerate=8000, int resampleQuality=0, int numBuffer=2);
  virtual ~SndPlayerAlsa();
  virtual bool isDevicePlaying();
};

typedef SndPlayerAlsa PLAYERIMPLEMENTATION;

}

#endif
