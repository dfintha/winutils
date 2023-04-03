BINARIES= \
	bin/killall.exe \
	bin/forceshow.exe \
	bin/adminrun.exe \
	bin/mdicapture.exe

CC=x86_64-w64-mingw32-gcc
RC=windres
LD=x86_64-w64-mingw32-gcc
CFLAGS=-Wall -Wextra -Wpedantic -std=c89 -O2
LDFLAGS=-lshell32 -lkernel32 -luser32 -lgdi32 -lcomctl32 -lcomdlg32

.PHONY: all clean

all: $(BINARIES)
	@printf "Success!\n"

clean:
	@rm -rf bin obj
	@printf "Success!\n"

obj/common.o: res/common.rc
	@printf "Compiling common manifest\n"
	@mkdir -p obj
	@$(RC) -i res/common.rc -o obj/common.o

obj/%.o: src/%.c
	@printf "Compiling $@\n"
	@mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

bin/%.exe: obj/%.o obj/common.o
	@printf "Compiling $@\n"
	@mkdir -p bin
	@$(LD) $< obj/common.o -o $@ -s $(LDFLAGS)

bin/mdicapture.exe: obj/mdicapture.o obj/common.o
	@printf "Compiling $@\n"
	@mkdir -p bin
	@$(LD) $< obj/common.o -o $@ -s $(LDFLAGS) -Wl,--subsystem,windows
