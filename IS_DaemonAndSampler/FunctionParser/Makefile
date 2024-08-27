include ../pbvr.conf
include ../arch/${PBVR_MACHINE}
TARGET=libpbvrFunc.a
CXXFLAGS+=-DCPU_VER -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I../include -I./

FUNC_SRCS =    ExpressionTokenizer.cpp \
               ExpressionConverter.cpp \
               ReversePolishNotation.cpp

FUNC_HEADERS = ExpressionTokenizer.h \
               ExpressionConverter.h \
               ReversePolishNotation.h \
               Token.h

FUNC_OBJS   = $(FUNC_SRCS:%.cpp=%.o)

OBJS = $(FUNC_OBJS)

all: $(TARGET) 

$(TARGET): $(FUNC_OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(FUNC_OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(FUNC_OBJS) : $(FUNC_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)
