PREFIX = /usr/local
BOOST_PREFIX = $(PREFIX)

#
# Compiler flags
#

CCXX   = g++
CC     = gcc
CXXFLAGS = -Wall -Werror -Wextra -std=gnu++20 -isystem $(BOOST_PREFIX)/include/boost_1_74_0
CFLAGS   = -Wall -Werror -Wextra

#
# Project files
#

CPPSRCS = $(wildcard *.cpp)
CSRCS = $(wildcard *.c)
OBJS = $(CPPSRCS:.cpp=.o) $(CSRCS:.c=.o)
LIB = libccalc
LIBDIR = lib

#
# Debug build settings
#

DBGDIR = debug
DBGLIB = $(LIBDIR)/$(LIB)-dbg.a
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGDEPS = $(DBGOBJS:%.o=%.d)
DBGFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#

RELDIR = release
RELLIB = $(LIBDIR)/$(LIB)-rel.a
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELDEPS = $(RELOBJS:%.o=%.d)
RELFLAGS = -Os -DNDEBUG

.PHONY: all clean debug release remake install installdbg uninstall

# Default build
all: install

#
# Debug rules
#

debug: make_dbgdir $(DBGLIB)

$(DBGLIB): $(DBGOBJS)
	ar rcs $(DBGLIB) $^

-include $(DBGDEPS)

$(DBGDIR)/%.o: %.cpp
	$(CCXX) -c $(CXXFLAGS) $(DBGFLAGS) -MMD -o $@ $<

$(DBGDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBGFLAGS) -MMD -o $@ $<

#
# Release rules
#

release: make_reldir $(RELLIB)

$(RELLIB): $(RELOBJS)
	ar rcs $(RELLIB) $^

-include $(RELDEPS)

$(RELDIR)/%.o: %.cpp
	$(CCXX) -c $(CXXFLAGS) $(RELFLAGS) -MMD -o $@ $<

$(RELDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELFLAGS) -MMD -o $@ $<

#
# Install/uninstall rules
#

install: release
	sudo mkdir -p $(DESTDIR)$(PREFIX)/include/ccalc
	sudo cp *.hpp $(DESTDIR)$(PREFIX)/include/ccalc
	sudo mkdir -p $(DESTDIR)$(PREFIX)/lib
	sudo cp $(RELLIB) $(DESTDIR)$(PREFIX)/lib

installdbg: debug
	sudo mkdir -p $(DESTDIR)$(PREFIX)/include/ccalc
	sudo cp *.hpp $(DESTDIR)$(PREFIX)/include/ccalc
	sudo mkdir -p $(DESTDIR)$(PREFIX)/lib
	sudo cp $(DBGLIB) $(DESTDIR)$(PREFIX)/lib

uninstall: # cleanup
	sudo rm -r -f $(DESTDIR)$(PREFIX)/include/ccalc
	sudo rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)-rel.a
	sudo rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)-dbg.a

#
# Other rules
#

make_dbgdir:
	@mkdir -p $(DBGDIR)
	@mkdir -p $(LIBDIR)

make_reldir:
	@mkdir -p $(RELDIR)
	@mkdir -p $(LIBDIR)

remake: clean all

clean:
	@rm -r -f $(RELDIR) $(DBGDIR) $(LIBDIR)
