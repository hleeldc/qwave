#! /bin/sh

list="\
Exceptions \
Events \
SndFile \
SndPlayer \
SndPlayerTicker \
TimeLabel \
Utils \
Waveform \
WaveformBar \
WaveformCursorProxy \
WaveformRegion \
WaveformRuler \
WaveformScrollBar \
WaveformSelectionProxy \
WaveformVRuler"

for f in $list ; do
    echo $f.sip
    perl h2sip.pl < ../src/QWave2/$f.h > $f.sip
done


perl fix1.pl < Exceptions.sip > tmp.sip
mv tmp.sip Exceptions.sip
#perl fix2.pl < QWave.sip > tmp.sip
#mv tmp.sip QWave.sip
