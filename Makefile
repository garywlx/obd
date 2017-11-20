CXX	= g++
CXXFLAGS = -Wall -O2 -m32
TARGET     = obd_server
SRCDIR=	src
OBJDIR=	obj
BINDIR=	bin
DSTDIR=	/obd
INCDIR=	-Iinc -I./inc -I/usr/include
LIBS= -L/usr/local/lib -levent -levent_pthreads -lpthread

$(shell install -d $(BINDIR) $(OBJDIR))

SRC=	$(wildcard $(SRCDIR)/*.cpp)
OBJ=	$(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
BIN=	$(BINDIR)/$(TARGET)

$(TARGET) : $(OBJ)
	@echo 'Creating binary file:  $(BIN)...'
	@$(CXX) $(CXXFLAGS) $(INCDIR) $(LIBS) -o $(BIN) $(OBJ)
	@echo 'Done.'

$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp
	@echo 'Compiling object file: $@...'
	@$(CXX) $(CXXFLAGS) $(INCDIR) $(LIBS) -c -o $@ $<

install:
	cp $(BIN) $(DSTDIR)/$(TARGET)

.PHONY: clean
clean:
	@echo 'Removing all objects...'
	@rm -f $(OBJ) $(BIN) $(DSTDIR)/$(TARGET)
	@echo 'Done.'
