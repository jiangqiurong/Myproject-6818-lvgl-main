SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

CC=gcc  #Makefile内置变量用来存放编译器的名字，默认等于gcc

TARGET=server #自定义变量用来存放目标程序名字


$(TARGET):$(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -lpthread

%.o:%.c
	$(CC) -c $(^) -o $(@) 

clean:
	rm $(OBJS) $(TARGET)