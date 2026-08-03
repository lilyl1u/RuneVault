CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude
TARGET = runevault
TEST_TARGET = basic_tests

SOURCES = $(wildcard src/*.cc)
OBJECTS = $(SOURCES:.cc=.o)
TEST_SOURCES = $(filter-out src/main.cc,$(SOURCES)) tests/basic_tests.cc
TEST_OBJECTS = $(TEST_SOURCES:.cc=.o)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

src/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET)

clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)
