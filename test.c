#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 1024
int main()
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	char *filename = "/dev/chardev";
	int fd = open(filename, O_RDONLY, mode);
	if (fd == -1) {
		fprintf(stderr, "Cannot open %s\n", filename);
		exit(1);
	}
	fprintf(stdout, "start reading...\n");
	char buf[BUF_SIZE];
	int len;
	while ((len = read(fd, buf, BUF_SIZE)) > 0) {
		write(1, buf, len);
		write(fd, buf, len);
	}
	close(fd);
	fprintf(stdout, "close fd\n");
	return 0;
}
