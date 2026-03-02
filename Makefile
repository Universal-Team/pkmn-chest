
BLOCKSDS	?= /opt/blocksds/core

NAME	:= pkmn-chest

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITROFSDIR		:= nitrofiles

# These set the information text in the nds file
GAME_TITLE		:= Pokemon Chest
GAME_SUBTITLE	:= 
GAME_AUTHOR 	:= Universal-Team
GAME_ICON 		:= icon.png

SOURCEDIRS	:= arm9/source arm9/core/source arm9/core/memecrypto
INCLUDEDIRS	:= arm9/include \
				arm9/core/memecrypto \
				arm9/core/swshcrypto \
				arm9/core/include \
				arm9/core/include/enums \
				arm9/core/include/i18n \
				arm9/core/include/personal \
				arm9/core/include/pkx \
				arm9/core/include/sav \
				arm9/core/include/utils \
				arm9/core/include/wcx \
				arm9/include/graphics \
				arm9/include/menus \
				arm9/include/saves \
				arm9/include/utils \
				arm9/include/utils/bank

# Libraries

LIBS		:= -lnds9 -lmm9
LIBDIRS		:= $(BLOCKSDS)/libs/maxmod

CXXFLAGS := -fexceptions -g -std=c++23

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile

CXXFLAGS += -fexceptions

SPECS		:= $(BLOCKSDS)/sys/crts/ds_arm9.specs

# If on a tagged commit, use just the tag
ifneq ($(shell echo $(shell git tag -l --points-at HEAD) | head -c 1),)
GIT_VER := $(shell git tag -l --points-at HEAD)
else
GIT_VER := $(shell git describe --tags --exclude git)
endif

# Ensure version.hpp exists
ifeq (,$(wildcard arm9/include/utils/version.hpp))
$(shell mkdir -p arm9/include/utils)
$(shell touch arm9/include/utils/version.hpp)
endif

# Print new version if changed
ifeq (,$(findstring $(GIT_VER), $(shell cat arm9/include/utils/version.hpp)))
$(shell printf "#ifndef VERSION_HPP\n#define VERSION_HPP\n\n#define VER_NUMBER \"$(GIT_VER)\"\n\n#endif\n" > arm9/include/utils/version.hpp)
endif

.PHONY	:	all skip-gs graphics lang cia sound clean format

#---------------------------------------------------------------------------------
graphics:
	$(MAKE) -C graphics

#---------------------------------------------------------------------------------
LANG_DIRS		:= chs cht eng fre ger ita jpn kor spa
LANG_FILES		:= abilities.txt balls.txt games.txt items*.txt locations*.txt moves.txt natures.txt species.txt
LANG_SOURCES	:= $(foreach lang,$(LANG_DIRS),$(foreach file,$(LANG_FILES),$(wildcard arm9/core/strings/$(lang)/$(file))))
LANG_TARGETS	:= $(subst arm9/core/strings,$(NITROFSDIR)/i18n,$(LANG_SOURCES))

$(NITROFSDIR)/i18n/%.txt	:	arm9/core/strings/%.txt
	@echo $$(basename $<)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@cp -f $< $@

lang	:	$(LANG_TARGETS)

#---------------------------------------------------------------------------------
sound:
	$(MAKE) -C sound

#---------------------------------------------------------------------------------

$(ROM): graphics sound lang $(NITRO_FILES) $(ELF)
	@printf $(GIT_VER) > nitrofiles/version.txt
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(ARM7ELF) -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		-g \#\#\#\# 00 "HOMEBREW" 87 -z 80040000 -a 00000138 \
		$(NDSTOOL_ARGS)

#---------------------------------------------------------------------------------
$(NAME).dsi	:	graphics sound lang $(NITRO_FILES) $(ELF)
	@printf $(GIT_VER) > nitrofiles/version.txt
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(ARM7ELF) -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		-g WPKA 00 "PKMN-CHEST" 87 -z 80040000 -u 00030004 -a 00000138 \
		$(NDSTOOL_ARGS)

#---------------------------------------------------------------------------------
$(NAME).cia	:	$(NAME).dsi $(ELF)
	make_cia --srl="pkmn-chest.dsi"

dsi	:	$(NAME).dsi
cia	:	$(NAME).cia

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	$(V)$(RM) $(ROM) $(DUMP) build $(SDIMAGE)
	@$(MAKE) -C graphics clean
	@$(MAKE) -C sound clean
	@rm -f $(LANG_TARGETS)
	@rm -f $(NAME).nds $(NAME).dsi $(NAME).cia
	@rm -f arm9/include/utils/version.hpp
	@rm -f nitrofiles/version.txt

format:
	make -C arm9 format
