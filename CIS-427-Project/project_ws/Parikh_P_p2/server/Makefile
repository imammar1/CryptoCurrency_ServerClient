TARGET  := server.out
EXT     := c cpp

# Directories 
SRC_DIR := ./src
OBJ_DIR := ./obj
DEP_DIR := ./dep
INC_DIR := ./include
# (12)Include files add together a prefix, gcc make sense that -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIR)) -I $(BOOST_ROOT)

# C compiler settings
CC       := gcc
CFLAGS   := -Wall -pthread -lm -c # -Wall -Wextra
CLIBS    := -lm 

# C++ compiler settingss
CXX := g++
CXXFLAGS := -std=c++11 $(CFLAGS)
CPPFLAGS := $(INC_FLAGS) -MMD -MP -MF $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

# Linker flags
LDFLAGS  := -pthread # -ldl
LDLIBS   := -L $(BOOST_LIB) -lm

SOURCE := $(foreach ext, $(EXT), $(wildcard $(SRC_DIR)/*.$(ext)))
OBJECT := $(SOURCE:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)
#DEPEND := $(OBJECT:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

define rule =
$(OBJ_DIR)/%.$(1).o: $(SRC_DIR)/%.$(1) | $(OBJ_DIR) $(DEP_DIR)
	$$(COMPILE.$(1)) $$< -o $$@
endef

.PHONY: all clean

all: $(TARGET)

# Old line 44 replace if need to fix
# $(CXX) $(LDFLAGS) $(LDLIBS) $^ -o $@ -ldl

$(TARGET): $(OBJECT)
	$(CXX) $(LDFLAGS) $(LDLIBS) $^ -o $@ -ldl
	$(RM) ./-c

$(foreach ext, $(EXT), $(eval $(call rule,$(ext))))

$(OBJ_DIR) $(DEP_DIR):
	mkdir -p $@

-include $(DEPEND)

clean:
	$(RM) -r $(TARGET) $(OBJ_DIR) $(DEP_DIR)
