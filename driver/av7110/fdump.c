#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


static
void fdump (char *name)
{
	unsigned char buf [8];
	int i, fd, count, bytes;

	if ((fd = open (name, O_RDONLY)) < 0) {
		perror ("open");
		exit (-1);
	}

	printf ("\nu8 %s [] __initdata = {", name);

	while ((count = read (fd, buf, sizeof(buf))) > 0) {
		printf ("\n\t");
		for (i=0;i<count;i++, bytes++)
			printf ("0x%02x, ", buf[i]);
	}

	printf ("\n};\n\n");
	close (fd);
}


int main (void)
{
	printf ("\n#include <asm/types.h>\n");

	fdump ("Dpram");
	fdump ("Root");

	return 0;
}

