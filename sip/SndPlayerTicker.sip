class SndPlayerTicker: QThread
{
%TypeHeaderCode
#include "QWave4/SndPlayerTicker.h"
using namespace QWave4;
%End

public:
  void registerReceiver(QObject*);
  void unregisterReceiver(QObject*);
  virtual void run();
  void resume();
  void stop();
private:
  SndPlayerTicker(SndPlayer*);
};

