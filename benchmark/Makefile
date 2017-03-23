.PHONY: all

CC ?= clang
CCFLAGS += -std=c99 -Wall -O3 -mtune=native -march=native
PYTHON ?= python3

all: md5 sha1
md5: md5/pyhashlib md5/openssl md5/proofofwork
sha1: sha1/pyhashlib sha1/openssl sha1/proofofwork

build:
	cmake ..
	make -C ..

md5/proofofwork: build
	${CC} ${CCFLAGS} -fopenmp -I../include proofofwork-md5.c ../libproofofwork.a
	${PYTHON} report.py ./a.out

md5/openssl:
	${CC} ${CCFLAGS} -lcrypto openssl-md5.c
	${PYTHON} report.py ./a.out

md5/pyhashlib:
	${PYTHON} report.py ${PYTHON} pyhashlib.py md5

sha1/proofofwork: build
	${CC} ${CCFLAGS} -fopenmp -I../include proofofwork-sha1.c ../libproofofwork.a
	${PYTHON} report.py ./a.out

sha1/openssl:
	${CC} ${CCFLAGS} -lcrypto openssl-sha1.c
	${PYTHON} report.py ./a.out

sha1/pyhashlib:
	${PYTHON} report.py ${PYTHON} pyhashlib.py sha1
