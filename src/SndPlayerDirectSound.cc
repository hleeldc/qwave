#include <QWave4/SndPlayerDirectSound.h>
#include <qmessagebox.h>
#include <sstream>
#include <iostream>
using namespace std;

namespace QWave4 {

SndPlayerDirectSound::SndPlayerDirectSound(
		HWND wid,
		int channels,
		int samplerate,
		int resampleQuality,
		int numBuffer)
  : SndPlayer(channels, samplerate, resampleQuality, numBuffer),
    winId(wid),
    wrt_pos(0),
    prev_play_cursor(0),
    ploop(0),
    wloop(0),
    playing(false)
{
}

SndPlayerDirectSound::~SndPlayerDirectSound()
{
  closeDevice();
}


bool
SndPlayerDirectSound::isDevicePlaying()
{
  return playing;
}


void
SndPlayerDirectSound::openDevice()
{
	frm_bytes = outChannels * sizeof(short);
	frag_bytes = 256;
	buf_bytes = 8 * frag_bytes;

	WAVEFORMATEX wfx;
	DSBUFFERDESC dsbdesc;

	// create DirectSound
	hr = DirectSoundCreate8(NULL, &pDs, NULL);
	if (FAILED(hr)) {
		stringstream s;
		s << "DirectSoundCreate8 error: 0x" << hex << hr;
		throw AudioDeviceError(s.str().c_str());
	}

	// set cooperation level
	hr = pDs->SetCooperativeLevel(winId, DSSCL_PRIORITY);
	if (FAILED(hr)) {
		stringstream s;
		s << "IDirectSound8::SetCooperativeLevel error: 0x" << hex << hr;
		throw AudioDeviceError(s.str().c_str());
	}

	// Set up WAV format structure. 
	memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
	wfx.wFormatTag = WAVE_FORMAT_PCM; 
	wfx.nChannels = outChannels; 
	wfx.nSamplesPerSec = outSampleRate; 
	wfx.wBitsPerSample = sizeof(short)*8;  // sample size in bits (short)
	wfx.nBlockAlign = frm_bytes;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 

	// Set up DSBUFFERDESC structure. 
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
	dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
	dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS;
	dsbdesc.dwBufferBytes = buf_bytes;
	dsbdesc.lpwfxFormat = &wfx;

	// Create buffer.
	hr = pDs->CreateSoundBuffer(&dsbdesc, &pDsb, NULL);
	if (FAILED(hr)) {
		stringstream s;
		s << "IDirectSound8::CreateSoundBuffer error: 0x" << hex << hr;
		throw AudioDeviceError(s.str().c_str());
	}

}

void
SndPlayerDirectSound::closeDevice()
{
}

unsigned
SndPlayerDirectSound::getBufferSize()
{
  return buf_bytes;
}

unsigned
SndPlayerDirectSound::getBufferFragmentSize()
{
  return frag_bytes;
}

DWORD
SndPlayerDirectSound::getPlayerCursorPos()
{
	DWORD pc;
	mutex.lock();
	hr = pDsb->GetCurrentPosition(&pc,NULL);
	if (pc < prev_play_cursor) ++ploop;
	prev_play_cursor = pc;
	mutex.unlock();
	return pc;
}

unsigned __int64
SndPlayerDirectSound::bytesPlayed()
{
	DWORD pc = getPlayerCursorPos();
	//qDebug("--> %d %12.5f", pc + ploop*buf_bytes, (double)(pc + ploop*buf_bytes)/frm_bytes/outSampleRate);
	return pc + ploop * buf_bytes;
}

void
SndPlayerDirectSound::stopDevice()
{
	hr = pDsb->Stop();
	playing = false;
	ploop = 0;
	wloop = 0;
	wrt_pos = 0;
	prev_play_cursor = 0;
	hr = pDsb->SetCurrentPosition(wrt_pos);
}

int
SndPlayerDirectSound::writeDevice(short* buf, int n)
{
	LPVOID ptr1, ptr2;
	DWORD bytes1, bytes2;
	DWORD cap, bytes;
	DWORD pc;

	bytes = n;

	pc = getPlayerCursorPos();
	cap = (ploop-wloop+1) * buf_bytes + pc - wrt_pos;
	//qDebug("pc: %d  wc: %d  cap: %d", pc, wrt_pos, cap);
	while (cap < (DWORD) n) {
		Sleep(10);   // FIXME: is this short enough?
		pc = getPlayerCursorPos();
		cap = (ploop-wloop+1) * buf_bytes + pc - wrt_pos;
		//qDebug("pc: %d  wc: %d  cap: %d", pc, wrt_pos, cap);
	}
	bytes = min(cap, (DWORD) n);

	hr = pDsb->Lock(wrt_pos, bytes, &ptr1, &bytes1, &ptr2, &bytes2, 0);
	CopyMemory(ptr1, buf, bytes1);
	if (ptr2 != NULL)
		CopyMemory(ptr2, &buf[bytes1/sizeof(short)], bytes2);

	hr = pDsb->Unlock(ptr1, bytes1, ptr2, bytes2);
	//if (FAILED(hr))
	//	qDebug("Unlock failed");

	wrt_pos += bytes1 + bytes2;

	if (!playing)
	     //&& wrt_pos > min(outSampleRate/10*frm_bytes, buf_bytes-frag_bytes))
	{
		hr = pDsb->Play(0,0,DSBPLAY_LOOPING);
		playing = true;
	}

	if (wrt_pos >= buf_bytes) {
		wrt_pos %= buf_bytes;
		wloop += 1;
	}

	return bytes1 + bytes2;
}

}  // end of namespace QWave4

