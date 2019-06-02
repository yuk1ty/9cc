9cc: 9cc.c

compile:
	gcc -o 9cc 9cc.c

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
