

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

GRAPHICS	:=	graphics \
				graphics/ball \
				graphics/box graphics/box/b2w2 graphics/box/bw graphics/box/dp graphics/box/e graphics/box/frlg graphics/box/hgss graphics/box/pt graphics/box/rs \
				i18n/brh/types i18n/eng/types i18n/fre/types i18n/ger/types i18n/ita/types i18n/jpn/types i18n/kor/types i18n/spa/types

OUT			:=	../nitrofiles

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))

TARGETS		:= $(foreach file,$(INPUTS:.bmp=.gfx),$(addprefix $(OUT)/, $(file)))

.PHONY	:	clean all directories pokemon

all	:	$(TARGETS) pokemon

no-pokemon	:	$(TARGETS)

$(OUT)/%.gfx	:	%.bmp %.txt
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@bmp2ds $< $@ -o $(shell cat $(word 2,$^))

$(OUT)/%.gfx	:	%.bmp
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@bmp2ds $< $@

clean	:
	@echo clean ...
	@rm -f $(TARGETS)
	@make -C pokemon clean

pokemon	:
	$(MAKE) -C pokemon
