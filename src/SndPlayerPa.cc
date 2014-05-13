#include "SndPlayerPa.h"

using std::string;

static void free_buf_cb(void* buf) {

}

static void stream_success_cb(pa_stream* s, int success, void* userdata) {
}

static void stream_state_cb(pa_stream* s, void* userdata)
{
    switch (pa_stream_get_state(s)) {
    case PA_STREAM_READY:
        qDebug("stream ready");
        pa_stream_update_timing_info(s, stream_success_cb, NULL);
        break;
    default:
        break;
    }
}

static void stream_write_cb(pa_stream* s, size_t n, void* userdata)
{
    struct session_t* sess = (struct session_t*) userdata;
    qDebug("write data %d", n);
    sess->capacity = n;

}

static void stream_underflow_cb(pa_stream* s, void* userdata) {
    qDebug("underrun");
}

static void state_cb(pa_context* c, void* userdata)
{
    struct session_t* sess = (struct session_t*) userdata;
    pa_context_state_t state = pa_context_get_state(c);
    switch (state) {
    case PA_CONTEXT_READY:
        sess->stream = pa_stream_new(c, "xtrans_playback", &sess->sample_spec, NULL);
        if (!sess->stream) {
            qDebug("pa_stream_new failed");
        }
        pa_stream_set_state_callback(sess->stream, stream_state_cb, NULL);
        pa_stream_set_write_callback(sess->stream, stream_write_cb, sess);
        pa_stream_set_underflow_callback(sess->stream, stream_underflow_cb, NULL);

        pa_buffer_attr buffer_attr;
        memset(&buffer_attr, 0, sizeof(buffer_attr));
        buffer_attr.maxlength = (uint32_t) -1;
        buffer_attr.prebuf = (uint32_t) -1;
        buffer_attr.fragsize = buffer_attr.tlength = (uint32_t) -1;
        buffer_attr.minreq = (uint32_t) -1;
        pa_stream_flags_t flags = (pa_stream_flags_t) 0;

        if (pa_stream_connect_playback(sess->stream, NULL, &buffer_attr, flags, NULL, NULL) < 0) {
            qDebug("pa_stream_connect_playback failed");
        }

        pa_stream_trigger(sess->stream, stream_success_cb, NULL);
        break;
    }
}

namespace QWave4 {

    SndPlayerPa::SndPlayerPa(
        int channels,
        int samplerate,
        int resampleQuality,
        int numBuffer
        )
        : SndPlayer(channels, samplerate, resampleQuality, numBuffer),
          mainloop_(0),
          api_(0),
          context_(0),
          buf_bytes_(4096 * 4),
          frag_bytes_(4096),
          counter_(0),
          playing_(false)
    {
        sess_.sample_spec.channels = channels;
        sess_.sample_spec.rate = samplerate;
        sess_.sample_spec.format = PA_SAMPLE_S16LE;
        sess_.capacity = 0;
        sess_.stream = 0;
    }

    SndPlayerPa::~SndPlayerPa()
    {
        closeDevice();
    }

    bool SndPlayerPa::isDevicePlaying()
    {
        if (sess_.stream) {
            pa_threaded_mainloop_lock(mainloop_);
            const pa_timing_info* t = pa_stream_get_timing_info(sess_.stream);
            bool r = t->playing != 0;
            pa_threaded_mainloop_unlock(mainloop_);
            return r || playing_;
        }
        else {
            return false;
        }
    }

    void SndPlayerPa::openDevice() throw (AudioDeviceError)
    {
        mainloop_ = pa_threaded_mainloop_new();
        api_ = pa_threaded_mainloop_get_api(mainloop_);
        context_ = pa_context_new(api_, "xtrans");
        pa_context_connect(context_, NULL, (pa_context_flags_t) 0, NULL);
        pa_context_set_state_callback(context_, state_cb, &sess_);
        pa_threaded_mainloop_start(mainloop_);
    }

    void SndPlayerPa::closeDevice()
    {
        stopDevice();
        pa_threaded_mainloop_stop(mainloop_);
        pa_threaded_mainloop_free(mainloop_);
    }

    unsigned SndPlayerPa::getBufferSize()
    {
        return buf_bytes_;
    }

    unsigned SndPlayerPa::getBufferFragmentSize()
    {
        return frag_bytes_;
    }

    unsigned long long SndPlayerPa::bytesPlayed()
    {
        pa_threaded_mainloop_lock(mainloop_);
        const pa_timing_info* t = pa_stream_get_timing_info(sess_.stream);
        unsigned long long m = t->write_index - counter_;
        pa_threaded_mainloop_unlock(mainloop_);
        return m;
    }

    void SndPlayerPa::stopDevice()
    {
        qDebug("stop");
        pa_threaded_mainloop_lock(mainloop_);
        pa_stream_cork(sess_.stream, 1, stream_success_cb, NULL);
        const pa_timing_info* t = pa_stream_get_timing_info(sess_.stream);
        counter_ = t->write_index;
        pa_threaded_mainloop_unlock(mainloop_);
        playing_ = false;
    }

    int SndPlayerPa::writeDevice(short *buf, int n)
    {
        if (sess_.capacity > 0) {
            playing_ = true;
            int m = min(sess_.capacity, n);
            pa_threaded_mainloop_lock(mainloop_);
            pa_stream_write(sess_.stream, buf, m, free_buf_cb, 0, PA_SEEK_RELATIVE);
            pa_stream_cork(sess_.stream, 0, stream_success_cb, NULL);
            pa_stream_trigger(sess_.stream, stream_success_cb, NULL);
            pa_threaded_mainloop_unlock(mainloop_);
            return m;
        }
        else {
            return 0;
        }
    }
}
