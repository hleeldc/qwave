#ifndef _SndPlayer_h_
#define _SndPlayer_h_

#include <QWave4/SndPlayerTicker.h>
#include <QWave4/Exceptions.h>
#include <QThread>
#include <cstddef>
#include <sndfile.h>
#include <map>
using namespace std;

namespace QWave4 {

class SndBuffer;
class SndFile;
class SndDataThread;

/// SndPlayer implements a audio player interface.
/**
   Some of the powerful features of SndPlayer are as follows:
   <ul>
   <li>Multi-file: load and play multiple files synchronously.</li>
   <li>Multi-format: support many different audio file formats (the ones that
   are supported by the libsndfile library).</li>
   <li>Multi-platform: currently supports OSS and DirectSound.</li>
   <li>Varispeed playback: playback speed can be changed within a range of
   x0.25 ~ x4.0 during the playback.</li>
   </ul>

   Note that this is an abstract class.  You will have to use a specific
   implementation that supports your platform.  For example, on Linux or
   FreeBSD, the SndPlayerOSS class can be used, and on Windows, the
   SndPlayerDirectSound can be used.

   <pre>
   SndPlayer* myPlayer = new SndPlayerOSS;
   SndFile sndfile("myfile.wav");
   myPlayer->initialize();
   myPlayer->addSndFile(&sndfile);
   myPlayer->play(0, sndfile.getLengthSeconds());
   myPlayer->closePlayer();
   </pre>
 */
class DLLEXPORT SndPlayer: public QThread
{
  friend class SndBuffer;
  friend class SndDataThread;
  friend class SndPlayerTicker;

public:
  /**
     maximum playback speed (realtime x 4.0)
  */
  static double const MAX_PLAYBACK_SPEED;
  /**
     minimum playback speed (realtime x 0.25)
  */
  static double const MIN_PLAYBACK_SPEED;

  /**
     @param channels Number of output channels.
     @param samplerate Output sampling rate.
     @param resampleQuality Quality of re-sampling (0-4).  0 gives the best
     quality but slowest.  4 results in the poorest quality but fastest.
     @param numBuffer Number of output sample buffers.  In most cases, the
     default value 2 is optimal.

     SndPlayer performs re-sampling in order to be able to play multiple
     files of different sampling rates.  By default, audio files are re-sampled
     at 8KHz.

     There are several re-sampling methods; the one that gives the
     best audio quality (0) requires most CPU power, and the one that gives the
     poorest audio quality (4) requires least CPU power.

     SndPlayer uses a number of buffers to synchronize the multi-threaded
     re-sampling process.  In theory, the sound will be smoother if there
     are more buffers.  As a trade-off, however, the latency in audio control
     will be increased.  In most cases, 2 is enuough.
   */
  SndPlayer(int channels=2, int samplerate=8000, int resampleQuality=0, int numBuffer=2);
  virtual ~SndPlayer();

  /**
     Acquire audio device and initialize it and other parameters as well.
     This method must be called before the player is used at all.
   */
  void initialize() throw(AudioDeviceError);

  /**
     Releases resources used by the player.  This method must be called after
     the player has been used.
   */
  void closePlayer();

  /**
     @param sndfile An audio file.
     @param offset The audio file will be synchronized at the begining of the
     file by default.  However, if you want it to be synchronized at different
     locataion, use this parameter.

     Add an audio file to play.
   */
  void addSndFile(SndFile* sndfile, double offset=0.0);

  /**
     Remove an audio file from the player so that it can't be processed
     and played.
   */
  void removeSndFile(SndFile* sndfile);

  /**
     SndPlayer has a ticker that signals every 0.025 seconds.  This method
     enables the ticker.
   */
  void enableTicker();

  /**
     SndPlayer has a ticker that signals every 0.025 seconds.  This method
     disables the ticker.
   */
  void disableTicker();

  /**
     SndPlayer has a ticker that signals every 0.025 seconds.  This method
     returns the ticker.
   */
  SndPlayerTicker* getPlayerTicker();

  /**
     Specify a mapping from a channel of an audio file and to an output
     channel.
   */
  bool setOutputChannel(SndFile* sndfile, int channel, int outchannel);

  /**
     Each input channel is assigned a weight value 1.0 by default.  This
     method can be used to adjust the relative loudness of an input channel.
     The weight will be normalized before it is applied to the audio samples.
     
     weight_real = weight / sum(weights of all input channels mapped to the same output channel)

     Thus, the weight value itself doesn't determine the loudness of the
     input channel.  For example, if you have two input channels mapped into
     an output channel, setting their weights to 0.1 and 0.2 will have the
     same effect as setting them to 1.5 and 3.0.
   */
  bool setWeight(SndFile* sndfile, int channel, double weight);

  /**
     @param speed Playback speed expressed as a ratio to the normal speed.
     @return False if the channel doesn't exist on the sndfile.
     True othrewise.

     Set playback speed.  The value will be limited by
     MAX_PLAYBACK_SPEED and MIN_PLAYBACK_SPEED.
   */
  void setSpeed(double speed);

  /**
     Play from <b><i>beg</i></b> seconds for <b><i>dur</i></b> seconds.
   */
  void play(double beg, double dur);

  /**
     Repeat the region from <b><i>beg</i></b> to <b><i>beg</i></b> +
     <b><i>dur</i></b>.
   */
  void repeat(double beg, double dur);

  /**
     Pause the playback.
   */
  void pause();

  /**
     Resume the playback from the point it was paused.
   */
  void resume();

  /**
     Stop the playback and reset the playback cursor.
   */
  void stop();

  /**
     Returns where the player is playing.
   */
  double playerPosition();

  /**
     Returns how many seconds of audio samples the player has been playing.
   */
  double secondsPlayed();

  /**
     Returns the maximum, possible delay in playback.
   */
  double maxPlaybackDelay();

  /**
     Tells whether the audio device is still playing.
   */
  virtual bool isDevicePlaying() = 0;
protected:
  /**
     Open the audio device.
   */
  virtual void openDevice() throw (AudioDeviceError) = 0;
  /**
     Close the audio device.
   */
  virtual void closeDevice() = 0;
  /**
     Returns the number of bytes of the audio samples the player has been
     playing.
   */
  #ifdef _MSC_VER
  virtual unsigned __int64 bytesPlayed() = 0;
  #else
  virtual unsigned long long bytesPlayed() = 0;
  #endif
  /**
     Returns the size of the audio device buffer (bytes).
   */
  virtual unsigned getBufferSize() = 0;
  /**
     Returns the size of the audio device buffer fragment (bytes).
     Fragment is a unit of processing in SndPlayer.
   */
  virtual unsigned getBufferFragmentSize() = 0;
  /**
     Stop the audio device.
   */
  virtual void stopDevice() = 0;
  /**
     Write audio samples into the audio device.  This actually makes the
     device play the audio samples.
   */
  virtual int writeDevice(short*,int) = 0;
  /**
     Reimplementation of QThread::run().
   */
  virtual void run();

  /**
     The (software) output buffer.  Samples in this buffer are written
     to the device.
   */
  SndBuffer* outBuffer;
  /**
     The size of the audio device buffer.
   */
  int outBufferSize;  // in frames
  /**
     The recommended size of the software buffer, considering the re-sampling
     process.  Re-sampling often generates samples whose number often exceeds
     the size of the audio device buffer.  This is three times of the audio
     device buffer.
   */
  int outBufferSizeMax;
  /**
     The size (in frames) of a fragment of the audio device buffer.
   */
  int outBufferFragSize;
  /**
     The size (in bytes) of a fragment of the audio device buffer.
   */
  int outBufferFragBytes;
  /**
     The size (in bytes) of a frame of the audio device buffer.
   */
  int outBufferFrameBytes;
  /**
     The size (in seconds) of the audio device buffer.
   */
  double outBufferDuration;
  /**
     The number of output channels.  Default value is 2, but this can be
     changed in the constructor.
   */
  int outChannels;
  /**
     Output sampling rate.  Default value is 8KHz, but this can be changed
     in the constructor.
   */
  int outSampleRate;
  /**
     Quality of re-sampling. 0 - best/slowest, 4 - poorest/fastest.
   */
  int resampleQuality;
  /**
     Number of output buffers.
   */
  int numBuffer;

  // s=true && p=true  --> pause
  // s=true && p=false --> stop
  /**
     Indicates that the player is playing in "repeat" mode.
   */
  bool repeatedPlay;
  /**
     Indicates that "stop" has been requested.
   */
  bool stopSignaled;
  /**
     Indicates that "pause" has been requested.
   */
  bool pauseSignaled;
  /**
     The begin time of the current playback region.
   */
  double beginTime;
  /**
     The duration of the current playback region.
   */
  double playDuration;
  /**
     The location of pause.
   */
  double pausedTime;

private:

  void startDataThreads(double t);
  void stopDataThreads();

  double _speed;
  short* _finalBuffer;
  int _byterate;
  map<SndFile*,SndDataThread*> _dataThreads;

  QWaitCondition resumeSignal;
  QMutex resumeSignalMutex;
  QMutex mutex;

  map<double,pair<double,double>,greater<double> > t2spd;

  SndPlayerTicker _ticker;
};

}

#endif
