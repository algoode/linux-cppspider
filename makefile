TARGET=spider
SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SOURCES))
REAL_LDFLAGS=$(LDFLAGS) $(PROF) -rdynamic -lpthread -levent -lcrypt -ldl

all: $(TARGET)

$(TARGET): $(OBJS) wrap.o 
	$(CXX) -o ./$@ $^ $(REAL_LDFLAGS)
%.o:%.cpp
	g++ -c $< -Wall
wrap.o:
	gcc -c wrap.c
	
.PHONY: clean all
clean: 
	-rm  $(spider) $(OBJS) wrap.o

