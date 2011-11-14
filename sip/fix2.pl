#! /usr/bin/env perl
#
# fix QWave.sip
#

while (<>) {
    if (/%HeaderCode/) {
        while (<>) {
            last if /%End/;
        }
        next;
print "%HeaderCode\n";
print "#include \"QWave.h\"\n";
print "%End\n\n";

    }
    if (/QWaveSndfile/) {
        while (<>) {
            last if /};/;
        }
        next;
    }
    if (/QWaveform/) {
        while (<>) {
            last if /};/;
        }
        next;
    }
    print unless /^\s*$/;
}
