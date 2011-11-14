#include "QWave4/SndFile.h"
#include "QWave4/Waveform.h"
#include "QWave4/Exceptions.h"
#include <limits.h>
#include <cmath>

namespace QWave4 {

SndFile::SndFile(char const * filename, int numCPages)
  : _filename(filename),
    _numCPages(numCPages<=0 ? 120: numCPages)
{
  _sndfile = sf_open(filename, SFM_READ, &_sfinfo);
  if (!_sndfile)
    throw SoundFileOpenError(QString("can't open '")+filename+"'");
  
  _cache = new short*[_numCPages];
  for (size_t i=0; i<_numCPages; ++i) {
    _cache[i] = new short[_sfinfo.channels*_sfinfo.samplerate];
    _unusedCPages.push_back(i);
  }
}

SndFile::SndFile()
{}

SndFile::~SndFile()
{
  if (!_filename.empty()) {
    for (size_t i=0; i<_numCPages; ++i)
      delete[] _cache[i];
    delete[] _cache;

    sf_close(_sndfile);
  }
}

void
SndFile::drawWaveform(Waveform* wave,
		      const int& ch,
		      const double& beg,
		      const double& dur)
{
  if (wave->getPaintDevice()->height() == 0 ||
      wave->getPaintDevice()->width() == 0)
      return;

  int s = (int)nearbyint(beg * _sfinfo.samplerate);     // first sample
  int e = (int)nearbyint((beg+dur)*_sfinfo.samplerate); // sample limit

  if (ch < 0 or
      ch >= _sfinfo.channels or
      e <= 0 or
      s >= _sfinfo.frames)
    return;

  int s1 = (s < 0) ? 0 : s;
  int e1 = (e > _sfinfo.frames) ? _sfinfo.frames : e;

  if (s1 >= e1)
    return;

  int k1 = s1 / _sfinfo.samplerate;         // first page
  int k2 = (int)ceil((double)e1/_sfinfo.samplerate); // page limit
  if (k2 - k1 > (int) _numCPages) {
    k2 = k1 + _numCPages;
    e1 = k2 * _sfinfo.samplerate;
  }

  int k, i;
  int m = k2-k1;    // number of pages we need
  #ifdef _MSC_VER
  int cpages[MAXCACHEPAGES];
  int starts[MAXCACHEPAGES];
  int ends[MAXCACHEPAGES];
  #else
  int cpages[m];
  int starts[m];    // index into frames within a cpage
  int ends[m];      // index into frames wrt the whole signal
  #endif
  int upage;
  list<int>::iterator pos;

  for (i=0, k=k1; k<k2; ++i, ++k) {
    if (_index.find(k)==_index.end()) {
      // cache miss!

      // find a cache page to load page k
      if (_index.size() >= _numCPages) {
	// cache is full; an old cpage should be gone
	pos = _heap.begin();
        while (*pos>=k1 && *pos<k2) ++pos;
	upage = _index[*pos];
        _index.erase(*pos);
        _heap.erase(pos);
      }
      else {
	upage = _unusedCPages.back();
	_unusedCPages.pop_back();
      }

      // load page k
      sf_seek(_sndfile, k*_sfinfo.samplerate, SEEK_SET);
      sf_readf_short(_sndfile, _cache[upage], _sfinfo.samplerate);
      _index[k] = upage;
      _heap.push_back(k);
    }
    else {
      upage = _index[k];
    }

    starts[i] = 0;
    ends[i] = (k+1) * _sfinfo.samplerate;
    cpages[i] = upage;
  }

  starts[0] = s1 % _sfinfo.samplerate;
  ends[m-1] = e1;

  // draw
  int f = s1;
  short *p = _cache[cpages[0]] + starts[0]*_sfinfo.channels + ch;
  QPainter painter(wave->getPaintDevice());
  double r = wave->getPixelsPerFrame();
  double h = wave->getAmplitudeRatio() * wave->getHeightPixels() / 2.0 / SHRT_MAX;
  int center = wave->getHeightPixels() / 2;

  double pps = wave->getPixelsPerSecond();
  double spp = wave->getSecondsPerPixel();
  double t0 = wave->getBeginSeconds();

  //qDebug("%f", r);
  if (r<1.0) {
    int x;
    if (s != s1)
      x = (int)nearbyint((s1 / _sfinfo.samplerate - t0) * pps);
    else
      x = (int)nearbyint((beg - t0) / spp);
    double t1 = (x+1) * spp + t0;
    int f1 = (int)nearbyint(t1 * _sfinfo.samplerate);
    short min, max, y0=0;
    bool new_pixel = false;
    min = max = *p;

    for (k=0; k<m; ++k) {
      p = _cache[cpages[k]] + starts[k]*_sfinfo.channels + ch;
      while (f1 <= ends[k]) {
	if (new_pixel) {
	  new_pixel = false;
	  if (*p < min or *p > max)
	    painter.drawLine(x,(int)nearbyint(center-h*y0),x,(int)nearbyint(center-h*(*p)));
	  min = max = *p;
	}
	for (; f<f1; ++f, p+=_sfinfo.channels) {
	  if (min > *p)
	    min = *p;
	  else if (max < *p)
	    max = *p;
	}
	// draw line here!
	painter.drawLine(x,(int)nearbyint(center-h*min),x,(int)nearbyint(center-h*max));
	++x;
	new_pixel = true;
	t1 += spp;
	f1 = (int)nearbyint(t1 * _sfinfo.samplerate);
	y0 = *(p-_sfinfo.channels);
      } // end of while
      if (f < ends[k]) {
	new_pixel = false;
	if (*p < min or *p > max)
	  painter.drawLine(x,(int)nearbyint(center-h*y0),x,(int)nearbyint(center-h*(*p)));
	min = max = *p;
	for (; f<ends[k]; ++f, p+=_sfinfo.channels) {
	  if (min > *p)
	    min = *p;
	  else if (max < *p)
	    max = *p;
	}
      } // end of if
    }   // end of outermost for
    if (!new_pixel)
      painter.drawLine(x,(int)nearbyint(center-h*min),x,(int)nearbyint(center-h*max));
  }
  else {
    //double x0 = wave->getBeginPixels();
    double x1;
    int x, y0, y;
    y0 = center - (int)(*p * h);
    for (k=0; k<m; ++k) {
      p = _cache[cpages[k]] + starts[k]*_sfinfo.channels + ch;
      for (; f<ends[k]; ++f, p+=_sfinfo.channels) {
	x1 = ((double)f/_sfinfo.samplerate - t0) * pps;
	x = (int)trunc(x1);
	y = center - (int)nearbyint(*p * h);
	painter.drawLine(x,y0,x,y);
	painter.drawLine(x,y,(int)trunc(x1+r),y);
	y0 = y;
      }
    }
  }
}

int
SndFile::getChannels()
{
  return _sfinfo.channels;
}

int
SndFile::getSampleRate()
{
  return _sfinfo.samplerate;
}

/*
unsigned long long
SndFile::getFrames()
{
  return _sfinfo.frames;
}
*/

double
SndFile::getLengthSeconds()
{
  return (double)_sfinfo.frames/_sfinfo.samplerate;
}

char const*
SndFile::getFileName()
{
  return _filename.c_str();
}

}
