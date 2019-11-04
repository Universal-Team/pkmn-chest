

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	. box box/b2w2 box/bw box/dp box/hgss box/pt

OUT			:=	../nitrofiles/graphics/grit

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.png))))
TARGETS		:= $(INPUTS:.png=.grf)

.PHONY: clean all directories

%.grf	:	%.png
	@mkdir -p $(OUT)/$(@D)
	@grit $< -gu8 -gb -gB8 -ftr -fh! -Zl -o $(OUT)/$@

all	:	$(GRAPHICS) $(TARGETS)
	@echo GRIT files ...

clean:
	@echo clean ...
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))