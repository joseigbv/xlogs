CC = gcc
INC = -I/opt/tommyds-1.2
LIB = -L/opt/tommyds-1.2
CFLAGS = -Wall -O2 ${INC} ${LIB} -ggdb -lz
OBJS = idx.o common.o sbuf.o sets.o th.o res.o tommy.o
HEADERS = idx.h common.h sbuf.h sets.h th.h res.h
BINS = parse query 

all: ${BINS}

parse: ${OBJS} ${HEADERS} parse.o
		${CC} parse.c -o parse ${CFLAGS} ${OBJS}

query: ${OBJS} ${HEADERS} query.o
		${CC} query.c -o query -lpthread ${CFLAGS} ${OBJS}

clean:
	rm -f ${OBJS} ${BINS}

