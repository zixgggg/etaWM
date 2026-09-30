CC=gcc
LIB=X11
BIN=etawm
etawm:main.c 
	${CC} main.c -o ${BIN} -l${LIB}
