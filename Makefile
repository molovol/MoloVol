CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
APP := MoloVol
DMGNAME :=MoloVol_macOS_beta_v1
TARGET := $(BINDIR)/$(APP)

BUNDLE := $(TARGET).app

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
WXFLAGS := --cxxflags --libs --version=3.1
INC := -I include

all: CXXFLAGS += $(DEBUGFLAGS)
all: CFLAGS += $(DEBUGFLAGS)
all: $(TARGET)

appbundle: CXXFLAGS += $(RELEASEFLAGS)
appbundle: CFLAGS += $(RELEASEFLAGS)
appbundle: $(TARGET)
	@echo "Creating bundle..."
	@echo " $(RM) -r $(BUNDLE)"; $(RM) -r $(BUNDLE)
	@echo " mkdir -p $(BUNDLE)"; mkdir -p $(BUNDLE)
	@echo " mkdir -p $(BUNDLE)/Contents"; mkdir -p $(BUNDLE)/Contents
	@echo " cp res/MacOS/Info.plist $(BUNDLE)/Contents"; cp res/MacOS/Info.plist $(BUNDLE)/Contents
	@echo " mkdir -p $(BUNDLE)/Contents/MacOS"; mkdir -p $(BUNDLE)/Contents/MacOS
	@echo " mv $(TARGET) $(BUNDLE)/Contents/MacOS"; mv $(TARGET) $(BUNDLE)/Contents/MacOS
	@echo " mkdir -p $(BUNDLE)/Contents/Resources"; mkdir -p $(BUNDLE)/Contents/Resources
	@echo " cp res/MacOS/icon.icns $(BUNDLE)/Contents/Resources"; cp res/MacOS/icon.icns $(BUNDLE)/Contents/Resources
	@echo " cp inputfile/radii.txt $(BUNDLE)/Contents/Resources"; cp inputfile/radii.txt $(BUNDLE)/Contents/Resources
	@echo " cp inputfile/space_groups.txt $(BUNDLE)/Contents/Resources"; cp inputfile/space_groups.txt $(BUNDLE)/Contents/Resources
	/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f $(BUNDLE)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $^ `wx-config $(WXFLAGS)` -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c `wx-config $(WXFLAGS)` -o $@ $<

dmg: appbundle
	@echo " $(RM) -r $(BINDIR)/$(DMGNAME).dmg"; $(RM) -r $(BINDIR)/$(DMGNAME).dmg
	@$(RM) -r $(BINDIR)/dmgdir
	@echo "Creating dmg file..."
	@mkdir -p $(BINDIR)/dmgdir
	@mv $(BUNDLE) $(BINDIR)/dmgdir
	@cp README.md $(BINDIR)/dmgdir/README.txt
	@cp LICENSE $(BINDIR)/dmgdir/LICENSE.txt
	@hdiutil create -fs HFS+ -srcfolder "$(BINDIR)/dmgdir" -volname "$(DMGNAME)" "$(BINDIR)/$(DMGNAME).dmg"
	@$(RM) -r $(BINDIR)/dmgdir

test: $(TESTOBJECTS)
	$(CC) $(CXXFLAGS) $^ `wx-config $(WXFLAGS)` -o $(TESTTARGET)

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c `wx-config $(WXFLAGS)` -o $@ $<

probetest:
	$(TARGET) -u excluded

protein:
	$(TARGET) -u protein

cleantest:
	@echo "Cleaning Test Directory..."
	@echo " $(RM) -r $(TESTBUILDDIR)"; $(RM) -r $(TESTBUILDDIR)
	@echo " $(RM) $(TESTTARGET)"; $(RM) $(TESTTARGET)

clean:
	@echo "Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo " $(RM) -r $(BUNDLE)"; $(RM) -r $(BUNDLE)
	@echo " $(RM) -r $(BINDIR)/$(DMGNAME).dmg"; $(RM) -r $(BINDIR)/$(DMGNAME).dmg

cleanall:
	@echo "Cleaning..."
	@echo " $(RM) -r $(BINDIR)"; $(RM) -r $(BINDIR)


.PHONY: all, clean, cleanall, appbundle, test, cleantest, probetest, protein
