# build parameters
DEBUG ?= 0
INSTALL_PREFIX ?= $(HOME)/software

# compiler suite
CXX = g++

# compiler flags, linker flags
CXXFLAGS = \
	-std=c++11 \
	-I$(CUDADIR)/include \
	-I$(INSTALL_PREFIX)/include

LDFLAGS = \
	-lm \
	-L$(INSTALL_PREFIX)/lib -lmlearn \
	-lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect

ifeq ($(DEBUG), 1)
CXXFLAGS += -g -pg -Wall
else
CXXFLAGS += -O3 -Wno-unused-result
endif

# binary targets
OBJDIR = obj
OBJS = \
	$(OBJDIR)/bboxiterator.o \
	$(OBJDIR)/main.o
BINS = face-rec

all: echo $(BINS)

echo:
	$(info DEBUG     = $(DEBUG))
	$(info CXX       = $(CXX))
	$(info LDFLAGS   = $(LDFLAGS))
	$(info CXXFLAGS  = $(CXXFLAGS))

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: src/%.cpp src/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

face-rec: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJDIR) $(BINS) gmon.out
