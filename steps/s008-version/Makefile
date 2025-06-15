CFLAGS += -g
LDFLAGS +=

all: build
build: xt_tee
xt_tee: xt_tee.c
	$(CC) $(CFLAGS) -o $@ xt_tee.c $(LDFLAGS)

clean:
	rm -rf xt_tee

rebuild: clean build
