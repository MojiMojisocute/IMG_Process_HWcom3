CXX      = g++
CXXFLAGS = -O3 -march=native -std=c++17 -Wall -Wextra
INCS     = -Iinclude
TARGET   = wormhole
SRCDIR   = src
OBJDIR   = obj

SRCS = $(SRCDIR)/main.cpp \
       $(SRCDIR)/Image.cpp \
       $(SRCDIR)/TIFFReader.cpp \
       $(SRCDIR)/Preprocessing.cpp \
       $(SRCDIR)/TIFFWriter.cpp \
       $(SRCDIR)/CCL.cpp

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(OBJDIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild