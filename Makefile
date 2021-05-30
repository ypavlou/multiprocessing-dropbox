#
# In order to execute this "Makefile" just type "make"
#

OBJS    = createFifos.o main.o FindAllFiles.o
SOURCE  = createFifos.c main.c FindAllFiles.c
HEADER  = createFifos.h FindAllFiles.h
OUT     = sys2

CC      = gcc
FLAGS   = -g -c 
# -g option enables debugging mode
# -c flag generates object code for separate files

all: 	$(OBJS)
		$(CC) -g $(OBJS) -o $(OUT)

# create/compile the individual files >>separately<<
main.o: main.c
	$(CC) $(FLAGS) main.c

createFifos.o: createFifos.c
	$(CC) $(FLAGS) createFifos.c

FindAllFiles.o: FindAllFiles.c
	$(CC) $(FLAGS) FindAllFiles.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
