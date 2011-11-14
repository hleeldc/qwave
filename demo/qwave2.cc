#include "qwave2.h"
#include <QApplication>
#include <QLabel>
#include <QSizePolicy>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QWave4/WaveformVRuler.h>
#ifdef _MSC_VER
#include <QWave4/SndPlayerDirectSound.h>
#else
#include <QWave4/SndPlayerAlsa.h>
#endif
#include <cmath>
#include <math.h>

using namespace QWave4;


MyWidget::MyWidget()
  : QWidget(),
	layout(new QVBoxLayout(this)),
#ifdef _MSC_VER
    player(new PLAYERIMPLEMENTATION(this->winId())),
#else
    player(new PLAYERIMPLEMENTATION()),
#endif
    cursor(new WaveformCursorProxy(this)),
    selection(new WaveformSelectionProxy(this)),
    paused(false),
    hasRuler(false),
    error_occurred(false)
{
    try {
        player->initialize();
    }
    catch (AudioDeviceError const& e) {
        QMessageBox::critical(
                    this,
                    "Error",
                    "Failed to open audio device.\n"
                    "Program will be closed.\n\n" + e.what()
                    );
        error_occurred = true;
        qApp->exit(1);
        return;
    }
    player->enableTicker();

  QHBoxLayout* l;

  l = new QHBoxLayout();
  l->addWidget(new QLabel("Audio File:", this));
  fileEntry = new QLineEdit(this);
  fileBrowseBtn = new QPushButton("Browse", this);
  fileAddBtn = new QPushButton("Add", this);
  l->addWidget(fileEntry);
  l->addWidget(fileBrowseBtn);
  l->addWidget(fileAddBtn);
  layout->addLayout(l);

  l = new QHBoxLayout();
  tm = new TimeLabel(this);
  tb = new TimeLabel(this);
  te = new TimeLabel(this);
  td = new TimeLabel(this);
  QLabel* spacer = new QLabel(this);
  spacer->setFixedHeight(0);
  spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  
  tm->setFrameStyle(QFrame::Box | QFrame::Sunken);
  tb->setFrameStyle(QFrame::Box | QFrame::Sunken);
  te->setFrameStyle(QFrame::Box | QFrame::Sunken);
  td->setFrameStyle(QFrame::Box | QFrame::Sunken);

  tm->setFixedSize(85,20);
  tb->setFixedSize(85,20);
  te->setFixedSize(85,20);
  td->setFixedSize(85,20);

  tm->setAlignment(Qt::AlignRight);
  tb->setAlignment(Qt::AlignRight);
  te->setAlignment(Qt::AlignRight);
  td->setAlignment(Qt::AlignRight);

  connect(selection, SIGNAL(waveformSelectionChanged(double,double,Waveform*)),
	  this, SLOT(changeSelection(double,double,Waveform*)));

  playBtn = new QPushButton(">", this);
  repeatBtn = new QPushButton("R", this);
  pauseBtn = new QPushButton("||", this);
  stopBtn = new QPushButton("X", this);

  speedSlider = new QSlider(this);
  speedSlider->setOrientation(Qt::Horizontal);
  speedSlider->setFixedWidth(60);
  speedSlider->setMinimum(-10);
  speedSlider->setMaximum(10);

  l->addWidget(tm);
  l->addWidget(tb);
  l->addWidget(te);
  l->addWidget(td);
  l->addWidget(spacer);
  l->addWidget(playBtn);
  l->addWidget(repeatBtn);
  l->addWidget(pauseBtn);
  l->addWidget(stopBtn);
  l->addWidget(speedSlider);
  layout->addLayout(l);

  connect(speedSlider, SIGNAL(valueChanged(int)),
	  this, SLOT(setSpeed(int)));

  playBtn->setFixedSize(20,20);
  repeatBtn->setFixedSize(20,20);
  pauseBtn->setFixedSize(20,20);
  stopBtn->setFixedSize(20,20);

  connect(playBtn, SIGNAL(clicked()), this, SLOT(play()));
  connect(repeatBtn, SIGNAL(clicked()), this, SLOT(repeat()));
  connect(pauseBtn, SIGNAL(clicked()), this, SLOT(pauseResume()));
  connect(stopBtn, SIGNAL(clicked()), this, SLOT(stop()));

  sb = new WaveformScrollBar(this);
  layout->addWidget(sb);

  player->getPlayerTicker()->registerReceiver(sb);
  player->getPlayerTicker()->registerReceiver(tm);
  player->getPlayerTicker()->registerReceiver(cursor);
  
  grid = new QGridLayout();
  grid->setSpacing(1);
  grid->addWidget(new QWidget(this), 0,0);
  ruler = new WaveformRuler(true, this);
  grid->addWidget(ruler,0,1);
  gridCurRow = 1;
  layout->addLayout(grid);

  connect(fileAddBtn, SIGNAL(clicked()), this, SLOT(addSndFile()));
  connect(fileEntry, SIGNAL(returnPressed()), this, SLOT(addSndFile()));
  connect(fileBrowseBtn, SIGNAL(clicked()), this, SLOT(browseSndFile()));
}

MyWidget::~MyWidget()
{
  for (size_t i=0; i<sndfiles.size(); ++i)
    delete sndfiles[i];
  if (! error_occurred)
    player->closePlayer();
  delete player;
}

void
MyWidget::addSndFile()
{
  QFileInfo finfo(fileEntry->text());
  if (!finfo.exists()) {
    QString msg("Can't find file: '%1'");
    QMessageBox::critical(this,"Error",msg.arg(finfo.filePath()));
    return;
  }
  SndFile* s = new SndFile(finfo.filePath().toStdString().c_str());
  player->pause();
  player->addSndFile(s);
  for (int ch=0; ch<s->getChannels(); ++ch, ++gridCurRow) {
    WaveformVRuler *r = new WaveformVRuler(this);
    Waveform* w = new Waveform(s, ch, 0.0, 10.0, this);

    QSlider* mag = new QSlider(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    QCheckBox* mutt = new QCheckBox(this);
    QSlider* vol = new QSlider(this);

        grid->addWidget(r, gridCurRow, 0);
        grid->addWidget(w, gridCurRow, 1);
        grid->addWidget(mag, gridCurRow, 2);
        grid->addLayout(vbox, gridCurRow, 3);

    mag->show();
    //vbox->show();

    waveforms[sndfiles.size()][ch] = w;
    r->connectToWaveform(w);
    r->show();
    w->show();

    cursor->registerWaveform(w);
    selection->registerWaveform(w);
    
    connect(w, SIGNAL(waveformMouseMoved(Waveform*,double)),
	    this, SLOT(setTime(Waveform*,double)));

    sb->registerWaveform(w);

    if (hasRuler == false) {
      ruler->connectToWaveform(w);
      ruler->show();
      hasRuler = true;
    }
  }
  //player->resume();
  sndfiles.push_back(s);

}

void
MyWidget::browseSndFile()
{
  QString s = QFileDialog::getOpenFileName(
          this,
          "Open Audio File",
          QString(),
          "WAVE (*.wav);;NIST SPHERE (*.sph);;All (*.*)"
          );
  if (s.length() > 0)
    fileEntry->setText(s);
}

void
MyWidget::play()
{
  paused = false;
  pauseBtn->setText("||");
  player->play(selection->getBeginSeconds(),
	       selection->getWidthSeconds());
}

void
MyWidget::repeat()
{
  paused = false;
  pauseBtn->setText("||");
  player->repeat(selection->getBeginSeconds(),
		 selection->getWidthSeconds());
}

void
MyWidget::pauseResume()
{
  if (paused) {
    paused = false;
    player->resume();
    pauseBtn->setText("||");
  }
  else {
    paused = true;
    player->pause();
    pauseBtn->setText("=");
  }
}

void
MyWidget::stop()
{
  paused = false;
  pauseBtn->setText("||");
  player->stop();
}

void
MyWidget::changeSelection(double beg, double dur, Waveform*)
{
  tb->setTime(beg);
  te->setTime(beg+dur);
  td->setTime(dur);
}

void
MyWidget::setSpeed(int v)
{
  player->setSpeed(pow(2.0,v/10.0));
}

void
MyWidget::setTime(Waveform*,double t)
{
  tm->setTime(t);
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  
  MyWidget w;

  w.show();

  if (w.has_error_occurred()) {
    app.exit(1);
    return 1;
  } else {
    return app.exec();
  }
}
