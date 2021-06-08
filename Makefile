CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := bin/MoloVol

TESTDIR := test
TESTTARGET := test/out
TESTBUILDDIR := test/build

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

TESTSOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJECTS := $(patsubst $(TESTDIR)/%,$(TESTBUILDDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.o))

DEBUGFLAGS := -O0 -g -D DEBUG
RELEASEFLAGS := -O3
CXXFLAGS := -std=c++17 -Wall -Werror 
CFLAGS := -std=c++17 -Wno-unused-command-line-argument -Wno-invalid-source-encoding
INC := -I include

all: CXXFLAGS += $(DEBUGFLAGS)
all: CFLAGS += $(DEBUGFLAGS)
all: $(TARGET)

appbundle: CXXFLAGS += $(RELEASEFLAGS)
appbundle: CFLAGS += $(RELEASEFLAGS)
appbundle: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $^ `wx-config --cxxflags --libs` -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c `wx-config --cxxflags --libs` -o $@ $<

test: $(TESTOBJECTS)
	$(CC) $(CXXFLAGS) $^ `wx-config --cxxflags --libs` -o $(TESTTARGET)

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c `wx-config --cxxflags --libs` -o $@ $<

probetest:
	$(TARGET) -u excluded

protein:
	$(TARGET) -u protein

cleantest:
	@echo " Cleaning Test Directory..."
	@echo " $(RM) -r $(TESTBUILDDIR)"; $(RM) -r $(TESTBUILDDIR)
	@echo " $(RM) $(TESTTARGET)"; $(RM) $(TESTTARGET)

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean, test, cleantest, probetest, protein, appbundle
