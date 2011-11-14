#ifndef _SndPlayerDirectSound_h_
#define _SndPlayerDirectSound_h_

#include <qmutex.h>
#include <QWave4/SndPlayer.h>
#include <QWave4/Exceptions.h>
#include <dsound.h>

namespace QWave4 {

class DLLEXPORT SndPlayerDirectSound: public SndPlayer
{
public:
  SndPlayerDirectSound(HWND wid,
		       int channels=2,
		       int samplerate=8000,
		       int resampleQuality=0,
		       int numBuffer=2);

  virtual ~SndPlayerDirectSound();
  
  virtual bool isDevicePlaying();

private:
	unsigned frm_bytes;    // size of one frame in bytes
	unsigned frag_bytes;   // size of one fragment in bytes
	unsigned buf_bytes;    // size of the buffer in bytes

	HWND winId;
	HRESULT hr;
	LPDIRECTSOUND8 pDs;
	LPDIRECTSOUNDBUFFER pDsb;
	
	DWORD wrt_pos;
	DWORD prev_play_cursor;
	int ploop;
	int wloop;
	bool playing;

	DWORD getPlayerCursorPos();

	QMutex mutex;

  void openDevice();
  void closeDevice();
  unsigned getBufferSize();
  unsigned getBufferFragmentSize();
  unsigned __int64 bytesPlayed();
  void stopDevice();
  int writeDevice(short*,int);
  
};

typedef SndPlayerDirectSound PLAYERIMPLEMENTATION;

}

#endif
