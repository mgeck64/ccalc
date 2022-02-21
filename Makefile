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
LIBDIR = lib

#
# Debug build settings
#

DBGDIR = debug
DBGLIB = libccalc-dbg.a
DBGLIBPATHNAME = $(LIBDIR)/$(DBGLIB)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGDEPS = $(DBGOBJS:%.o=%.d)
DBGFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#

RELDIR = release
RELLIB = libccalc-rel.a
RELLIBPATHNAME = $(LIBDIR)/$(RELLIB)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELDEPS = $(RELOBJS:%.o=%.d)
RELFLAGS = -Os -DNDEBUG

.PHONY: all clean debug release remake install installdbg uninstall

# Default build
all: install

#
# Debug rules
#

debug: make_dbgdir $(DBGLIBPATHNAME)

$(DBGLIBPATHNAME): $(DBGOBJS)
	ar rcs $(DBGLIBPATHNAME) $^

-include $(DBGDEPS)

$(DBGDIR)/%.o: %.cpp
	$(CCXX) -c $(CXXFLAGS) $(DBGFLAGS) -MMD -o $@ $<

$(DBGDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBGFLAGS) -MMD -o $@ $<

#
# Release rules
#

release: make_reldir $(RELLIBPATHNAME)

$(RELLIBPATHNAME): $(RELOBJS)
	ar rcs $(RELLIBPATHNAME) $^

-include $(RELDEPS)

$(RELDIR)/%.o: %.cpp
	$(CCXX) -c $(CXXFLAGS) $(RELFLAGS) -MMD -o $@ $<

$(RELDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELFLAGS) -MMD -o $@ $<

#
# Install/uninstall rules
#

install: release
	install -D -t $(DESTDIR)$(PREFIX)/include/ccalc *.hpp
	install -D -t $(DESTDIR)$(PREFIX)/lib $(RELLIBPATHNAME)

installdbg: debug
	install -D -t $(DESTDIR)$(PREFIX)/include/ccalc *.hpp
	install -D -t $(DESTDIR)$(PREFIX)/lib $(DBGLIBPATHNAME)

uninstall: # cleanup
	rm -r -f $(DESTDIR)$(PREFIX)/include/ccalc
	rm -f $(DESTDIR)$(PREFIX)/lib/$(RELLIB)
	rm -f $(DESTDIR)$(PREFIX)/lib/$(DBGLIB)

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
