CXX=g++
CXXFLAGS=-O2 `pkg-config --cflags --libs opencv` -lpthread -w -std=c++11
EXE=run.bin
ANALYTICS_EXE=analytics.bin
TRAINING_EXE=training.bin
SRCDIR=src
BLDDIR=build
SRCS=$(shell find $(SRCDIR) -name "*.cpp")
DEPS=$(shell find $(SRCDIR) -name "*.hpp")
OBJS=$(SRCS:$(SRCDIR)/%.cpp=$(BLDDIR)/%.o)


all: $(EXE)


.PHONY: add_macro_analytics
analytics: add_macro_analytics $(ANALYTICS_EXE)

add_macro_analytics:
	$(eval CXXFLAGS += -D ANALYTICS)


.PHONY: add_macro_training
training: add_macro_training $(TRAINING_EXE)


.PHONY: raspberrypi
raspberrypi: $(EXE)
	$(eval CXXFLAGS += -D RASPBERRYPI)

.PHONY: beagleboneblack
beagleboneblack: $(EXE)
	$(eval CXXFLAGS += -D BEAGLEBONEBLACK)

add_macro_training:
	$(eval CXXFLAGS += -D SAVE_ALL_IMAGES)


$(EXE) $(ANALYTICS_EXE) $(TRAINING_EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BLDDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(DEPS):


.PHONY: clean
clean: 
	@rm -rf *.o $(EXE)
	@find build/ -name "*.o" -type f -delete
