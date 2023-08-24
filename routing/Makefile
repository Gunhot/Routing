CC=g++
CFLAGS=-Wall

all: distvec_20191616 linkstate_20191616

distvec_20200001: distvec_20191616.cc
	$(CC) $(CFLAGS) -o distvec_20200001 distvec_20191616.cc

linkstate_20200001: linkstate_20191616.cc
	$(CC) $(CFLAGS) -o linkstate_20200001 linkstate_20191616.cc

clean:
	rm -f distvec_20191616 linkstate_20191616

fclean :
	rm -f *.txt
