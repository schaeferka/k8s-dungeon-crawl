include config.mk

CC := gcc
CFLAGS := -Isrc/brogue -Isrc/platform -Isrc/variants -std=c99 \
    -Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result \
    -Wformat -Werror=format-security -Wformat-overflow=0 -Wmissing-prototypes
# Set standard Ubuntu paths for curl
CFLAGS += -I/usr/include/x86_64-linux-gnu
LDLIBS := -lm -lmicrohttpd -L/usr/lib/x86_64-linux-gnu -lcurl
CPPFLAGS := -DDATADIR=\"$(DATADIR)\"

sources := $(wildcard src/brogue/*.c) $(wildcard src/variants/*.c) \
           $(addprefix src/platform/,main.c platformdependent.c null-platform.c)
objects := $(sources:.c=.o)

ifeq ($(SYSTEM), WINDOWS)
    objects += windows/icon.o
    .exe := .exe
endif

ifeq ($(SYSTEM), OS2)
    CFLAGS += -march=pentium4 -mtune=pentium4 -Zmt -Wno-narrowing
    CPPFLAGS += -D__ST_MT_ERRNO__
    LDLIBS += -lcx -lc -Zomf -Zbin-files -Zargs-wild -Zargs-resp -Zhigh-mem -Zstack 8000
    objects += os2/icon.res os2/brogue.lib
    .exe := .exe
endif

ifeq ($(TERMINAL), YES)
    sources += $(addprefix src/platform/,curses-platform.c term.c)
    CPPFLAGS += -DBROGUE_CURSES
    LDLIBS += -lncurses
    ifeq ($(SYSTEM), OS2)
        LDLIBS += -ltinfo
    endif
endif

ifeq ($(GRAPHICS), YES)
    sources += $(addprefix src/platform/,sdl2-platform.c tiles.c)
    CFLAGS += $(shell $(SDL_CONFIG) --cflags)
    CPPFLAGS += -DBROGUE_SDL
    LDLIBS += $(shell $(SDL_CONFIG) --libs) -lSDL2_image
endif

ifeq ($(WEBBROGUE), YES)
    sources += $(addprefix src/platform/,web-platform.c)
    CPPFLAGS += -DBROGUE_WEB
endif

ifeq ($(DEBUG), YES)
    CFLAGS += -g -Og
    CPPFLAGS += -DENABLE_PLAYBACK_SWITCH
else
    CFLAGS += -O2
endif

# Define the objects list
objects += $(sources:.c=.o)

# Set the default goal to create the Brogue binary
.DEFAULT_GOAL := bin/brogue$(.exe)

%.o: %.c
    $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ 

clean:
    rm -f $(objects) bin/brogue$(.exe)

.PHONY: clean