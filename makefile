CC = g++

SDIR = src
IDIR = inc
ODIR = obj
BDIR = bin

LIBS = -lgphoto2 -lgphoto2_port -lGL -lglfw -lGLEW -lMagick++ -lcurl

CFLAGS = -Wall -I$(IDIR) -I/usr/local/include -I/usr/include -pthread

SRC = $(wildcard $(SDIR)/*.cpp)
OBJ := $(addprefix $(ODIR)/,$(notdir $(SRC:.cpp=.o)))

build: $(BDIR) $(OBJ)
	$(CC) -o $(BDIR)/photobooth $(filter-out $<,$^) $(CFLAGS) $(LIBS)

clean:
	rm -r -f bin obj

$(ODIR):
	mkdir $@

$(BDIR):
	mkdir $@

$(ODIR)/%.o: $(SDIR)/%.cpp $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)
