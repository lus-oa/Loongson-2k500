
const unsigned char BCD_decode_tab[ 29 ] = { 0X3F, 0X06, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F, 0X77, 0X7C, 0X58, 0X5E, 0X79, 0X71, 0x00, 0x46, 0x40, 0x41, 0x39, 0x0F, 0x08, 0x76, 0x38, 0x73, 0x80, 0xFF, 0x00 };

int count_max = 9999, count_curr = 0;
pthread_mutex_t count_max_lock, count_curr_lock;
unsigned char databuf[5];

void set_digital_tube(int fd, int count);
void reset_digital_tube(int fd);
void get_mutex_lock(pthread_mutex_t *lock);
void release_mutex_lock(pthread_mutex_t *lock);
void set_count_max(int count);
int get_count_max(void);
void set_count_curr(int curr);
int get_count_curr(void);

void *countdown(void *arg)
{
	//收到cancel信号，取消此线程
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //立即取消此线程
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int count_curr_tmp;
	int fd = *((int *)arg);
	databuf[0] = 0x0;

	printf("Countdown thread is running\n");
	while (1)
	{
		get_mutex_lock(&count_max_lock);
		get_mutex_lock(&count_curr_lock);

		if (count_curr > count_max)
		{
			count_curr = 0;
		}
		count_curr_tmp = count_curr;
		count_curr += 1;

		release_mutex_lock(&count_max_lock);
		release_mutex_lock(&count_curr_lock);

		set_digital_tube(fd, count_curr_tmp);
		sleep(1);
	}
}

int ch422g(int argc, char** argv) 
{ 
	int fd, ret, i, tmp;
	char buff[32];

	fd = open("/dev/ch422g", O_RDWR); 
	printf("test: fd=%d\n", fd); 
	if(fd == -1) 
	{ 
		printf("test: Error Opening ch422g\n"); 
		return(-1); 
	}

	ret = pthread_mutex_init(&count_max_lock, NULL);
	if (ret != 0)
	{
		printf("test: Error Init count_max_lock\n");
		return(-1);
	}

	ret = pthread_mutex_init(&count_curr_lock, NULL);
	if (ret != 0)
	{
		printf("test: Error Init count_curr_lock\n");
		return(-1);
	}

	pthread_t pth;
	ret = pthread_create(&pth, NULL, countdown, &fd);
	if (ret != 0)
	{
		printf("test: Error Creating pthread\n");
		return(-1);
	}

	printf("Test program is running, enter \"help\" for help.\n");
 
	while (1)
	{
		memset(buff, 0, sizeof(buff));
		fgets(buff, sizeof(buff), stdin);

		if (strncmp(buff, "exit", 4) == 0)
		{
			printf("Exiting\n");
			break;
		}

		else if (strncmp(buff, "help", 4) == 0)
		{
			printf("---------------------------------------\n");
			printf(" info: Show the value of current data.\n");
			printf(" setmax <NUM>: Set maximum count.\n");
			printf(" setcurr <NUM>: Set current count.\n");
			printf(" exit: End the process\n");
			printf("---------------------------------------\n");
		}
		
		else if (strncmp(buff, "info", 4) == 0)
		{
			printf("count_max:%d\ncount_curr:%d\n", \
				get_count_max(), get_count_curr());
		}
		
		else if (strncmp(buff, "setmax", 6) == 0)
		{
			tmp = atoi(buff + 6);
			set_count_max(tmp);
			printf("count_max = %d\n", tmp);
		}

		else if (strncmp(buff, "setcurr", 7) == 0)
		{
			tmp = atoi(buff + 7);
			set_count_curr(tmp);
			printf("count_curr = %d\n", tmp);
		}
		
	}

	pthread_cancel(pth);
	printf("Thread closed\n");

	reset_digital_tube(fd);
	close(fd);

	return 0;
}

void set_digital_tube(int fd, int count)
{
	for (int i = 4; i >= 1; i--)
	{
		databuf[i] = BCD_decode_tab[count % 10];
		count /= 10;
	}

	write(fd, databuf, sizeof(databuf));
}

void reset_digital_tube(int fd)
{
	databuf[1] = 0x0;
	databuf[2] = 0x0;
	databuf[3] = 0x0;
	databuf[4] = 0x0;

	write(fd, databuf, sizeof(databuf));
}

void get_mutex_lock(pthread_mutex_t *lock)
{
	if (pthread_mutex_lock(lock) != 0)
	{
		printf("ERROR: get lock error!\n");
		return;
	}
}

void release_mutex_lock(pthread_mutex_t *lock)
{
	if (pthread_mutex_unlock(lock) != 0)
	{
		printf("ERROR: release lock error!\n");
		return;
	}
}

void set_count_max(int count)
{
	get_mutex_lock(&count_max_lock);
	count_max = count;
	release_mutex_lock(&count_max_lock);	
}

int get_count_max(void)
{
	int count;
	get_mutex_lock(&count_max_lock);
	count = count_max;
	release_mutex_lock(&count_max_lock);
	return count;
}

void set_count_curr(int curr)
{
	get_mutex_lock(&count_curr_lock);
	count_curr = curr;
	release_mutex_lock(&count_curr_lock);	
}

int get_count_curr(void)
{
	int count;
	get_mutex_lock(&count_curr_lock);
	count = count_curr;
	release_mutex_lock(&count_curr_lock);
	return count;
}
