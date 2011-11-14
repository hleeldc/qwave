#include <QWave4/SndBuffer.h>
#include <QWave4/SndPlayer.h>
#include <QWave4/SndDataThread.h>
#include <cmath>

#if __GNUC__ < 3
#define not !
#define or ||
#define and &&
#endif

namespace QWave4 {

//------------------------------------------------------------
SndBufferEnt::SndBufferEnt(int channels, int frames)
  : counter(0),
    data(new double[channels*(frames)]),
    weight(2),
    _channels(channels),
    _frames(frames)
{
}

SndBufferEnt::~SndBufferEnt()
{
  delete[] data;
}

void
SndBufferEnt::initialize()
{
  double *p = data;
  for (int ch=0; ch<_channels; ++ch) {
    for (int i=0; i<_frames; ++i) {
      *p++ = 0.0;
    }
    weight[ch] = 0.0;
  }
  counter = 0;
  frames = 0;
}


//------------------------------------------------------------
SndBuffer::SndBuffer(SndPlayer* _player,
		     int _channels,
		     int _frames,
		     double _interval,
		     int _size,
		     double& _speed)
  : player(_player),
    channels(_channels),
    size(_size),
    interval(_interval),
    speed(_speed),
    player_called_forceWakeAll(false),
    consumerSlot(0),
    consumerRound(false),
    regSem(1),
    numDt(0)
{
  for (int i=0; i<_size; ++i) {
    buffers.push_back(new SndBufferEnt(_channels,_frames));
  }
}

SndBuffer::~SndBuffer()
{
  for (unsigned i=0; i<buffers.size(); ++i)
    delete buffers[i];
}

void
SndBuffer::initialize()
{
  map<SndDataThread*,SndBufferDTRegEnt>::iterator pos;
  for (int i=0; i<size; ++i) {
    SndBufferEnt* bufEnt = buffers[i];
    bufEnt->initialize();
    bufEnt->speed = speed;
    latestTiming = bufEnt->timing = interval * bufEnt->speed * (i+1);
    //qDebug("slot %d timing %f", i, latestTiming);
  }
  for (pos=dtReg.begin(); pos!=dtReg.end(); ++pos) {
    pos->second.slot = 0;
    pos->second.round = false;
  }
  consumerSlot = 0;
  consumerRound = false;
  player_called_forceWakeAll = false;
}

void
SndBuffer::registerProducer(SndDataThread* t)
{
  regSem.acquire();
  vector<ChannelInfo>* chinfo = t->getChannelInfo();
  dtReg[t].channelInfo = chinfo;
  dtReg[t].slot = consumerSlot;
  dtReg[t].round = consumerRound;
  dtReg[t].channels = chinfo->size();
  dtReg[t].buf = t->getResultBuffer();
  numDt++;
  regSem.release();
  if (!player_called_forceWakeAll) {
    //qDebug("%d", (consumerSlot+size-1)%size);
    t->seek2(player->pausedTime, player->pausedTime + buffers[consumerSlot]->timing);
    // t will write 0 frames for the first buffer
    t->start();
  }
}

void
SndBuffer::unregisterProducer(SndDataThread* t)
{
  regSem.acquire();
  dtReg.erase(t);
  numDt--;
  regSem.release();
}

void
SndBuffer::getSpeedAndTiming(SndDataThread* t, double& spd, double& timing)
{
  SndBufferDTRegEnt& dtEnt = dtReg[t];
  SndBufferEnt* bufEnt = buffers[dtEnt.slot];

  // this region is locked to avoid a deadlock condition, in which
  //   t1. player thread calls bufferEmpty.wakeAll() (by calling forceWakeAll())
  //   t2. data thread just gets on bufferEmpty.wait() (in getSpeedAndTiming())
  //   t3. player thread waits for data thread to be terminated
  // 
  // by locking the whole "if/else if" block, only one of these holds
  //   - t2 doesn't happen
  //   - if t2 happens, then t1 follows t2
  // in either case, data thread doesn't wait bufferEmpty.  eliminating
  // the deadlock condition
  bufEnt->mutex.lock();
  if (player_called_forceWakeAll) {
    spd = timing = -1.0;
  }
  else if (dtEnt.slot == consumerSlot and dtEnt.round != consumerRound) {
    //qDebug("bufferEmpty wait .. %p", t);
    bufEnt->bufferEmpty.wait(&(bufEnt->mutex));
    //qDebug("bufferEmpty wait .. %p done", t);
  }
  bufEnt->mutex.unlock();
  spd = bufEnt->speed;
  timing = bufEnt->timing;
}

void
SndBuffer::mix(SndDataThread* t, int n)
{
  SndBufferDTRegEnt& dtEnt = dtReg[t];
  SndBufferEnt* bufEnt = buffers[dtEnt.slot];
  float* p1;
  double* p2;

  int ch, och, i;

  bufEnt->mutex.lock();
  if (player_called_forceWakeAll) {
    bufEnt->mutex.unlock();
    return;
  }
  else if (dtEnt.slot == consumerSlot and dtEnt.round != consumerRound) {
    //qDebug("bufferEmpty wait %p", t);
    bufEnt->bufferEmpty.wait(&(bufEnt->mutex));
    //qDebug("bufferEmpty wait %p done", t);
    bufEnt->mutex.unlock();
    if (player_called_forceWakeAll) return;
    bufEnt->mutex.lock();
  }
  for (ch=0; ch<dtEnt.channels; ++ch) {
    #if __GNUC__ < 3
    ChannelInfo& chinfo = (*dtEnt.channelInfo)[ch];
    #else
    ChannelInfo& chinfo = dtEnt.channelInfo->at(ch);
    #endif
    if (chinfo.outchannel == -1) {
      for (och=0; och<channels; ++och) {
	bufEnt->weight[och] += chinfo.weight;
	p1 = dtEnt.buf + ch;
	p2 = bufEnt->data + och;
	for (i=0; i < n; ++i,p1+=dtEnt.channels,p2+=channels) {
	  *p2 += *p1 * chinfo.weight;
	}
      }
    }
    else {
      och = chinfo.outchannel;
      bufEnt->weight[och] += chinfo.weight;
      p1 = dtEnt.buf + ch;
      p2 = bufEnt->data + och;
      for (i=0; i < n; ++i,p1+=dtEnt.channels,p2+=channels)
	*p2 += *p1 * chinfo.weight;
    }
  }
  if (bufEnt->frames < n)
    bufEnt->frames = n;
  dtEnt.slot = (dtEnt.slot + 1) % size;
  dtEnt.round = dtEnt.slot ? dtEnt.round : !dtEnt.round;
  if (++bufEnt->counter >= numDt) {
    //qDebug("bufferReady.wakeAll %p %d", t, bufEnt->counter);
    bufEnt->bufferReady.wakeAll();
  }
  bufEnt->mutex.unlock();
}

int
SndBuffer::produce(short* data, double& timing, double& spd, double limit)
{
  SndBufferEnt* bufEnt = buffers[consumerSlot];
  double* p1;
  short* p2;
  int ch, i;
  double scale;
  int frm2cp;

  regSem.acquire();
  bufEnt->mutex.lock();
  if (player_called_forceWakeAll) {
    bufEnt->mutex.unlock();
    regSem.release();
    return 0;
  }
  else if (bufEnt->counter < numDt or numDt==0) {
    //qDebug("bufferReady wait");
    bufEnt->bufferReady.wait(&(bufEnt->mutex));
    //qDebug("bufferReady wait done");
    bufEnt->mutex.unlock();
    if (player_called_forceWakeAll) {
      regSem.release();
      return 0;
    }
    bufEnt->mutex.lock();
  }

  // If the size of data that has been produced exceeds the size (in time)
  // specified by "limit", set value of the extra samples to 0.
  if (limit > 0.0 && bufEnt->timing >= limit) {
      double ratio_extra = (bufEnt->timing - limit) / interval;
      if (ratio_extra >= 1.0) {
          frm2cp = 0;
      }
      else {
          frm2cp = round((1.0 - ratio_extra) * bufEnt->frames);
          frm2cp = min(frm2cp, bufEnt->frames);
      }
  }
  else {
      frm2cp = bufEnt->frames;
  }

  for (ch=0; ch<channels; ++ch) {
    p1 = bufEnt->data + ch;
    p2 = data + ch;
    scale = (double) SHRT_MAX / bufEnt->weight[ch];
    bufEnt->weight[ch] = 0.0;
    for (i=0; i < frm2cp; ++i, p1+=channels, p2+=channels) {
      *p2 = (short)(*p1 * scale);
      *p1 = 0.0;
    }
    for (i=frm2cp; i < bufEnt->frames; ++i, p1+=channels, p2+=channels) {
        *p2 = 0;
        *p1 = 0.0;
    }
  }

  bufEnt->counter = 0;
  timing = bufEnt->timing;
  spd = bufEnt->speed;
  bufEnt->speed = speed;
  latestTiming += interval * bufEnt->speed;
  bufEnt->timing = latestTiming;
  //qDebug("slot %d timing %f", consumerSlot, latestTiming);
  i = bufEnt->frames;
  bufEnt->frames = 0;
  consumerSlot = (consumerSlot+1) % size;
  consumerRound = consumerSlot ? consumerRound : !consumerRound;
  bufEnt->bufferEmpty.wakeAll();
  bufEnt->mutex.unlock();
  regSem.release();
  return i;
}

void
SndBuffer::forceWakeAll()
{
  //qDebug("forceWakeAll ---------------");
  player_called_forceWakeAll = true;
  for (int i=0; i<size; ++i) {
    buffers[i]->mutex.lock();
    buffers[i]->bufferEmpty.wakeAll();
    buffers[i]->bufferReady.wakeAll();
    buffers[i]->mutex.unlock();
  }
  //qDebug("forceWakeAll --------------- done");
}

}
