%Module qwave4

%Import QtGui/QtGuimod.sip

/*
%MappedType sf_count_t
{
%TypeHeaderCode
#include <sndfile.h>
%End

%ConvertFromTypeCode
    return PyLong_FromLongLong(*sipCpp);
%End

%ConvertToTypeCode
    *sipCppPtr = new sf_count_t(PyLong_AsLongLong(sipPy));
    return 1;
%End
};
*/

%Include Exceptions.sip
%Include Events.sip
%Include SndFile.sip
%Include SndPlayer.sip
%Include SndPlayerTicker.sip
%Include TimeLabel.sip
%Include Utils.sip
%Include Waveform.sip
%Include WaveformBar.sip
%Include WaveformCursorProxy.sip
%Include WaveformRegion.sip
%Include WaveformRuler.sip
%Include WaveformScrollBar.sip
%Include WaveformSelectionProxy.sip
%Include WaveformVRuler.sip
