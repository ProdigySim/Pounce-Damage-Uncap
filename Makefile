# (C)2004-2010 Metamod:Source Development Team
# Makefile written by David "BAILOPAN" Anderson

###########################################
### EDIT THESE PATHS FOR YOUR OWN SETUP ###
###########################################

HL2SDKL4D2 ?= ../../hl2sdk-l4d2
MMSOURCE18 ?= ..

#####################################
### EDIT BELOW FOR OTHER PROJECTS ###
#####################################

PROJECT = pounce_damage_uncap
OBJECTS = pounce_damage_uncap.cpp memutils.cpp

##############################################
### CONFIGURE ANY OTHER FLAGS/OPTIONS HERE ###
##############################################

OPT_FLAGS = -O3 -funroll-loops -s -pipe
GCC4_FLAGS = -fvisibility=hidden -fvisibility-inlines-hidden
DEBUG_FLAGS = -g -ggdb3 -D_DEBUG
CPP = gcc

##########################
### SDK CONFIGURATIONS ###
##########################

ENGINE=left4dead2
HL2SDK = $(HL2SDK_L4D2)
CFLAGS += -DSOURCE_ENGINE=6

HL2PUB = $(HL2SDK)/public

INCLUDE += -I$(HL2SDK)/public/game/server
METAMOD = $(MMSOURCE18)/core

OS := $(shell uname -s)

ifeq "$(OS)" "Darwin"
	LIB_EXT = dylib
	HL2LIB = $(HL2SDK)/lib/mac
else
	LIB_EXT = so
	ifeq "$(ENGINE)" "original"
		HL2LIB = $(HL2SDK)/linux_sdk
	else
		HL2LIB = $(HL2SDK)/lib/linux
	endif
endif

LIB_PREFIX = lib
LIB_SUFFIX = .$(LIB_EXT)

CFLAGS += -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_ORANGEBOXVALVE=4 \
	-DSE_LEFT4DEAD=5 -DSE_LEFT4DEAD2=6 -DSE_ALIENSWARM=7

LINK += $(HL2LIB)/tier1_i486.a $(LIB_PREFIX)vstdlib$(LIB_SUFFIX) $(LIB_PREFIX)tier0$(LIB_SUFFIX) \
	-static-libgcc

INCLUDE += -I. -I.. -I$(HL2PUB) -I$(HL2PUB)/engine -I$(HL2PUB)/mathlib -I$(HL2PUB)/vstdlib \
	-I$(HL2PUB)/tier0 -I$(HL2PUB)/tier1 -I. -I$(METAMOD) -I$(METAMOD)/sourcehook

################################################
### DO NOT EDIT BELOW HERE FOR MOST PROJECTS ###
################################################

BINARY = $(PROJECT).$(LIB_EXT)

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug.$(ENGINE)
	CFLAGS += $(DEBUG_FLAGS)
else
	BIN_DIR = Release.$(ENGINE)
	CFLAGS += $(OPT_FLAGS)
endif

GCC_VERSION := $(shell $(CPP) -dumpversion >&1 | cut -b1)

ifeq "$(OS)" "Darwin"
	LIB_EXT = dylib
	CFLAGS += -isysroot /Developer/SDKs/MacOSX10.5.sdk
	LINK += -dynamiclib -lstdc++ -mmacosx-version-min=10.5
else
	LIB_EXT = so
	CFLAGS += -D_LINUX
	LINK += -shared
endif

CFLAGS += -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp \
	-Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca \
	-Dstrcmpi=strcasecmp -Wall -Wno-non-virtual-dtor -Werror -fPIC -fno-exceptions \
	-fno-rtti -msse -m32 -fno-strict-aliasing

ifeq "$(GCC_VERSION)" "4"
	CFLAGS += $(GCC4_FLAGS)
endif

OBJ_BIN := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) -o $@ -c $<

all:
	mkdir -p $(BIN_DIR)
	ln -sf $(HL2LIB)/$(LIB_PREFIX)vstdlib$(LIB_SUFFIX)
	ln -sf $(HL2LIB)/$(LIB_PREFIX)tier0$(LIB_SUFFIX)
	$(MAKE) -f Makefile stub_mm

stub_mm: $(OBJ_BIN)
	$(CPP) $(INCLUDE) -m32 $(OBJ_BIN) $(LINK) -ldl -lm -o $(BIN_DIR)/$(BINARY)

default: all

clean:
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/$(BINARY)

