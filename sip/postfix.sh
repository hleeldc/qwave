sed -i -e 's/unsigned  *long  *bytesPlayed/unsigned long long bytesPlayed/' ../python/sipqwave2SndPlayer.h
sed -i -e 's/unsigned long sipSndPlayer::bytesPlayed/unsigned long long sipSndPlayer::bytesPlayed/' ../python/sipqwave2SndPlayer.cpp
sed -i -e 's#^\(srcdir *=.*\)/sip *$#\1/python#' ../python/Makefile
