#ifndef _SndPlayerPa_h_
#define _SndPlayerPa_h_

#include <QWave4/SndPlayer.h>
#include <QWave4/Exceptions.h>
#include <pulse/pulseaudio.h>

struct session_t {
    pa_stream* stream;
    pa_sample_spec sample_spec;
    int capacity;
};

namespace QWave4 {

class SndPlayerPa: public SndPlayer
{
private:
    pa_threaded_mainloop* mainloop_;
    pa_mainloop_api* api_;
    pa_context* context_;
    struct session_t sess_;
    unsigned int buf_bytes_;
    unsigned int frag_bytes_;
    unsigned long long counter_;
    bool playing_;

  virtual void openDevice() throw (AudioDeviceError);
  virtual void closeDevice();
  virtual unsigned getBufferSize();
  virtual unsigned getBufferFragmentSize();
  virtual unsigned long long bytesPlayed();
  virtual void stopDevice();
  virtual int writeDevice(short*,int);

public:
  SndPlayerPa(int channels=2, int samplerate=8000, int resampleQuality=0, int numBuffer=2);
  virtual ~SndPlayerPa();
  virtual bool isDevicePlaying();
};

typedef SndPlayerPa PLAYERIMPLEMENTATION;

}

#endif
