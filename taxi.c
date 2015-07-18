#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define T_COUNT 8 // thread count
#define LIMIT 1600

typedef struct taxi_t {
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
	unsigned long long solution;
} taxi_t;

static pthread_mutex_t MutexPrint = PTHREAD_MUTEX_INITIALIZER;

static taxi_t *Taxi;
static int TaxiCount;
static int TaxiSize;

static int
CmpTaxi(const void *p1, const void *p2)
{
	const taxi_t *t1, *t2;

	t1 = p1;
	t2 = p2;

	return (t1->solution > t2->solution) - (t1->solution < t2->solution);
}

static inline unsigned long long
Cubed(unsigned long long a)
{
	return a * a * a;
}

static int
Check(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	unsigned int good;
	unsigned long long a_cubed, b_cubed, c_cubed, d_cubed, x, y;

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
		Taxi[TaxiCount].a = a;
		Taxi[TaxiCount].b = b;
		Taxi[TaxiCount].c = c;
		Taxi[TaxiCount].d = d;
		Taxi[TaxiCount].solution = x;
		++TaxiCount;
		if (TaxiCount == TaxiSize)
		{
			TaxiSize += 100;
			Taxi = (taxi_t *)realloc(Taxi, TaxiSize * sizeof(taxi_t));
		}
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
	return 0;
}

int
main(void)
{
	unsigned int i;
	unsigned int a, t;
	unsigned int arg[T_COUNT];
	pthread_t loop_thread[T_COUNT];

	pthread_mutex_init(&MutexPrint, NULL);

	TaxiCount = 0;
	TaxiSize = 10;
	Taxi = (taxi_t *)malloc(TaxiSize * sizeof(taxi_t));

	a = 1;
	while (a < LIMIT)
	{
		for (t = 0; t < T_COUNT; ++t)
		{
			fprintf(stderr, ".");
			arg[t] = a;
			pthread_create(&loop_thread[t], NULL, LoopThread, &arg[t]);
			++a;
		}
		for (t = 0; t < T_COUNT; ++t)
			pthread_join(loop_thread[t], 0);
	}

	fprintf(stderr, "\n");

	qsort(Taxi, TaxiCount, sizeof(taxi_t), CmpTaxi);
	for (i = 0; i < TaxiCount; ++i)
		printf("Taxi(%4d): %4u^3 + %4u^3 == %4u^3 + %4u^3 == %10llu%s\n", i + 1, Taxi[i].a, Taxi[i].b, Taxi[i].c, Taxi[i].d, Taxi[i].solution,
		    i > 0 && Taxi[i].solution == Taxi[i - 1].solution ? "(!)" : "");
		

	return 0;
}
