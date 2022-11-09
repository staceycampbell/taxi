#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define T_COUNT 8 // thread count
#define LIMIT 1024

typedef struct taxi_t {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint64_t solution;
} taxi_t;

static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;

static taxi_t *Taxi;
static int TaxiCount;
static int TaxiSize;

static uint32_t Limit = LIMIT;

static int
CmpTaxi(const void *p1, const void *p2)
{
	const taxi_t *t1, *t2;

	t1 = p1;
	t2 = p2;

	return (t1->solution > t2->solution) - (t1->solution < t2->solution);
}

static uint64_t *CubeTable;

static inline uint64_t
Cubed(uint64_t a)
{
	return a * a * a;
}

static void
InitCubed(uint32_t limit)
{
	uint64_t a;

	CubeTable = (uint64_t *)malloc(limit * sizeof(uint64_t));
	for (a = 0; a < limit; ++a)
		CubeTable[a] = Cubed(a);
}

static inline int
Check(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	uint32_t good;
	uint64_t a_cubed, b_cubed, c_cubed, d_cubed, x, y;

	a_cubed = CubeTable[a];
	b_cubed = CubeTable[b];
	c_cubed = CubeTable[c];
	d_cubed = CubeTable[d];

	x = a_cubed + b_cubed;
	y = c_cubed + d_cubed;

	good = x == y;
	if (good)
	{
		pthread_mutex_lock(&Mutex);
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
		pthread_mutex_unlock(&Mutex);
	}

	return good;
}

static void *
LoopThread(void *arg)
{
	uint32_t a, b, c, d, a_start, c_start;
	uint32_t limit;

	a = *(uint32_t *)arg;
	a_start = a + 1;
	c_start = a + 2;
	limit = Limit;
	for (b = a_start; b < limit; ++b)
		for (c = c_start; c < limit; ++c)
			for (d = c + 1; d < limit; ++d)
				Check(a, b, c, d);
	return 0;
}

int
main(int argc, char *argv[])
{
	uint32_t i, a, t, taxicabcount;
	uint32_t check_size;
	uint32_t arg[T_COUNT];
	pthread_t loop_thread[T_COUNT];
	static const uint64_t valid[] = {
		1729, 4104, 13832, 20683, 32832, 39312, 40033, 46683, 64232, 65728, 110656, 110808,
		134379, 149389, 165464, 171288, 195841, 216027, 216125, 262656, 314496, 320264, 327763,
		373464, 402597, 439101, 443889, 513000, 513856, 515375, 525824, 558441, 593047, 684019,
		704977
	};

	taxicabcount = LIMIT;
	switch (argc)
	{
	case 1 :
		taxicabcount = LIMIT;
		break;
	case 2 :
		taxicabcount = strtoul(argv[1], 0, 0);
		break;
	default :
		fprintf(stderr, "usage: %s count\n", argv[0]);
		exit(1);
	}


	Limit = (taxicabcount * 3) / 2 + 50;
	InitCubed(Limit);

	pthread_mutex_init(&Mutex, NULL);

	TaxiCount = 0;
	TaxiSize = 10;
	Taxi = (taxi_t *) malloc(TaxiSize * sizeof(taxi_t));

	a = 1;
	while (a < Limit && TaxiCount < taxicabcount * 2)
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
	i = 0;
	if (sizeof(valid) / sizeof(valid[0]) > taxicabcount)
		check_size = taxicabcount;
	else
		check_size = sizeof(valid) / sizeof(valid[0]);
	while (i < check_size && Taxi[i].solution == valid[i])
		++i;
	if (i < check_size)
	{
		fprintf(stderr, "Sequence bad at position %u (you have %" PRIu64 ", should be %" PRIu64 ")\n", i, Taxi[i].solution, valid[i]);
		exit(1);
	}
	for (i = 0; i < taxicabcount; ++i)
		printf("Taxi(%4d): %4u^3 + %4u^3 == %4u^3 + %4u^3 == %10" PRIu64 "%s\n", i + 1,
			Taxi[i].a, Taxi[i].b, Taxi[i].c, Taxi[i].d, Taxi[i].solution,
			i > 0 && Taxi[i].solution == Taxi[i - 1].solution ? "(!)" : "");

	return 0;
}
