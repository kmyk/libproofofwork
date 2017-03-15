.PHONY: build

CC ?= clang
CCFLAGS += -std=c99 -Wall -O3 -mtune=native -march=native
PYTHON ?= python3

all: proofofwork openssl pyhashlib

proofofwork:
	${CC} ${CCFLAGS} -fopenmp -I../include ../src/md5-avx2.c proofofwork.c
	${PYTHON} report.py ./a.out

openssl:
	${CC} ${CCFLAGS} -lcrypto openssl.c
	${PYTHON} report.py ./a.out

pyhashlib:
	${PYTHON} report.py ${PYTHON} pyhashlib.py
