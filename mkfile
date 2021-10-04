</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=city
OFILES=\
	city.$O\
	defs.$O\
	drw.$O\
	fs.$O\
	map.$O\
	sim.$O\

HFILES=dat.h fns.h
</sys/src/cmd/mkone

sysinstall:V:
	mkdir -p /sys/games/lib/$TARG
	dircp $TARG /sys/games/lib/$TARG
