CXX=g++
CXXFLAGS=-O2 `pkg-config --cflags --libs opencv` -lpthread -w
EXE=run.bin
SRCDIR=src
BLDDIR=build
SRCS=$(shell find $(SRCDIR) -name "*.cpp")
DEPS=$(shell find $(SRCDIR) -name "*.hpp")
OBJS=$(SRCS:$(SRCDIR)/%.cpp=$(BLDDIR)/%.o)


all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BLDDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $^

$(DEPS):

.PHONY: clean
clean: 
	@rm -rf *.o $(EXE)
	@find build/ -name "*.o" -type f -delete
