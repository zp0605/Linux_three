#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define EV_KEY			0x01
#define KEY_L			38

struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	int  value;
};


/* power <dev> */
int main(int argc, char **argv)
{
	int fd;
	int ret;
	struct input_event event;
	
	if (argc != 2)
	{
		printf("Usage:\n");
		printf("%s <dev>\n", argv[0]);
		return -1;
	}
	
	/* open */
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		printf("can't open %s\n", argv[1]);
		return -1;
	}

	while (1)
	{
		/* read */
		ret = read(fd, &event, sizeof(event));
		if (ret != sizeof(event))
		{
			printf("can't read: ret = %d\n", ret);
			return -1;
		}

		/* °´¼üÀà, L¼ü, ËÉ¿ª */	
		if (event.type == EV_KEY && event.code == KEY_L && event.value == 0)
		{
			/* echo mem > /sys/power/state */
			system("echo mem > /sys/power/state");
		}
	}
	return 0;
}

