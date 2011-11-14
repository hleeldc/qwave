#include "QWave4/SndPlayer.h"
#include "QWave4/SndDataThread.h"
#include "QWave4/SndBuffer.h"
#include "QWave4/Exceptions.h"
#include "QWave4/SndFile.h"
#include <float.h>
#include <cmath>
#include <QDebug>

namespace QWave4 {

const double SndPlayer::MAX_PLAYBACK_SPEED = 4.0;
const double SndPlayer::MIN_PLAYBACK_SPEED = 0.25;

SndPlayer::SndPlayer(int channels, int samplerate, int quality, int _numBuffer)
  : QThread(),
    outBufferFrameBytes(sizeof(short)*channels),
    outChannels(channels),
    outSampleRate(samplerate),
    resampleQuality(quality),
    numBuffer(_numBuffer),
    repeatedPlay(false),
    stopSignaled(true),
    pauseSignaled(false),
    beginTime(0.0),
    _speed(1.0),
    _byterate(samplerate*sizeof(short)*channels),
    _ticker(this)
{
}

SndPlayer::~SndPlayer()
{
  //qDebug("SndPlayer being destroyed");
  _ticker.stop();
  stop();
  //qDebug("SndPlayer destroyed");
}

void
SndPlayer::initialize()
throw(AudioDeviceError)
{
    try {
        openDevice();
    }
    catch (AudioDeviceError const& e) {
        throw;
    }

  outBufferSize = getBufferSize() / outBufferFrameBytes;
  outBufferSizeMax = outBufferSize * 3;
  outBufferFragSize = getBufferFragmentSize() / outBufferFrameBytes;
  outBufferFragBytes = outBufferFragSize * outBufferFrameBytes;
  outBufferDuration = (double)outBufferSize / outSampleRate;
  outBuffer = new SndBuffer(this, outChannels,outBufferSizeMax,outBufferDuration,numBuffer,_speed);
  _finalBuffer = new short[(outBufferSizeMax) * outChannels];
  t2spd[0.0] = make_pair(0.0,0.0);
}

void
SndPlayer::closePlayer()
{
  _ticker.stop();
  stop();
  map<SndFile*,SndDataThread*>::iterator pos;
  for (pos=_dataThreads.begin(); pos != _dataThreads.end(); ++pos)
    delete pos->second;
  delete[] _finalBuffer;
  closeDevice();
}

void
SndPlayer::addSndFile(SndFile* sndfile, double offset)
{
  if (_dataThreads.find(sndfile) == _dataThreads.end())
    _dataThreads[sndfile] = new SndDataThread(this, sndfile, offset);
}

void
SndPlayer::removeSndFile(SndFile* sndfile)
{
  delete _dataThreads[sndfile];
  _dataThreads.erase(sndfile);
}

void
SndPlayer::enableTicker()
{
  _ticker.stop();
  _ticker.start();
}

void
SndPlayer::disableTicker()
{
  _ticker.stop();
}

SndPlayerTicker*
SndPlayer::getPlayerTicker()
{
  return &_ticker;
}

bool
SndPlayer::setWeight(SndFile* sndfile, int channel, double w)
{
  if (channel<0 or channel>=sndfile->getChannels())
    return false;

  if (_dataThreads.find(sndfile) == _dataThreads.end())
    return false;

  _dataThreads[sndfile]->channelInfo[channel].weight = w;
  return true;
}

bool
SndPlayer::setOutputChannel(SndFile* sndfile, int channel, int outchannel)
{
  if (channel<0 or channel>=sndfile->getChannels())
    return false;

  if (outchannel<-1 or outchannel>=2)
    return false;

  _dataThreads[sndfile]->channelInfo[channel].outchannel = outchannel;
  return true;
}

void
SndPlayer::setSpeed(double speed)
{
  if (speed <= SndPlayer::MIN_PLAYBACK_SPEED)
    _speed = SndPlayer::MIN_PLAYBACK_SPEED;
  else if (speed >= SndPlayer::MAX_PLAYBACK_SPEED)
    _speed = SndPlayer::MAX_PLAYBACK_SPEED;
  else
    _speed = speed;
}

void
SndPlayer::play(double beg, double dur)
{
  if (_dataThreads.size() == 0)
      return;

  if (dur <= 0.0)
      return;

  if (mutex.tryLock()) {
    if (isRunning())
        stop();
    beginTime = pausedTime = beg;
    playDuration = dur;
    repeatedPlay = false;
    stopSignaled = false;
    pauseSignaled = false;
    mutex.unlock();
    start();
    _ticker.resume();
  }
}

void
SndPlayer::repeat(double beg, double dur)
{
  if (_dataThreads.size() == 0) return;
  //qDebug("--------------------------");
  if (dur <= 0.0) return;
  if (mutex.tryLock()) {
    //qDebug("+++++++++++++++++++++++");
    if (isRunning()) stop();
    beginTime = pausedTime = beg;
    playDuration = dur;
    repeatedPlay = true;
    stopSignaled = false;
    pauseSignaled = false;
    mutex.unlock();
    start();
    _ticker.resume();
  }
}

void
SndPlayer::pause()
{
  if (_dataThreads.size() == 0) return;
  if (!stopSignaled)
    pauseSignaled = true;
}

void
SndPlayer::resume()
{
  if (_dataThreads.size() == 0) return;
  if (pauseSignaled) {
    pauseSignaled = false;
    resumeSignal.wakeAll();
    _ticker.resume();
  }
}

void
SndPlayer::stop()
{
  if (_dataThreads.size() == 0) return;
  repeatedPlay = false;
  stopSignaled = true;
  pauseSignaled = false;
  resumeSignal.wakeAll();
  wait();
}

double
SndPlayer::playerPosition()
{
    if (isDevicePlaying()) {
        if (repeatedPlay)
            return beginTime + fmod(pausedTime - beginTime + secondsPlayed(), playDuration);
        else
            return pausedTime + secondsPlayed();
    }
    else {
        if (stopSignaled)
            return beginTime;
        else if (pauseSignaled)
            return pausedTime;
        else
            return pausedTime + secondsPlayed();
    }
}

double
SndPlayer::secondsPlayed()
{
  double elapsed = static_cast<double>(bytesPlayed())/_byterate;
  map<double,pair<double,double>,greater<double> >::iterator pos = t2spd.lower_bound(elapsed);
  return pos->second.first + (elapsed - pos->first) * pos->second.second;
}

double
SndPlayer::maxPlaybackDelay()
{
  return (double)outBufferFragSize/outSampleRate;
}

void
SndPlayer::run()
{
  int n; // total number of frames/bytes to write
  int m; // total number of bytes written
  int k; // number of bytes to write in the current call
  double spd;
  double timing;
  int total_frames = 0;
  double spd0 = -1.0;
  int fragsamples = outBufferFragSize * outChannels;
  int i;

  startDataThreads(beginTime);
  t2spd.clear();
  t2spd[0.0] = make_pair(0.0,0.0);

  if (repeatedPlay) {
    double paused_time = 0.0;
    while (!stopSignaled) {

      n = outBuffer->produce(_finalBuffer, timing, spd);
      total_frames += n;
      if (spd != spd0) {
	t2spd[(double)total_frames/outSampleRate] = make_pair(timing,spd);
	spd0 = spd;
      }

      for (i=0; i < n/outBufferFragSize; ++i) {
      	writeDevice(_finalBuffer+i*fragsamples, outBufferFragBytes);
	if (stopSignaled || pauseSignaled) break;
      }

      if (pauseSignaled) {
	//qDebug("puaseSignal detected");
	paused_time = fmod(paused_time+secondsPlayed(),playDuration);
	stopDevice();
	stopDataThreads();
	pausedTime = beginTime + paused_time;
	startDataThreads(pausedTime);
	t2spd.clear();
	t2spd[0.0] = make_pair(0.0,0.0);
	total_frames = 0;
	spd0 = -1.0;
	//qDebug("resumeSignal wait");
	resumeSignalMutex.lock();
	resumeSignal.wait(&resumeSignalMutex);
        resumeSignalMutex.unlock();
	//qDebug("resumeSignal wait done");
      }
    }
  }
  else {
    double limit = playDuration;
    while (secondsPlayed()<limit && !stopSignaled) {

        n = outBuffer->produce(_finalBuffer, timing, spd, limit);
        // qDebug("timing: %f  spd: %f", timing, spd);
        total_frames += n;
        if (spd != spd0) {
            t2spd[(double)total_frames/outSampleRate] = make_pair(timing,spd);
            spd0 = spd;
        }

        n = n * outBufferFrameBytes;
        m = 0;
        while (m < n && secondsPlayed() < limit && !stopSignaled) {

            if (pauseSignaled) {
                limit -= secondsPlayed();
                stopDevice();
                stopDataThreads();
                pausedTime = beginTime + playDuration - limit;
                startDataThreads(pausedTime);
                t2spd.clear();
                t2spd[0.0] = make_pair(0.0,0.0);
                total_frames = 0;
                spd0 = -1.0;
                resumeSignalMutex.lock();
                resumeSignal.wait(&resumeSignalMutex);
                resumeSignalMutex.unlock();
            }

            k = (n-m > outBufferFragBytes) ? outBufferFragBytes : n-m;
            m += writeDevice(_finalBuffer + m / sizeof(short), k);

        }
//        if (m < 0) break;
//        if (pauseSignaled) {
//            limit -= secondsPlayed();
//            stopDevice();
//            stopDataThreads();
//            pausedTime = beginTime + playDuration - limit;
//            startDataThreads(pausedTime);
//            t2spd.clear();
//            t2spd[0.0] = make_pair(0.0,0.0);
//            total_frames = 0;
//            spd0 = -1.0;
//            resumeSignalMutex.lock();
//            resumeSignal.wait(&resumeSignalMutex);
//            resumeSignalMutex.unlock();
//        }
    }
  }

  //qDebug("//////////////////////////////");
  stopDevice();
  stopDataThreads();
}

void
SndPlayer::startDataThreads(double t)
{
  map<SndFile*,SndDataThread*>::iterator dt;

  stopSignaled = false;

  for (dt=_dataThreads.begin(); dt!=_dataThreads.end(); ++dt)
    dt->second->seek(t);

  outBuffer->initialize();

  for (dt=_dataThreads.begin(); dt!=_dataThreads.end(); ++dt)
    dt->second->start();
}

void
SndPlayer::stopDataThreads()
{
  //qDebug("................................");
  map<SndFile*,SndDataThread*>::iterator dt;
  
  stopSignaled = true;        // stop data threads
  
  // there could be data threads stuck in the buffer
  // wake them all by force
  outBuffer->forceWakeAll();
  
  //qDebug("wait data threads");
  for (dt=_dataThreads.begin(); dt!=_dataThreads.end(); ++dt)
    dt->second->wait();
  //qDebug("wait data threads done");
}

}
