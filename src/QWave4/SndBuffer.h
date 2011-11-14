#ifndef _SndBuffer_h_
#define _SndBuffer_h_

#include <qmutex.h>
#include <qwaitcondition.h>
#include <qsemaphore.h>
#include <vector>
#include <map>
using namespace std;

namespace QWave4 {

struct ChannelInfo;
class SndDataThread;
class SndPlayer;

struct SndBufferDTRegEnt
{
  vector<ChannelInfo>* channelInfo;
  float* buf;
  int channels;
  int slot;
  bool round;
};

// buffer frame
class SndBufferEnt
{
public:
  SndBufferEnt(int channels, int frames);
  ~SndBufferEnt();

  void initialize();

  int counter;  // number of mixes
  int frames;   // # of written frames
  double *data;
  double speed;
  double timing;
  vector<double> weight;
  QMutex mutex;
  QWaitCondition bufferReady;
  QWaitCondition bufferEmpty;

private:
  int _channels;
  int _frames;      // size of data
};

// assume N producer threads and 1 consumer thread
class SndBuffer
{
public:
  SndBuffer(SndPlayer* player,
	    int channels,
	    int frames,
	    double interval,
	    int size,
	    double& speed);
  ~SndBuffer();

  void initialize();

  void registerProducer(SndDataThread* t);
  void unregisterProducer(SndDataThread* t);
  void getSpeedAndTiming(SndDataThread* t, double& spd, double& timing);
  void mix(SndDataThread* t, int n);
  int produce(short* data, double& timing, double& spd, double limit = 0.0);

  /**
   * Wakes up any pending data/player threads.
   * It's important that this method be called after SndPlayer::stopSignaled
   * or SndPlayer::stopSignaled is set to true.  Otherwise, it has an
   * unexpected effects.
   */
  void forceWakeAll();
  void wakeUpDataThreads();

private:
  SndPlayer *player;
  int channels;
  int size;
  double interval;
  double& speed;
  bool player_called_forceWakeAll;
  double latestTiming;
  vector<SndBufferEnt*> buffers;
  int consumerSlot;
  bool consumerRound;
  QSemaphore regSem;
  int numDt;
  map<SndDataThread*,SndBufferDTRegEnt> dtReg;
};

}

#endif
