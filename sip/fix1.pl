#! /usr/bin/env perl
#
# fix QWaveExceptions.sip
#

while (<>) {
    if (/%TypeHeaderCode/) {
        while (<>) {
            last if /%End/;
        }
print "%TypeHeaderCode\n";
print "#include \"QWave2/Exceptions.h\"\n";
print "using namespace QWave2;";
print "%End\n\n";
        next;

    }
    print;
}
