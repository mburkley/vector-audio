SUFFIXES+=.d

OBJECTS=draw.o 

LIBS=\
-lpulse-simple\
-lpulse\
-lm 

CFLAGS=-Wall -ggdb3
# LDFLAGS=

all:  draw

DEPS=$(OBJECTS:.o=.d)
-include $(DEPS)

draw: $(OBJECTS)
	@echo "\t[LD] $@..."
	@$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.d: %.cc
	@echo "\t[I] $<... $(patsubst %.cc,%.o,$<)"
	@$(CXX) $(CFLAGS) -MM -MT '$(patsubst %.cc,%.o,$<)' $< -MF $@

%.o: %.cc # %.d
	@echo "\t[CC] $<..."
	@$(CXX) -c $(CFLAGS) $< -o $@

