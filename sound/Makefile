MUSIC		:=	.
OUT			:=	../nitrofiles/sound

WAVFILES	:=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.wav)),$(MUSIC)/$(dir))
MODFILES	:=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.mod)),$(MUSIC)/$(dir))
MODFILES	+=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.s3m)),$(MUSIC)/$(dir))
MODFILES	+=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.it)),$(MUSIC)/$(dir))
MODFILES	+=	$(foreach dir,$(notdir $(wildcard $(MUSIC)/*.xm)),$(MUSIC)/$(dir))

TARGETS		:= $(MODFILES:.mod=.msl)
TARGETS		+= $(MODFILES:.s3m=.msl)
TARGETS		+= $(MODFILES:.it=.msl)
TARGETS		+= $(MODFILES:.xm=.msl)

all	:	sfx.msl $(TARGETS)
	@echo sound ...

#---------------------------------------------------------------------------------
# rule to build soundbank from music files
#---------------------------------------------------------------------------------
sfx.msl	:	$(WAVFILES)
#---------------------------------------------------------------------------------
	@mkdir -p $(OUT)/$(@D)
	@mmutil $^ -d -o$(OUT)/sfx.msl

#---------------------------------------------------------------------------------
%.msl	:	$(WAVFILES) %.mod
#---------------------------------------------------------------------------------
	@mkdir -p $(OUT)/$(@D)
	@mmutil $^ -d -o$(OUT)/$@

#---------------------------------------------------------------------------------
%.msl	:	$(WAVFILES) %.s3m
#---------------------------------------------------------------------------------
	@mkdir -p $(OUT)/$(@D)
	@mmutil $^ -d -o$(OUT)/$@

#---------------------------------------------------------------------------------
%.msl	:	$(WAVFILES) %.it
#---------------------------------------------------------------------------------
	@mkdir -p $(OUT)/$(@D)
	@mmutil $^ -d -o$(OUT)/$@

#---------------------------------------------------------------------------------
%.msl	:	$(WAVFILES) %.xm
#---------------------------------------------------------------------------------
	@mkdir -p $(OUT)/$(@D)
	@mmutil $^ -d -o$(OUT)/$@

clean	:
	@echo clean ...
	@rm -f bmp2ds
	@rm -rf $(OUT)/sfx.msl
	@rm -rf $(addprefix $(OUT)/,$(TARGETS))
