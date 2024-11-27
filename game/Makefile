include config.mk
# Compiler flags and includes
cflags := -Isrc/brogue -Isrc/platform -Isrc/variants -Isrc/portal -std=c18 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result \
	-Wformat -Werror=format-security -Wshift-overflow -Wmissing-prototypes \
	-I$(shell brew --prefix curl)/include \
	-I/opt/homebrew/opt/cmocka/include \
	-I$(shell brew --prefix ncurses)/include
libs := -lm -lmicrohttpd \
	-L$(shell brew --prefix curl)/lib -lcurl \
	-lcmocka \
	-L$(shell brew --prefix ncurses)/lib -lncurses

cppflags := -DDATADIR=$(DATADIR)

# Define the sources, excluding test files
sources := $(wildcard src/brogue/*.c) $(wildcard src/variants/*.c) $(wildcard src/portal/*.c) $(addprefix src/platform/,main.c platformdependent.c null-platform.c)
objects :=

# Only include test files when `make test` is called
ifeq ($(MAKECMDGOALS),test)
    sources += $(wildcard src/portal/tests/*.c)
else
    # Exclude test files from the main build
    sources := $(filter-out src/portal/tests/*.c, $(sources))
endif

ifeq ($(SYSTEM),WINDOWS)
objects += windows/icon.o
.exe := .exe
endif

ifeq ($(SYSTEM),OS2)
cflags += -march=pentium4 -mtune=pentium4 -Zmt -Wno-narrowing
cppflags += -D__ST_MT_ERRNO__
libs += -lcx -lc -Zomf -Zbin-files -Zargs-wild -Zargs-resp -Zhigh-mem -Zstack 8000
objects += os2/icon.res os2/brogue.lib
.exe := .exe
endif

ifeq ($(RELEASE),YES)
extra_version :=
else
extra_version := $(shell bash tools/git-extra-version)
endif

ifeq ($(TERMINAL),YES)
sources += $(addprefix src/platform/,curses-platform.c term.c)
cppflags += -DBROGUE_CURSES
libs += -lncurses
ifeq ($(SYSTEM),OS2)
libs += -ltinfo
endif
endif

ifeq ($(GRAPHICS),YES)
sources += $(addprefix src/platform/,sdl2-platform.c tiles.c)
cflags += $(shell $(SDL_CONFIG) --cflags)
cppflags += -DBROGUE_SDL
libs += $(shell $(SDL_CONFIG) --libs) -lSDL2_image
endif

ifeq ($(WEBBROGUE),YES)
sources += $(addprefix src/platform/,web-platform.c)
cppflags += -DBROGUE_WEB
endif

ifeq ($(RAPIDBROGUE),YES)
cppflags += -DRAPID_BROGUE
endif

ifeq ($(MAC_APP),YES)
cppflags += -DSDL_PATHS
endif

ifeq ($(DEBUG),YES)
cflags += -g -Og
cppflags += -DENABLE_PLAYBACK_SWITCH
else
cflags += -O2
endif

# Add user-provided flags.
cflags += $(CFLAGS)
cppflags += $(CPPFLAGS)
libs += $(LDLIBS)

objects += $(sources:.c=.o)

include make/*.mk
.DEFAULT_GOAL := bin/brogue$(.exe)

clean:
	rm -f $(objects)
	$(warning 'make clean' is no longer needed in many situations, so is not supported. Use 'make -B' to force rebuild something.)

escape = $(subst ','\'',$(1))
vars:
	mkdir -p vars
# Write the value to a temporary file and only overwrite if it's different.
vars/%: vars FORCE
	@echo '$(call escape,$($*))' > vars/$*.tmp
	@if cmp --quiet vars/$*.tmp vars/$*; then :; else cp vars/$*.tmp vars/$*; fi

test: 
	@echo "Running all Portal C tests..."
	# Compile the tests
	make -B -C src/portal/tests || exit 1

FORCE: