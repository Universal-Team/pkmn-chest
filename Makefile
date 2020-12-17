#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET	:= pkmn-chest
export TOPDIR	:= $(CURDIR)

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES		:= nitrofiles

# These set the information text in the nds file
GAME_TITLE		:= Pokemon Chest
GAME_SUBTITLE1	:= Universal-Team

include $(DEVKITARM)/ds_rules

.PHONY	:	all skip-gs checkarm9 graphics lang cia sound clean format

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:	checkarm9 $(TARGET).nds

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
graphics:
	$(MAKE) -C graphics

#---------------------------------------------------------------------------------
LANG_DIRS		:= chs cht eng fre ger ita jpn kor spa
LANG_FILES		:= abilities.txt balls.txt games.txt items*.txt locations*.txt moves.txt natures.txt species.txt
LANG_SOURCES	:= $(foreach lang,$(LANG_DIRS),$(foreach file,$(LANG_FILES),$(wildcard arm9/core/strings/$(lang)/$(file))))
LANG_TARGETS	:= $(subst arm9/core/strings,$(NITRO_FILES)/i18n,$(LANG_SOURCES))

$(NITRO_FILES)/i18n/%.txt	:	arm9/core/strings/%.txt
	@echo $$(basename $<)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@cp -f $< $@

lang	:	$(LANG_TARGETS)

#---------------------------------------------------------------------------------
sound:
	$(MAKE) -C sound

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds	: graphics sound lang $(NITRO_FILES) arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -9 arm9/$(TARGET).elf \
			-b1 icon.bmp "$(GAME_TITLE);$(GAME_SUBTITLE1)" $(_ADDFILES) \
			-g \#\#\#\# 00 "HOMEBREW" 87 -z 80040000 -u 00030004 -a 00000138

#---------------------------------------------------------------------------------
$(TARGET).dsi	:	arm9/$(TARGET).elf
	ndstool	-c $(TARGET).dsi -9 arm9/$(TARGET).elf \
	-b1 icon.bmp "$(GAME_TITLE);$(GAME_SUBTITLE1)" $(_ADDFILES) \
	-g WPKA 00 "PKMN-CHEST" 87 -z 80040000 -u 00030004 -a 00000138

#---------------------------------------------------------------------------------
$(TARGET).cia	:	$(TARGET).dsi arm9/$(TARGET).elf
	make_cia --srl="pkmn-chest.dsi"

dsi	:	$(TARGET).dsi
cia	:	$(TARGET).cia

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C graphics clean
	@$(MAKE) -C sound clean
	@rm -f $(LANG_TARGETS)
	@rm -f $(TARGET).nds $(TARGET).arm9

format:
	make -C arm9 format
