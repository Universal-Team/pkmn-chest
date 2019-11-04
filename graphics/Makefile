

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules


GRAPHICS	:=	graphics graphics/box \
				graphics/box/b2w2 graphics/box/bw graphics/box/dp graphics/box/hgss graphics/box/pt \
				lang/de lang/en lang/es lang/fr lang/it lang/jp lang/ko

OUT			:=	../nitrofiles

INPUTS		:= $(foreach dir,$(GRAPHICS),$(addprefix $(dir)/,$(notdir $(wildcard $(dir)/*.png))))
TARGETS		:= $(INPUTS:.png=.grf)

.PHONY: clean all directories

%.grf	:	%.png
	@mkdir -p $(OUT)/$(@D)
	# @grit $< -gu8 -gb -gB8 -ftr -fh! -Zl -o $(OUT)/$@
	@grit $< -gu8 -gb -gB8 -gT7C1F -ftr -fh! -o $(OUT)/$@

all	:	$(GRAPHICS) $(TARGETS)
	@echo GRIT files ...

clean:
	@echo clean ...
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))