#! /bin/sh
$EXTRACTRC `find part -name \*.ui -o -name \*.rc` >> rc.cpp
$XGETTEXT rc.cpp *.cpp `find part -name \*.cpp -o -name \*.cpp -o -name \*.h` -o $podir/words.pot

