

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	graphics graphics/box graphics/box/b2w2 graphics/box/bw graphics/box/dp graphics/box/hgss graphics/box/pt lang/*/*

OUT			:=	../nitrofiles

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.bmp))))

TARGETS		:= $(INPUTS:.bmp=.gfx)

.PHONY	:	bmp2ds clean all directories pokemon

all	:	bmp2ds $(TARGETS) pokemon
	@echo graphics ...

bmp2ds	:
	@g++ bmp2ds.cpp -o bmp2ds


%.gfx	:	%.bmp %.txt
	@mkdir -p $(OUT)/$(@D)
	@./bmp2ds $< $(OUT)/$@ $(shell cat $(word 2,$^))

%.gfx	:	%.bmp
	@mkdir -p $(OUT)/$(@D)
	@./bmp2ds $< $(OUT)/$@

clean	:
	@echo clean ...
	@rm -f bmp2ds
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))

pokemon	:
	$(MAKE) -C pokemon
