

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

.PHONY	:	bmp2ds clean all directories

all	:	bmp2ds $(TARGETS)
	@echo graphics ...

bmp2ds	:
	@g++ bmp2ds.cpp -o bmp2ds

%.gfx	:	%.bmp
	@mkdir -p $(OUT)/$(@D)
	@./bmp2ds $< $(OUT)/$@

clean	:
	@echo clean ...
	@rm bmp2ds
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))