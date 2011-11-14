class TimeLabel: QLabel
{
%TypeHeaderCode
#include "QWave4/TimeLabel.h"
#include "QWave4/Utils.h"
using namespace QWave4;
%End

public:
  TimeLabel(QWidget* /TransferThis/ =0);
public slots:
  void setTime(double);
protected:
  void customEvent(QEvent*);
};


