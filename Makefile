# THIS FILE WILL BE PHASED OUT IN FAVOUR OF CMAKE

CC := g++
SRCDIR := src
BUILDDIR := build
BINDIR := bin
LINUXRES := res/linux
SRCEXT := cpp
APP := MoloVol
VERSION := v1.0
DMGNAME := $(APP)_macOS-10.11+_$(VERSION)
DEBNAME := $(APP)_debian_$(VERSION)

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

DEBUGFLAGS := -O3 -g -DDEBUG
RELEASEFLAGS := -O3 -DABS_PATH
CXXFLAGS := -std=c++17 -Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding

WXCONFIGLIBS := $(shell wx-config --libs)
WXCONFIGLIBS := $(WXCONFIGLIBS:-ltiff=/usr/local/opt/libtiff/lib/libtiff.a)
LDFLAGS := $(WXCONFIGLIBS)
ARCHFLAG := 
X86FLAG := -target x86_64-apple-macos10.11
ARM64FLAG := -target arm64-apple-macos11
MACOS_VERSIONFLAG := -mmacosx-version-min=10.11
INC := -I include

# DEVELOPMENT BUILD
all: CXXFLAGS += $(DEBUGFLAGS)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ $(LDFLAGS) -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CXXFLAGS) $(INC) $(ARCHFLAG) -c `wx-config --cxxflags` -o $@ $<

# RELEASE BUILD - SHOULD BE PLACED IN INSTALLER PACKAGE
release: CXXFLAGS += $(RELEASEFLAGS)
release: $(TARGET)

# FOR UNIVERSAL BINARY ON MACOS
arm64_app: ARCHFLAG = $(ARM64FLAG)
arm64_app: $(OBJECTS_ARM64)
	@echo "Linking..."
	@mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ $(LDFLAGS) -o $(BINDIR)/arm64_app

$(BUILDDIR_ARM64)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR_ARM64)
	$(CC) $(CXXFLAGS) $(INC) $(ARCHFLAG) -c `wx-config --cxxflags` -o $@ $<

x86_app: ARCHFLAG = $(X86FLAG)
x86_app: $(OBJECTS_X86)
	@echo "Linking..."
	@mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) $(ARCHFLAG) $^ $(LDFLAGS) -o $(BINDIR)/x86_app

$(BUILDDIR_X86)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR_X86)
	$(CC) $(CXXFLAGS) $(INC) $(ARCHFLAG) -c `wx-config --cxxflags` -o $@ $<

universal_app: CXXFLAGS += $(RELEASEFLAGS)
universal_app: x86_app arm64_app
	@lipo -create -output $(TARGET) $(BINDIR)/x86_app $(BINDIR)/arm64_app

appbundle_universal: universal_app
appbundle_universal: appbundle_entry

dmg_universal: appbundle_universal
dmg_universal: dmg_entry

# INSTALLER FOR MACOS
appbundle: CXXFLAGS += $(MACOS_VERSIONFLAG)
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
	@echo " cp inputfile/elements.txt $(BUNDLE)/Contents/Resources"; cp inputfile/elements.txt $(BUNDLE)/Contents/Resources
	@echo " cp inputfile/space_groups.txt $(BUNDLE)/Contents/Resources"; cp inputfile/space_groups.txt $(BUNDLE)/Contents/Resources
	/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f $(BUNDLE)

dmg: appbundle
dmg: dmg_entry
dmg_entry:
	@echo " $(RM) -r $(BINDIR)/$(DMGNAME).dmg"; $(RM) -r $(BINDIR)/$(DMGNAME).dmg
	@$(RM) -r $(BINDIR)/dmgdir
	@echo "Creating dmg file..."
	@mkdir -p $(BINDIR)/dmgdir
	@mv $(BUNDLE) $(BINDIR)/dmgdir
	@cp README.md $(BINDIR)/dmgdir/README.txt
	@cp LICENSE $(BINDIR)/dmgdir/LICENSE.txt
	@ln -s /Applications/ $(BINDIR)/dmgdir/drag_app_here
	@hdiutil create -fs HFS+ -srcfolder "$(BINDIR)/dmgdir" -volname "$(DMGNAME)" "$(BINDIR)/$(DMGNAME).dmg"
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
	@cp inputfile/elements.txt $(BINDIR)/deb-staging/usr/share/molovol/
	@cp $(LINUXRES)/molovol.png $(BINDIR)/deb-staging/usr/share/pixmaps/
	@bash $(LINUXRES)/shell/icons.sh $(LINUXRES)/molovol.png $(BINDIR)/deb-staging/usr/share/icons/hicolor
	@find $(BINDIR)/deb-staging/usr -type f -exec chmod 0644 {} +
	@chmod 0755 $(BINDIR)/deb-staging/usr/bin/molovol
	@dpkg-deb --root-owner-group --build "$(BINDIR)/deb-staging" "$(BINDIR)/$(DEBNAME).deb"
	@$(RM) -r $(BINDIR)/deb-staging

# COMPILES ONLY FILES INSIDE TEST DIRECTORY
test: $(TESTOBJECTS)
	$(CC) $(CXXFLAGS) $^ -o $(TESTTARGET)

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(TESTBUILDDIR)
	$(CC) $(CXXFLAGS) $(INC) -c -o $@ $<

# REMOVE TEST DIRECTORY BINARIES
cleantest:
	@echo "Cleaning Test Directory..."
	@echo " $(RM) -r $(TESTBUILDDIR)"; $(RM) -r $(TESTBUILDDIR)
	@echo " $(RM) $(TESTTARGET)"; $(RM) $(TESTTARGET)

# EXPLICIT CLEAN
clean:
	@echo "Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR)"; $(RM) -r $(BUILDDIR)
	@echo " $(RM) -r $(TARGET)"; $(RM) -r $(TARGET)
	@echo " $(RM) -r $(BINDIR)/arm64_app"; $(RM) -r $(BINDIR)/arm64_app
	@echo " $(RM) -r $(BINDIR)/x86_app"; $(RM) -r $(BINDIR)/x86_app
	@echo " $(RM) -r $(BUNDLE)"; $(RM) -r $(BUNDLE)
	@echo " $(RM) -r $(BINDIR)/$(DMGNAME).dmg"; $(RM) -r $(BINDIR)/$(DMGNAME).dmg
	@echo " $(RM) -r $(BINDIR)/$(DEBNAME).deb"; $(RM) -r $(BINDIR)/$(DEBNAME).deb

# DELETE BIN DIRECTORY
cleanall:
	@echo "Cleaning..."
	@echo " $(RM) -r $(BUILDDIR) $(BINDIR)"; $(RM) -r $(BUILDDIR) $(BINDIR)


.PHONY: all, clean, cleanall, release, universal_app, x86_app, arm64_app, appbundle, appbundle_entry, appbundle_universal, dmg, dmg_entry, dmg_universal, deb, test, cleantest, probetest, protein
