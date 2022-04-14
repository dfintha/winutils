BINARIES= \
	bin/killall.exe

.PHONY: all clean

all: $(BINARIES)
	@printf "Success!\n"

clean:
	@rm -rf bin
	@printf "Success!\n"

bin/%.exe: src/%.c
	@printf "Compiling $@\n"
	@mkdir -p bin
	@gcc -Wall -Wextra -Wpedantic -std=c89 -O2 -s $< -o $@
