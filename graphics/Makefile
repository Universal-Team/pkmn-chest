

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

TARGETS		:= $(INPUTS:.bmp=.gfx)

.PHONY	:	clean all directories pokemon

all	:	$(TARGETS) pokemon
	@echo graphics ...

no-pokemon	:	$(TARGETS)
	@echo graphics ...


%.gfx	:	%.bmp %.txt
	@mkdir -p $(OUT)/$(@D)
	@bmp2ds $< $(OUT)/$@ -o $(shell cat $(word 2,$^))

%.gfx	:	%.bmp
	@mkdir -p $(OUT)/$(@D)
	@bmp2ds $< $(OUT)/$@

clean	:
	@echo clean ...
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))

pokemon	:
	$(MAKE) -C pokemon
