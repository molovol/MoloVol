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

CXXFLAGS := -O3 -g -std=c++17 -Wall -Werror 
CFLAGS := -O3 -g -std=c++17 -Wno-unused-command-line-argument
INC := -I include

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

.PHONY: clean, test, cleantest, probetest, protein
