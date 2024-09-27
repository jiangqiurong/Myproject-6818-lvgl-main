#
# Makefile
#
# CC ?= gcc

#把.0文件和.c文件放在一起
# CC = arm-linux-gcc
# LVGL_DIR_NAME ?= lvgl
# LVGL_DIR ?= ${shell pwd}
# # -Wshift-negative-value
# CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
# LDFLAGS ?= -lm -lpthread 
# BIN = demo

# prefix ?= /usr
# bindir ?= $(prefix)/bin

# LV_PROJECT_SRC_DIR = ./my_lvgl_test/src
# LV_PROJECT_INC_DIR = ./my_lvgl_test/inc
# IMG_ARR_DIR = $(LV_PROJECT_SRC_DIR)/img_arr


# #Collect the files to compile
# MAINSRC = ./main.c 
# #./my_lvgl_test/src/myplay.c ./my_lvgl_test/src/show_dir.c ./my_lvgl_test/src/lv_font_SiYuanRouHei_Regular_16_cjk.c ./my_lvgl_test/src/login.c ./my_lvgl_test/src/document.c ./my_lvgl_test/src/weather.c ./my_lvgl_test/src/cJSON.c ./my_lvgl_test/src/music.c ./my_lvgl_test/src/img_arr/icon_music.c ./my_lvgl_test/src/client.c ./my_lvgl_test/src/img_arr/.c

# LV_PROJECT_CSRCS = $(wildcard $(LV_PROJECT_SRC_DIR)/*.c)
# IMG_ARR_CSRCS = $(wildcard $(IMG_ARR_DIR)/*.c)
# # include $(LVGL_DIR)/lvgl/lvgl.mk
# # include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

# CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

# CSRCS += $(LV_PROJECT_CSRCS) $(IMG_ARR_CSRCS)
# CFLAGS += -I$(LV_PROJECT_INC_DIR)

# OBJEXT ?= .o

# AOBJS = $(ASRCS:.S=$(OBJEXT))
# COBJS = $(CSRCS:.c=$(OBJEXT))

# MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

# SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
# OBJS = $(AOBJS) $(COBJS)

# ## MAINOBJ -> OBJFILES

# all: default

# %.o: %.c
# 	@$(CC)  $(CFLAGS) -c $< -o $@
# 	@echo "CC $<"
    
# default: $(AOBJS) $(COBJS) $(MAINOBJ)
# 	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS) -llvgl -L. 
# #-I ./my_lvgl_test

# clean: 
# 	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

# install:
# 	install -d $(DESTDIR)$(bindir)
# 	install $(BIN) $(DESTDIR)$(bindir)

# uninstall:
# 	$(RM) -r $(addprefix $(DESTDIR)$(bindir)/,$(BIN))

# 编译器设置
#把生成的.o文件放在build文件夹下
CC = arm-linux-gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wstack-usage=2048
LDFLAGS ?= -lm -lpthread 
BIN = demo

# 安装路径设置
prefix ?= /usr
bindir ?= $(prefix)/bin

# 项目路径设置
LV_PROJECT_SRC_DIR = ./my_lvgl_test/src
LV_PROJECT_INC_DIR = ./my_lvgl_test/inc
IMG_ARR_DIR = $(LV_PROJECT_SRC_DIR)/img_arr

# 收集源文件
MAINSRC = ./main.c 
LV_PROJECT_CSRCS = $(wildcard $(LV_PROJECT_SRC_DIR)/*.c)
IMG_ARR_CSRCS = $(wildcard $(IMG_ARR_DIR)/*.c)
CSRCS = $(LVGL_DIR)/mouse_cursor_icon.c 
CSRCS += $(LV_PROJECT_CSRCS) $(IMG_ARR_CSRCS)
CFLAGS += -I$(LV_PROJECT_INC_DIR)

# 对象文件设置
OBJEXT ?= .o
BUILD_DIR = ./my_lvgl_test/build

# 去除路径前的./
stripdot = $(patsubst ./%,%,$(1))

# 定义对象文件列表
COBJS = $(addprefix $(BUILD_DIR)/,$(call stripdot,$(CSRCS:.c=$(OBJEXT))))
MAINOBJ = $(addprefix $(BUILD_DIR)/,$(call stripdot,$(MAINSRC:.c=$(OBJEXT))))
OBJS = $(COBJS)

# 目标规则
all: default

# 编译规则，生成的.o文件放在build目录下
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

# 默认规则，链接生成可执行文件
default: $(OBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(OBJS) $(LDFLAGS) -llvgl -L.

# 清理生成的文件
clean: 
	rm -f $(BIN)
	rm -rf $(BUILD_DIR)

# 安装规则
install:
	install -d $(DESTDIR)$(bindir)
	install $(BIN) $(DESTDIR)$(bindir)

# 卸载规则
uninstall:
	$(RM) -r $(addprefix $(DESTDIR)$(bindir)/,$(BIN))
