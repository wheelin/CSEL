TOOLCHAIN_PATH=~/workspace/xu3/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)arm-linux-

CC=$(TOOLCHAIN)gcc
CFLAGS=-Wall -Wextra  -g -O2 -std=c11
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ex2

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
	
clean:
	@rm -f $(OBJECTS)
	@rm -f *.d *~
	
clean_all: clean
	@rm -f $(EXECUTABLE)
	@rm -f perf.data perf.data.old	

-include *.d
