/*
	PIN number generator for Interview.
	Uses a coprime of 10000 to map numbers from 0 to 9999 to
	unique numbers in that same range pseudorandomly.

	The numbers emitted excludes the set: {x | (x % 1111) = 0 } (so, 0000 1111 2222 3333 4444 5555 6666 7777 8888 9999)

	We use a (sizeof(int)*2)-byte file to store the state: (index, offset)
		index - the index of the last number emitted
		offset - the offset which defines the mapping in combination with the magic numbers.
*/


#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

const int prime = 6277;	// A nice number, coprime with 10000

const char fileName[] = "randomFile.bin";

const char intSize = sizeof(int);

const int fileError = 1;
const int timeError = 2;

// Does the file already exist?
int fileExists() {
	struct stat s;
	return stat(fileName,&s) != -1;
}

// Open file, return randomFile fd
int openRandomFile() {
	int fd = open(fileName, O_RDWR | O_CREAT,0700);
	if(fd < 0) {
		perror("Unable to open file");
		exit(fileError);
	}
	return fd;
}

// Store int to file
void store(int *index, int *salt) {
	int fd = openRandomFile();
	write(fd, index,intSize);
	write(fd,salt,intSize);
	close(fd);
}

// Load int from file
void load(int *index, int *salt) {
	int fd = openRandomFile();
	int pin;
	read(fd, index, intSize);
	read(fd, salt, intSize);
	close(fd);
}

// Grab the time of day
int makeSalt() {
	struct timeval tvs;
	if(gettimeofday(&tvs,NULL)) {
		perror("Computer wouldn't give me the time of day");
		exit(timeError);
	}

	return tvs.tv_sec;

}

// Get the next 'random' number
int randomFor(int salt, int index) {
	return (salt + index * prime) % 10000;
}

// Is it a 'good' PIN?
int validPin(int pin) {
	return (pin % 1111 > 0);	// Picks up and rejects 1111, 2222, 3333,  etc..
}

/**
Usage:
./makePIN [number]
default number is 1
*/
int main(int argc, char *argv[]) {
	int count = 1;
	if(argc > 1) {
		count = atoi(argv[1]);
	}

	// To store our PINs before we emit them
	int pins[count];
	
	// These two ints make up our state
	int pIndex, salt;

	// If the file exists, load state from there
	if(fileExists()) {
		load(&pIndex, &salt);
	} else {
		pIndex = 9999;
	}

	// Actually generate the PINs
	int i;
	for(i = 0; i < count; i++) {
		int valid = 0;
		while(!valid) {
			if(pIndex == 9999) {		// If we covered the last number in the set, reset with new salt
				salt = makeSalt();
				pIndex = 0;
			} else {
				pIndex++;
			}
			valid = validPin(pins[i] = randomFor(salt, pIndex));
		}
	}

	// Save new state to disk before emitting
	store(&pIndex, &salt);

	// Emit
	for(i = 0; i < count; i++) {
		printf("%04d\n", pins[i]);
	}

	return 0;
}
