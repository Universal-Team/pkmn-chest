

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	graphics graphics/box graphics/box/b2w2 graphics/box/bw graphics/box/dp graphics/box/hgss graphics/box/pt \
				lang/br/types lang/de/types lang/en/types lang/es/types lang/fr/types lang/it/types lang/jp/types lang/ko/types

OUT			:=	../nitrofiles

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))

TARGETS		:= $(INPUTS:.bmp=.gfx)

.PHONY	:	bmp2ds clean all directories pokemon

no-pokemon	:	bmp2ds $(TARGETS)
	@echo graphics ...

all	:	bmp2ds $(TARGETS) pokemon
	@echo graphics ...

bmp2ds	:
	@g++ bmp2ds.cpp -o bmp2ds


%.gfx	:	%.bmp %.txt
	@mkdir -p $(OUT)/$(@D)
	@./bmp2ds $< $(OUT)/$@ -o $(shell cat $(word 2,$^))

%.gfx	:	%.bmp
	@mkdir -p $(OUT)/$(@D)
	@./bmp2ds $< $(OUT)/$@

clean	:
	@echo clean ...
	@rm -f bmp2ds
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))

pokemon	:
	$(MAKE) -C pokemon
