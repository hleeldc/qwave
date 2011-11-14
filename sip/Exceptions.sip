class SoundFileOpenError
{
%TypeHeaderCode
#include "QWave4/Exceptions.h"
using namespace QWave4;
%End


public:
  SoundFileOpenError(const QString&);
  const QString& what();
};


class AudioDeviceError
{
%TypeHeaderCode
#include "QWave4/Exceptions.h"
using namespace QWave4;
%End


public:
  AudioDeviceError(const QString&);
  const QString& what();
};


class SampleRateConverterError
{
%TypeHeaderCode
#include "QWave4/Exceptions.h"
using namespace QWave4;
%End


public:
  SampleRateConverterError(const QString&);
  const QString& what();
};


