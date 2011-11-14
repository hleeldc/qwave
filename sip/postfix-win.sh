sed -e 's/unsigned  *long  *bytesPlayed/__int64 bytesPlayed/' ../python/sipqwave2SndPlayer.h > x.x.x
mv x.x.x ../python/sipqwave2SndPlayer.h
sed -e 's/unsigned long sipSndPlayer::bytesPlayed/__int64 sipSndPlayer::bytesPlayed/' ../python/sipqwave2SndPlayer.cpp > x.x.x
mv x.x.x ../python/sipqwave2SndPlayer.cpp
#sed -i -e 's#^\(srcdir *=.*\)/sip *$#\1/python#' ../python/Makefile > x.x.x
#mv x.x.x ../python/Makefile
