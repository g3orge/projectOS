# Pizza server project makefile 2012-2013
# `make` to compile all
# `make test` to run tests
# `make clean` to clean

# Options
COMP=gcc
WARN=-w
FLAGS=-pthread

all: server client
	echo > logfile

server: pizza_server.c pizza.h
	$(COMP) $(WARN) pizza_server.c -o server $(FLAGS)

client: client.c pizza.h
	$(COMP) $(WARN) client.c -o client

clean:
	-@rm server a.out client 2>/dev/null || true

test:
	# Running run_tests.sh
	./run_tests.sh

kill:
	-@echo "Number of server processes: (correct: 2)"
	-@pgrep server | wc -l
	killall server
