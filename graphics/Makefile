

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	graphics graphics/box \
				graphics/box/b2w2 graphics/box/bw graphics/box/dp graphics/box/hgss graphics/box/pt \
				lang/de lang/en lang/es lang/fr lang/it lang/jp lang/ko

OUT			:=	../nitrofiles

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))
TARGETS		:= $(INPUTS:.bmp=.gfx)

BMP2DS		:= ../resources/bmp2ds

.PHONY: clean all directories

%.gfx	:	%.bmp
	@mkdir -p $(OUT)/$(@D)
	@$(BMP2DS) $< $(OUT)/$@

all	:	$(TARGETS)
	@echo GRIT files ...

clean:
	@echo clean ...
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))