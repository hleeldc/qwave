#include "SndPlayerAlsa.h"

using std::string;
static const char* device = "default";


namespace QWave4 {

    static inline void check_err(int err, char const* msg)
    {
        if (err < 0) {
            throw AudioDeviceError(QString(msg) + ": " + snd_strerror(err));
        }
    }

    SndPlayerAlsa::SndPlayerAlsa(
        int channels,
        int samplerate,
        int resampleQuality,
        int numBuffer
        )
        : SndPlayer(channels, samplerate, resampleQuality, numBuffer),
          _counter(0),
          _handle(0)
    {
    }

    SndPlayerAlsa::~SndPlayerAlsa()
    {
        if (_handle) {
            snd_pcm_close(_handle);
        }
    }

    bool SndPlayerAlsa::isDevicePlaying()
    {
        return snd_pcm_state(_handle) == SND_PCM_STATE_RUNNING;
    }

    void SndPlayerAlsa::openDevice() throw (AudioDeviceError)
    {
        int err;
        int dir;
        unsigned int val;
        snd_pcm_uframes_t size;
        snd_pcm_hw_params_t* hwparams;
        snd_pcm_sw_params_t* swparams;

        snd_pcm_hw_params_alloca(&hwparams);
        snd_pcm_sw_params_alloca(&swparams);

        err = snd_pcm_open(&_handle, device, SND_PCM_STREAM_PLAYBACK, 0);
        check_err(err, "Can't open ALSA device for playback");

        err = snd_pcm_hw_params_any(_handle, hwparams);
        check_err(err, "Can't configure ALSA device (1)");

        err = snd_pcm_hw_params_set_access(_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        check_err(err, "Can't configure ALSA device (3)");

        err = snd_pcm_hw_params_set_format(_handle, hwparams, SND_PCM_FORMAT_S16);
        check_err(err, "Can't configure ALSA device (2)");

        val = outChannels;
        err = snd_pcm_hw_params_set_channels_near(_handle, hwparams, &val);
        check_err(err, "Can't configure ALSA device (4)");
        outChannels = val;

        val = outSampleRate;
        err = snd_pcm_hw_params_set_rate_near(_handle, hwparams, &val, 0);
        check_err(err, "Can't configure ALSA device (5)");
        outSampleRate = val;

        val = 50000;
        dir = -1;
        err = snd_pcm_hw_params_set_period_time_near(_handle, hwparams, &val, &dir);
        check_err(err, "Can't configure ALSA device (7)");

        snd_pcm_hw_params_get_period_size(hwparams, &size, &dir);
        _bufferPeriodBytes = size * outChannels * 2;

        size = size * 2;
        err = snd_pcm_hw_params_set_buffer_size_near(_handle, hwparams, &size);
        check_err(err, "Can't configure ALSA device (6)");
        _bufferBytes = size * outChannels * 2;

        err = snd_pcm_hw_params(_handle, hwparams);
        check_err(err, "Can't configure ALSA device (8)");

        err = snd_pcm_sw_params_current(_handle, swparams);
        check_err(err, "Can't obtain sw config container");

        //err = snd_pcm_sw_params_set_start_threshold(_handle, swparams, 1);
        //check_err(err, "Can't set start threshold");

        err = snd_pcm_sw_params(_handle, swparams);
        check_err(err, "Can't configure ALSA device (9)");

        err = snd_pcm_nonblock(_handle, 0);
        check_err(err, "Can't set block mode");

        qDebug("buffer size %d %d %d %d", outChannels, outSampleRate, _bufferBytes, _bufferPeriodBytes);
    }

    void SndPlayerAlsa::closeDevice()
    {
        snd_pcm_close(_handle);
        _handle = 0;
    }

    unsigned SndPlayerAlsa::getBufferSize()
    {
        return _bufferBytes;
    }

    unsigned SndPlayerAlsa::getBufferFragmentSize()
    {
        return _bufferPeriodBytes;
    }

    unsigned long long SndPlayerAlsa::bytesPlayed()
    {
        snd_pcm_sframes_t t = snd_pcm_avail(_handle);
        unsigned long long r = _bufferBytes - t * outChannels * 2;
        if (_counter <= r)
            return 0;
        else
            return _counter - r;
    }

    void SndPlayerAlsa::stopDevice()
    {
        QMutexLocker lock(&_mutex_counter);
        snd_pcm_drop(_handle);
        _counter = 0;
        snd_pcm_prepare(_handle);
    }

    int SndPlayerAlsa::writeDevice(short *buf, int n)
    {
        QMutexLocker lock(&_mutex_counter);
        snd_pcm_uframes_t f = n / outChannels / 2;
        snd_pcm_sframes_t k = snd_pcm_mmap_writei(_handle, buf, f);
        if (k < 0) {
            qDebug("write request %d", n);
            k = snd_pcm_recover(_handle, k, 0);
            return 0;
        }
        else {
            f = k * outChannels * 2;
            _counter += f;
            return f;
        }
    }
}
