#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> //file_open,close
#include <unistd.h>//file_read,write

//file mv assignment using low-level file input

void error_handling(char* message);

int main(int argc, char* argv[])
{
	int f1d;
	int f2d;
	int sizer = 0;
	int sizew = 0;
	int total = 0;
	int r;
	char buf[5];

	if(argc!=3) //checking inputs are ok or not
		error_handling("[Error] mymove Usage: ./mymove src_file dest_file");
	
	f1d =  open(argv[1], O_RDONLY); //f1 is only for reading
	if(f1d==-1)
		 error_handling("f1d open() error");
	
	f2d = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644); //f2 is new file for writing 
	if(f2d==-1)
		error_handling("f2d open() error");

	//moving files process
	while((sizer = read(f1d, buf, sizeof(buf)-1))>0)
	{
		sizew = write(f2d, buf, sizer);
		if(sizer!=sizew) //checking sizes are same or not
			error_handling("sizes are diff.");
		memset(buf, 0, 5);
		total += sizer;
	}
	close(f1d); //reading process is over
	close(f2d); //writing process is over
	
	r = remove(argv[1]); //remove files

	printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], total); //print result
	
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

