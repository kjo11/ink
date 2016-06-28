CC = gcc 
CFLAGS += -std=c99 -g -pedantic -Wall

SRCDIR = src
INCLUDEDIR = include
OBJDIR = build
TARGETDIR = bin

TARGET := $(TARGETDIR)/ink
HEADERS := $(wildcard $(INCLUDEDIR)/*.h)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS  := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS := $(OBJS:%.o=%.d)

DIRECTORIES := $(OBJDIR) $(TARGETDIR)

LIBRARYDIRS :=
LIBRARIES := 

CPPFLAGS += $(foreach includedir,$(INCLUDEDIR),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(LIBRARYDIR),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

LINK.c := $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

-include $(DEPS)

.PHONY: all clean cleaner

all: $(TARGET)

clean: 
	@rm -rf $(OBJDIR)

cleaner: clean
	@rm -rf $(TARGETDIR)

$(TARGET): $(OBJS) | $(TARGETDIR)
	$(LINK.c) $(OBJS) -o $(TARGET)


$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(LINK.c) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $<
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(DIRECTORIES):
	@mkdir -p $@

