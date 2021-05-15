TARGET = FourierDraw

OUT_DIR = Out
OBJ_DIR = Objs

OBJS += $(OBJ_DIR)/FourierDraw.o

CXX = clang++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -pthread -fopenmp -g 
CFLAGS += -O3 # optimization
CFLAGS += -DNDEBUG # no debug

SRC_DIR = Source
LDFLAGS += $(LIBS)

default: $(TARGET)
all: $(TARGET)

dirs:
	mkdir -p $(OUT_DIR)
	mkdir -p $(OBJ_DIR)

$(TARGET): dirs $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

DEPS = $(OBJS:%.o=%.d)
-include $(DEPS)

clean: 
	rm $(TARGET) || true
	rm -rf $(OUT_DIR) || true
	rm -rf $(OBJ_DIR) || true