#ifndef _SndDataThread_h_
#define _SndDataThread_h_

#include <qthread.h>
#include <sndfile.h>
#include <samplerate.h>
#include <vector>
using namespace std;

namespace QWave4 {

class SndPlayer;
class SndFile;
class SndBuffer;

struct ChannelInfo {
  int outchannel; // -1:all
  double weight;  // used when mixing streams (0:mute)
};

class SndDataThread: public QThread
{
  friend class SndPlayer;

public:
  ~SndDataThread();
  vector<ChannelInfo>* getChannelInfo();
  float* getResultBuffer();
  void seek(double t);
  void seek2(double t1, double t2);

  virtual void run();

protected:
  SndDataThread(SndPlayer* player, SndFile* sndfile, double offset=0.0);
  vector<ChannelInfo> channelInfo;
  
private:
  SNDFILE* _sndfile;
  SF_INFO _sfinfo;
  SRC_STATE* _src_state;
  SRC_DATA _src_data;
  SndPlayer* _player;
  SndBuffer* _outBuffer;

  float* _buf1;
  float* _buf2;
  double _inOutRatio;
  
  double _seekTime;
  int _seekFrame;
  double _offset;
};


}

#endif
