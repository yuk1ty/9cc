CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)

$(OBJS): 9cc.h

compile:
	gcc -o 9cc $(SRCS) -Wall -std=c11

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: docker-build
docker-build:
	docker build -t 9cc .

.PHONY: docker-run
docker-run:
	docker container run -it 9cc
