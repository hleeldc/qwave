#ifndef _QWavePlayerSunAudio_h_
#define _QWavePlayerSunAudio_h_

#include "QWavePlayer.h"
#include "QWaveExceptions.h"
#include <sys/audio.h>

class QWavePlayerSunAudio: public QWavePlayer
{
private:
  static const char* device;
  int fd;
  audio_info_t audio_info;
  int frame_size;

  void openDevice(int*,int*);
  void closeDevice();
  unsigned getBufferSize();
  unsigned bytesPlayed();
  void stopDevice();
  bool isDevicePlaying() { return audio_info.play.active; }
  int writeDevice(short*,int);
  
public:
  QWavePlayerSunAudio(QWave* wave)
    : QWavePlayer(wave)
  {}
  
  QWavePlayerSunAudio(QWave* wave, SNDFILE* sndfile, SF_INFO* sfinfo)
    : QWavePlayer(wave,sndfile,sfinfo)
  {}

  ~QWavePlayerSunAudio();
};

typedef QWavePlayerSunAudio PLAYERIMPLEMENT;

#endif
