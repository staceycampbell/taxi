#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define LIMIT 350		// max is 1625 before 32 bit overflow (log2(LIMIT^3))

static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t MutexPrint = PTHREAD_MUTEX_INITIALIZER;
static int ThreadCount;

static inline unsigned int
Cubed(unsigned int a)
{
	return a * a * a;
}

static int
Check(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	unsigned int good;
	unsigned int a_cubed, b_cubed, c_cubed, d_cubed, x, y;

	a_cubed = Cubed(a);
	b_cubed = Cubed(b);
	c_cubed = Cubed(c);
	d_cubed = Cubed(d);

	x = a_cubed + b_cubed;
	y = c_cubed + d_cubed;

	good = x == y;
	if (good)
	{
		pthread_mutex_lock(&MutexPrint);
		printf("%4u^3 + %4u^3 == %4u^3 + %4u^3 == %10u\n", a, b, c, d, x);
		pthread_mutex_unlock(&MutexPrint);
	}

	return good;
}

static void *
LoopThread(void *arg)
{
	unsigned int a, b, c, d;

	a = *(unsigned int *)arg;
	for (c = a + 1; c < LIMIT; ++c)
		for (d = c; d < LIMIT; ++d)
			for (b = a; b < LIMIT; ++b)
				if (a != c && a != d && b != c && b != d)
					(void)Check(a, b, c, d);
	pthread_mutex_lock(&Mutex);
	--ThreadCount;
	pthread_mutex_unlock(&Mutex);

	return 0;
}

#define T_COUNT 3

int
main(void)
{
	unsigned int a, ready;
	unsigned int arg[LIMIT];
	pthread_t last_loop_thread = 0;

	pthread_mutex_init(&Mutex, NULL);
	pthread_mutex_init(&MutexPrint, NULL);

	ThreadCount = 0;

	a = 1;
	while (a < LIMIT)
	{
		pthread_mutex_lock(&Mutex);
		ready = ThreadCount < T_COUNT;
		pthread_mutex_unlock(&Mutex);
		if (ready)
		{
			pthread_mutex_lock(&Mutex);
			++ThreadCount;
			pthread_mutex_unlock(&Mutex);
			arg[a] = a;
			pthread_create(&last_loop_thread, NULL, LoopThread, &arg[a]);
			++a;
		}
		else
			pthread_join(last_loop_thread, 0);
	}

	return 0;
}
