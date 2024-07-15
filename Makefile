# Define the compiler
CXX = g++

# Define compiler flags
CXXFLAGS = -std=c++11 -I$(VCPKG_ROOT)/installed/$(TRIPLET)/include

# Define linker flags
LDFLAGS = -L$(VCPKG_ROOT)/installed/$(TRIPLET)/lib -lcurl -ljsoncpp

# Define the target executable
TARGET = test

# Define the source files
SRCS = server.cpp

# Define the object files
OBJS = $(SRCS:.cpp=.o)

# Define VCPKG variables (adjust the triplet to match your environment)
VCPKG_ROOT = /Users/lochiefrawley/js-projects/recipe/src/server/vcpkg
TRIPLET = x64-osx

# Define the build rule
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Define the rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Define the clean rule
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

