MAKEFLAGS+=-s  # silent mode, disables command output

BINARIES= \
	bin/killall.exe \
	bin/forceshow.exe \
	bin/adminrun.exe \
	bin/mdicapture.exe \
	bin/modules.exe \
	bin/dbglist.exe \
	bin/errcode.exe \
	bin/topology.exe

CC=x86_64-w64-mingw32-gcc
RC=x86_64-w64-mingw32-windres
LD=x86_64-w64-mingw32-gcc

CFLAGS=-Wall -Wextra -Wpedantic -std=c89 -O2
LDFLAGS=-s -lshell32 -lkernel32 -luser32 -lgdi32 -lcomctl32 -lcomdlg32

ifdef TERM
GREEN=`tput setaf 2``tput bold`
BLUE=`tput setaf 4``tput bold`
RESET=`tput sgr0`
DIM=`tput dim`
endif

.PHONY: all clean

all: $(BINARIES)
	@printf '%sBuild successful%s\n' "$(BLUE)$(DIM)" "$(RESET)"

clean:
	@printf '%sRemoving build artifacts%s\n' "$(BLUE)" "$(RESET)"
	@rm -rf bin obj

obj/%.rc.o: res/%.rc makefile
	@printf "%sCompiling resource object %s\n" "$(GREEN)$(DIM)" "$@$(RESET)"
	@mkdir -p obj
	@$(RC) -i $< -o $@

obj/%.o: src/%.c makefile
	@printf '%sCompiling C object %s\n' "$(GREEN)$(DIM)" "$@$(RESET)"
	@mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

bin/%.exe: obj/%.o obj/common-console.rc.o
	@printf '%sLinking executable %s\n' "$(GREEN)" "$@$(RESET)"
	@mkdir -p bin
	@$(LD) $< obj/common-console.rc.o -o $@ -s $(LDFLAGS)
	@printf '%sBuilt target %s\n' "$(BLUE)" "$@$(RESET)"

bin/mdicapture.exe: obj/mdicapture.o obj/common-gui.rc.o
	@printf '%sLinking executable %s\n' "$(GREEN)" "$@$(RESET)"
	@mkdir -p bin
	@$(LD) $< obj/common-gui.rc.o -o $@ -s $(LDFLAGS) -Wl,--subsystem,windows
	@printf '%sBuilt target %s\n' "$(BLUE)" "$@$(RESET)"

bin/dbglist.exe: obj/dbglist.o obj/admin-gui.rc.o
	@printf '%sLinking executable %s\n' "$(GREEN)" "$@$(RESET)"
	@mkdir -p bin
	@$(LD) $< obj/admin-gui.rc.o -o $@ -s $(LDFLAGS) -Wl,--subsystem,windows
	@printf '%sBuilt target %s\n' "$(BLUE)" "$@$(RESET)"
