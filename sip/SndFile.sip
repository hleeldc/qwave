class SndFile
{
%TypeHeaderCode
#include "QWave4/SndFile.h"
using namespace QWave4;
%End

public:
  SndFile(const char* , int = 120);
  virtual void drawWaveform(Waveform*,const int&,const double&,const double&);
  virtual int getChannels();
  virtual int getSampleRate();
  virtual double getLengthSeconds();
  virtual const char* getFileName();

protected:
  SndFile();

};


