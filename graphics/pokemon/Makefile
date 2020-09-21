

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	.

GFX_OUT		:=	../pokemon-gfx
COMBO_OUT	:=	../../nitrofiles/graphics

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))
TARGETS		:= $(foreach file,$(INPUTS:.bmp=.gfx),$(addprefix $(GFX_OUT)/, $(file)))

.PHONY	:	clean all

all	:	$(TARGETS) $(COMBO_OUT)/pokemon.combo.gfx

$(COMBO_OUT)/pokemon.combo.gfx	:
	@cat $(GFX_OUT)/*.gfx > $(COMBO_OUT)/pokemon.combo.gfx

$(GFX_OUT)/%.gfx	:	%.bmp
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@bmp2ds $< $@

clean	:
	@echo clean ...
	@rm -f $(TARGETS)
	@rm -f $(COMBO_OUT)/pokemon.combo.gfx
