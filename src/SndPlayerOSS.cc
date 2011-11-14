#include "SndPlayerOSS.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/soundcard.h>
#include <string>
using std::string;

namespace QWave4 {

const char* SndPlayerOSS::dsp = "/dev/dsp";
const int SndPlayerOSS::format = AFMT_S16_NE;
const char* SndPlayerOSS::format_str = "AFMT_S16_NE";

SndPlayerOSS::SndPlayerOSS(int channels, int samplerate, int resampleQuality, int numBuffer)
  : SndPlayer(channels, samplerate, resampleQuality, numBuffer),
    playing(false),
    counter(0)
{
}

SndPlayerOSS::~SndPlayerOSS()
{
  //qDebug("SndPlayerOSS being destroyed");
}

void
SndPlayerOSS::openDevice()
{
  int val;

  /* open audio device */
  if ((fd = open(dsp,O_WRONLY,0)) == -1) {
    perror(dsp);
    throw AudioDeviceError(QString(dsp) + ": " +
                           const_cast<char const*>(strerror(errno)));
  }

  /* check trigger */
  ioctl(fd,SNDCTL_DSP_GETCAPS,&val);
  _trigger = (val & DSP_CAP_TRIGGER) ? true : false;

  /* set fragment size */
  val = 0x00080008;
  if (ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&val) == -1) {
    perror("SNDCTL_DSP_SETFMT");
    throw AudioDeviceError(QString("SNDCTL_DSP_SETFRAGMENT: ") +
                           const_cast<char const*>(strerror(errno)));
  }

  /* set data format */
  val = format;
  if (ioctl(fd,SNDCTL_DSP_SETFMT,&val) == -1) {
    perror("SNDCTL_DSP_SETFMT");
    throw AudioDeviceError(QString("SNDCTL_DSP_SETFMT: ") +
                           const_cast<char const*>(strerror(errno)));
  }

  if (val != format) {
    printf("%s not supported\n", format_str);
    throw AudioDeviceError(QString(format_str) + " not supported");
  }

  /* set channels */
  val = outChannels;
  if (ioctl(fd,SNDCTL_DSP_CHANNELS,&val) == -1) {
    perror("SNDCTL_DSP_CHANNELS");
    throw AudioDeviceError(QString("SNDCTL_DSP_CHANNELS: ") +
                           const_cast<char const*>(strerror(errno)));
  }
  outChannels = val;

  /* set speed */
  val = outSampleRate;
  if (ioctl(fd, SNDCTL_DSP_SPEED, &val) == -1) {
    perror("SNDCTL_DSP_SPEED");
    throw AudioDeviceError(QString("SNDCTL_DSP_SPEED: ") +
                           const_cast<char const*>(strerror(errno)));
  }
  outSampleRate = val;

  if (ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &val) == -1) {
    perror("SNDCTL_DSP_GETBLKSIZE");
    throw AudioDeviceError(QString("SNDCTL_DSP_GETBLKSIZE: ") +
                           const_cast<char const*>(strerror(errno)));
  }

  audio_buf_info info;

  if (ioctl(fd, SNDCTL_DSP_GETOSPACE, &info) == -1) {
    perror("SNDCTL_DSP_GETOSPACE");
    throw AudioDeviceError(QString("SNDCTL_DSP_GETOSPACE: ") +
                           const_cast<char const*>(strerror(errno)));
  }

  _bufferBytes = (unsigned)(val * info.fragstotal);
  _bufferFragBytes = (unsigned)(val);
}

void
SndPlayerOSS::closeDevice()
{
  close(fd);
}

unsigned
SndPlayerOSS::getBufferSize()
{
  return _bufferBytes;
}

unsigned
SndPlayerOSS::getBufferFragmentSize()
{
  return _bufferFragBytes;
}

unsigned long long
SndPlayerOSS::bytesPlayed()
{
  /*
  count_info val;
  //QMutexLocker locker(&_mutex);
  if (ioctl(fd, SNDCTL_DSP_GETOPTR, &val) == -1) {
    perror("SNDCTL_DSP_GETOPTR");
    throw AudioDeviceError(string("SNDCTL_DSP_GETOPTR: ")+strerror(errno));
  }
  return val.bytes;
  */

  QMutexLocker locker(&_mutex);
  int val;
  if (ioctl(fd, SNDCTL_DSP_GETODELAY, &val) == -1) {
    perror("SNDCTL_DSP_GETODELAY");
    throw AudioDeviceError(QString("SNDCTL_DSP_GETODELAY: ") +
                           const_cast<char const*>(strerror(errno)));
  }
  int x = counter - val;
  return x<0 ? 0 : x;
}

void
SndPlayerOSS::stopDevice()
{
  ioctl(fd, SNDCTL_DSP_RESET, 0);
  playing = false;
  counter = 0;
}

int
SndPlayerOSS::writeDevice(short* buf, int n)
{
  write(fd, buf, n);
  playing = true;
  counter += n;
  return n;
}

}
