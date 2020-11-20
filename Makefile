CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := bin/MoloVol

TESTDIR := test
TESTTARGET := test/out

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

TESTSOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJECTS := $(patsubst $(TESTDIR)/%,$(TESTDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.o))

CXXFLAGS := -O0 -g -std=c++17 -Wall -Werror 
CFLAGS := -O0 -g -std=c++17 -Wno-unused-command-line-argument
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

$(TESTDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(INC) -c `wx-config --cxxflags --libs` -o $@ $<

cleantest:
	@echo " Cleaning Test Directory..."
	@echo " $(RM) $(TESTTARGET)"; $(RM) $(TESTTARGET)
	@echo " $(RM) $(TESTOBJECTS)"; $(RM) -r $(TESTOBJECTS)

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean, test, cleantest
