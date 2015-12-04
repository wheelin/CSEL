CXX?=g++
CXXFLAGS=-Wall -Wextra -g -MMD
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=read-apache-logs

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

.c.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
	
clean:
	@rm -f $(OBJECTS)
	@rm -f *.d *~
	
clean_all: clean
	@rm -f $(EXECUTABLE)
	@rm -f perf.data perf.data.old

-include *.d
