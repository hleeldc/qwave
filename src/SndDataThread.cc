#include "QWave4/SndDataThread.h"
#include "QWave4/SndPlayer.h"
#include "QWave4/SndBuffer.h"
#include "QWave4/Exceptions.h"
#include "QWave4/SndFile.h"

namespace QWave4 {


SndDataThread::SndDataThread(SndPlayer* player, SndFile* sndfile, double offset)
  : QThread(),
    _player(player),
    _outBuffer(player->outBuffer),
    _seekTime(offset),
    _seekFrame((int)(offset*_sfinfo.samplerate)),
    _offset(offset)
{
  // open audio file
  char const * filename = sndfile->getFileName();
  _sndfile = sf_open(filename, SFM_READ, &_sfinfo);
  if (!_sndfile)
    throw SoundFileOpenError(QString("can't open '")+filename+"'");

  // set default output channel and weight on volume
  channelInfo.resize(_sfinfo.channels);
  for (int i=0; i < _sfinfo.channels; ++i) {
    channelInfo[i].weight = 1.0;
    channelInfo[i].outchannel = i % _player->outChannels;
  }

  int err;
  _src_state = src_new(_player->resampleQuality, _sfinfo.channels, &err);
  if (_src_state == NULL)
    throw SampleRateConverterError(src_strerror(err));

  // prepare buffers
  _buf1 = new float[(int)(_player->outBufferDuration * _sfinfo.samplerate * _sfinfo.channels * SndPlayer::MAX_PLAYBACK_SPEED)];
  // we triple the buffer size to avoid segfaults caused by buffer overflow
  _buf2 = new float[(_player->outBufferSizeMax) * _player->outChannels];

  _inOutRatio = (double)_player->outSampleRate / _sfinfo.samplerate;
  _src_data.data_in = _buf1;
  _src_data.data_out = _buf2;
  _src_data.output_frames = (long)(_player->outBufferSize * SndPlayer::MAX_PLAYBACK_SPEED);
  _src_data.end_of_input = 0;

  _outBuffer->registerProducer(this);

}

SndDataThread::~SndDataThread()
{
  _outBuffer->unregisterProducer(this);

  sf_close(_sndfile);
  src_delete(_src_state);
  delete[] _buf1;
  delete[] _buf2;
}

vector<ChannelInfo>*
SndDataThread::getChannelInfo()
{
  return &channelInfo;
}

float*
SndDataThread::getResultBuffer()
{
  return _buf2;
}

void
SndDataThread::seek(double t)
{
  _seekTime = t + _offset;
  _seekFrame = (int)(_seekTime*_sfinfo.samplerate);
  if (_seekFrame > _sfinfo.frames)
    sf_seek(_sndfile, 0, SEEK_END);
  else
    sf_seek(_sndfile, _seekFrame, SEEK_SET);
}

void
SndDataThread::seek2(double t1, double t2)
{
  _seekTime = t1 + _offset;
  _seekFrame = (int)(t2*_sfinfo.samplerate);
  if (_seekFrame > _sfinfo.frames)
    sf_seek(_sndfile, 0, SEEK_END);
  else
    sf_seek(_sndfile, _seekFrame, SEEK_SET);
}

void
SndDataThread::run()
{
  double speed, timing;
  int err;
  int n;
  int n0 = _seekFrame;

  src_reset(_src_state);

  if (_player->repeatedPlay) {

    double limit = _player->beginTime + _offset + _player->playDuration - _seekTime;

    while(!_player->stopSignaled) {

      _outBuffer->getSpeedAndTiming(this, speed, timing);

      // resamle
      n = (int)((_seekTime+timing)*_sfinfo.samplerate);
      //qDebug("%p %d %d %f %f", this, n0, n, _seekTime, timing);
      sf_readf_float(_sndfile, _buf1, n-n0);
      _src_data.input_frames = n-n0;
      _src_data.src_ratio = _inOutRatio / speed;
      n0 = n;

      if ((err=src_process(_src_state, &_src_data)))
	qWarning("%s:%d:%p: %s", __FILE__, __LINE__, this, src_strerror(err));

      _outBuffer->mix(this, _src_data.output_frames_gen);

      // wrap?
      if (timing >= limit) {
	if (timing > 3600.0) {
	  _player->stopSignaled = true;
	  break;
	}
	else {
	  seek(_player->beginTime);
	  limit += _player->playDuration;
	}
      }
    }
  }
  else {
    while(!_player->stopSignaled) {

      _outBuffer->getSpeedAndTiming(this, speed, timing);

      // resamle
      n = (int)((_seekTime+timing)*_sfinfo.samplerate);
      sf_readf_float(_sndfile, _buf1, n-n0);
      _src_data.input_frames = n-n0;
      _src_data.src_ratio = _inOutRatio / speed;
      n0 = n;

      if ((err=src_process(_src_state, &_src_data)))
	qWarning("%s:%d:%p: %s", __FILE__, __LINE__, this, src_strerror(err));

      _outBuffer->mix(this, _src_data.output_frames_gen);
    }
  }
}

}
