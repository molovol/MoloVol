CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
LINUXRES := res/linux
SRCEXT := cpp
APP := MoloVol
INSTALLERNAME := MoloVol_macOS_beta_v1

TARGET := $(BINDIR)/$(APP)
BUNDLE := $(TARGET).app
BUILDDIR_ARM64 := $(BUILDDIR)/arm64
BUILDDIR_X86 := $(BUILDDIR)/x86

TESTDIR := test
TESTTARGET := test/out
TESTBUILDDIR := test/build

SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
OBJECTS_ARM64 := $(patsubst $(SRCDIR)/%,$(BUILDDIR_ARM64)/%,$(SOURCES:.$(SRCEXT)=.o))
OBJECTS_X86 := $(patsubst $(SRCDIR)/%,$(BUILDDIR_X86)/%,$(SOURCES:.$(SRCEXT)=.o))

TESTSOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJECTS := $(patsubst $(TESTDIR)/%,$(TESTBUILDDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.o))

DEBUGFLAGS := -O0 -g -D DEBUG
RELEASEFLAGS := -O3
CXXFLAGS := -std=c++17 -Wall -Werror 
CFLAGS := -std=c++17 -Wno-unused-command-line-argument -Wno-invalid-source-encoding
WXFLAGS := --cxxflags --libs --version=3.1
ARCHFLAG := 
X86FLAG := -target x86_64-apple-macos10.12
ARM64FLAG := -target arm64-apple-macos11
INC := -I include

# DEVELOPMENT BUILD
all: CXXFLAGS += $(DEBUGFLAGS)
all: CFLAGS += $(DEBUGFLAGS)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ `wx-config $(WXFLAGS)` -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) $(ARCHFLAG) -c `wx-config $(WXFLAGS)` -o $@ $<

# RELEASE BUILD - SHOULD BE PLACED IN INSTALLER PACKAGE
release: CXXFLAGS += $(RELEASEFLAGS)
release: CFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

# FOR UNIVERSAL BINARY ON MACOS
arm64_app: ARCHFLAG = $(ARM64FLAG)
arm64_app: $(OBJECTS_ARM64)
	@echo "Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ `wx-config $(WXFLAGS)` -o $(BINDIR)/arm64_app

$(BUILDDIR_ARM64)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR_ARM64)
	$(CC) $(CFLAGS) $(INC) $(ARCHFLAG) -c `wx-config $(WXFLAGS)` -o $@ $<

x86_app: ARCHFLAG = $(X86FLAG)
x86_app: $(OBJECTS_X86)
	@echo "Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ `wx-config $(WXFLAGS)` -o $(BINDIR)/x86_app

$(BUILDDIR_X86)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR_X86)
	$(CC) $(CFLAGS) $(INC) $(ARCHFLAG) -c `wx-config $(WXFLAGS)` -o $@ $<

universal_app: CXXFLAGS += $(RELEASEFLAGS)
universal_app: CFLAGS += $(RELEASEFLAGS)
universal_app: x86_app arm64_app
	@lipo -create -output $(TARGET) $(BINDIR)/x86_app $(BINDIR)/arm64_app

appbundle_universal: universal_app
appbundle_universal: appbundle_entry

dmg_universal: appbundle_universal
dmg_universal: dmg_entry

# INSTALLER FOR MACOS
appbundle: release
appbundle: appbundle_entry
appbundle_entry:
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

dmg: appbundle
dmg: dmg_entry
dmg_entry:
	@echo " $(RM) -r $(BINDIR)/$(INSTALLERNAME).dmg"; $(RM) -r $(BINDIR)/$(INSTALLERNAME).dmg
	@$(RM) -r $(BINDIR)/dmgdir
	@echo "Creating dmg file..."
	@mkdir -p $(BINDIR)/dmgdir
	@mv $(BUNDLE) $(BINDIR)/dmgdir
	@cp README.md $(BINDIR)/dmgdir/README.txt
	@cp LICENSE $(BINDIR)/dmgdir/LICENSE.txt
	@ln -s /Applications/ $(BINDIR)/dmgdir/drag_app_here
	@hdiutil create -fs HFS+ -srcfolder "$(BINDIR)/dmgdir" -volname "$(INSTALLERNAME)" "$(BINDIR)/$(INSTALLERNAME).dmg"
	@$(RM) -r $(BINDIR)/dmgdir

# INSTALLER ON DEBIAN AND UBUNTU
deb: release
	@$(RM) -r $(BINDIR)/deb-staging
	@echo "Creating deb file..."
	@bash $(LINUXRES)/shell/deb-filestructure.sh $(BINDIR)
	@cp $(LINUXRES)/control $(BINDIR)/deb-staging/DEBIAN/
	@bash $(LINUXRES)/shell/f-architecture.sh >> $(BINDIR)/deb-staging/DEBIAN/control
	@strip $(TARGET)
	@mv $(TARGET) $(BINDIR)/deb-staging/usr/bin/molovol
	@cp $(LINUXRES)/MoloVol.desktop $(BINDIR)/deb-staging/usr/share/applications/
	@cp $(LINUXRES)/copyright $(BINDIR)/deb-staging/usr/share/doc/molovol/
	@cp $(LINUXRES)/changelog $(BINDIR)/deb-staging/usr/share/doc/molovol/
	@gzip -9 -n $(BINDIR)/deb-staging/usr/share/doc/molovol/changelog
	@cp $(LINUXRES)/molovol.1 $(BINDIR)/deb-staging/usr/share/man/man1/
	@gzip -9 -n $(BINDIR)/deb-staging/usr/share/man/man1/molovol.1
	@cp inputfile/space_groups.txt $(BINDIR)/deb-staging/usr/share/molovol/
	@cp inputfile/radii.txt $(BINDIR)/deb-staging/usr/share/molovol/
	@cp $(LINUXRES)/molovol.png $(BINDIR)/deb-staging/usr/share/pixmaps/
	@bash $(LINUXRES)/shell/icons.sh $(LINUXRES)/molovol.png $(BINDIR)/deb-staging/usr/share/icons/hicolor
	@find $(BINDIR)/deb-staging/usr -type f -exec chmod 0644 {} +
	@chmod 0755 $(BINDIR)/deb-staging/usr/bin/molovol
	@dpkg-deb --root-owner-group --build "$(BINDIR)/deb-staging" "$(BINDIR)/$(INSTALLERNAME).deb"
	@$(RM) -r $(BINDIR)/deb-staging

# COMPILES ONLY FILES INSIDE TEST DIRECTORY
test: $(TESTOBJECTS)
	$(CC) $(CXXFLAGS) $^ `wx-config $(WXFLAGS)` -o $(TESTTARGET)

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c `wx-config $(WXFLAGS)` -o $@ $<

# REMOVE TEST DIRECTORY BINARIES
cleantest:
	@echo "Cleaning Test Directory..."
	@echo " $(RM) -r $(TESTBUILDDIR)"; $(RM) -r $(TESTBUILDDIR)
	@echo " $(RM) $(TESTTARGET)"; $(RM) $(TESTTARGET)

# EXPLICIT CLEAN
clean:
	@echo "Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
	@echo " $(RM) -r $(BINDIR)/arm64_app"; $(RM) -r $(BINDIR)/arm64_app
	@echo " $(RM) -r $(BINDIR)/x86_app"; $(RM) -r $(BINDIR)/x86_app
	@echo " $(RM) -r $(BUNDLE)"; $(RM) -r $(BUNDLE)
	@echo " $(RM) -r $(BINDIR)/$(INSTALLERNAME).dmg"; $(RM) -r $(BINDIR)/$(INSTALLERNAME).dmg
	@echo " $(RM) -r $(BINDIR)/$(INSTALLERNAME).deb"; $(RM) -r $(BINDIR)/$(INSTALLERNAME).deb

# DELETE BIN DIRECTORY
cleanall:
	@echo "Cleaning..."
	@echo " $(RM) -r $(BUILDIR) $(BINDIR)"; $(RM) -r $(BUILDIR) $(BINDIR)


.PHONY: all, clean, cleanall, release, universal_app, x86_app, arm64_app, appbundle, dmg, deb, test, cleantest, probetest, protein
