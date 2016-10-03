  # the compiler: gcc for C program, define as g++ for C++
CC = mpicc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:
TARGET = q2
Q2 = q2
Q2 = q1

all: $(TARGET)

q2:	$(Q2)

q1: $(Q1)


$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lm

clean:
	$(RM) $(TARGET) $(Q2) $(Q1)