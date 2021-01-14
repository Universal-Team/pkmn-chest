#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

# If on a tagged commit, use the tag instead of the commit
ifneq ($(shell echo $(shell git tag -l --points-at HEAD) | head -c 1),)
GIT_VER := $(shell git tag -l --points-at HEAD)
else
GIT_VER := $(shell git describe --abbrev=0 --tags)-$(shell git rev-parse --short HEAD)
endif

# Ensure version.hpp exists
ifeq (,$(wildcard include/utils/version.hpp))
$(shell mkdir -p include/utils)
$(shell touch include/utils/version.hpp)
endif

# Print new version if changed
ifeq (,$(findstring $(GIT_VER), $(shell cat include/utils/version.hpp)))
$(shell printf "#ifndef VERSION_HPP\n#define VERSION_HPP\n\n#define VER_NUMBER \"$(GIT_VER)\"\n\n#endif\n" > include/utils/version.hpp)
endif

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# MUSIC contains a directory of music and sound effect files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	source \
				Universal-Core/source \
				core/memecrypto core/swshcrypto \
				core/source core/source/i18n core/source/personal core/source/pkx core/source/sav core/source/utils core/source/wcx \
				source/graphics \
				source/menus \
				source/saves \
				source/utils source/utils/bank
INCLUDES	:=	include \
				Universal-Core/include \
				core/memecrypto core/swshcrypto \
				core/include core/include/enums core/include/i18n core/include/personal core/include/pkx core/include/sav core/include/utils core/include/wcx \
				include/graphics \
				include/menus \
				include/saves \
				include/utils include/utils/bank
FORMAT_SOURCES	:=	source \
					source/graphics \
					source/menus \
					source/saves \
					source/utils
FORMAT_INCLUDES	:=	include \
					include/graphics \
					include/menus \
					include/saves \
					include/utils

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb-interwork

CFLAGS	:=	-g -Wall -Wno-psabi -O2\
 			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9
# Exeptions needed on for swshcrypto
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fexceptions -std=gnu++20 -Wno-volatile

ASFLAGS	:=	-g $(ARCH) -march=armv5te -mtune=arm946e-s

LDFLAGS	=	-specs=../ds_arm9_pkch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lfat -lmm9 -lnds9

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export ARM9ELF	:=	$(CURDIR)/$(TARGET).elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all $(BUILD) clean

all: $(BUILD)

#---------------------------------------------------------------------------------
$(BUILD):
	@printf $(GIT_VER) > ../nitrofiles/version.txt
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -f include/utils/version.hpp
	@rm -fr $(BUILD) *.elf *.nds* *.bin
	@rm -f ../nitrofiles/version.txt

#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9ELF)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

cppcheck:
	@rm -f ../cppcheck.log
	@cppcheck . --enable=all $(INCLUDE) -UJSON_CATCH_USER -U_Check_return_ -U_MSC_VER -U_Ret_notnull_ -U__INTEL_COMPILER -U__PGI -U__SUNPRO_CC -UJSON_INTERNAL_CATCH_USER -UJSON_THROW_USER -UJSON_TRY_USER -U__IBMCPP__ -U__SUNPRO_CC -D__GNUC__=9 -D__GNUC_MINOR__=1 -DNULL=nullptr --force 2> ../cppcheck.log
	@echo build/cppcheck.log file created...

format:
	clang-format -i -style=file $(foreach dir,$(FORMAT_SOURCES),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.cpp) $(wildcard $(dir)/*.hpp) $(wildcard $(dir)/*.h)) $(foreach dir,$(FORMAT_INCLUDES),$(wildcard $(dir)/*.h) $(wildcard $(dir)/*.hpp))