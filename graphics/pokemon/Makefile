

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	.

GFX_OUT		:=	../pokemon-gfx
COMBO_OUT	:=	../../nitrofiles/graphics

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))
TARGETS		:= $(INPUTS:.bmp=.gfx)

.PHONY	:	combo clean all directories

all	:	$(TARGETS) combo
	@echo pokemon ...

combo	:
	@cat $(GFX_OUT)/*.gfx > $(COMBO_OUT)/pokemon.combo.gfx

%.gfx	:	%.bmp
	@mkdir -p $(GFX_OUT)/$(@D)
	@bmp2ds $< $(GFX_OUT)/$@ -c 16

clean	:
	@echo clean ...
	@rm -rf $(addprefix $(GFX_OUT)/,$(TARGETS))
