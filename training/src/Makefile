CXX=g++
CXXFLAGS=-O2 `pkg-config --cflags --libs opencv` -I../../src/
OTHER_OBJS=../../build/PipelineBuffer.o ../../build/Process.o
EXE=printvariances.bin


all: $(EXE)

$(EXE): printvariances.cpp
	$(CXX) -o $@ $^ $(OTHER_OBJS) $(CXXFLAGS)

.PHONY: clean
clean:
	@rm -f *.bin

.PHONY: re
re: clean all
	
