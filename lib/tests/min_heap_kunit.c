// SPDX-License-Identifier: GPL-2.0-only

/*
 * Test cases for the min max heap.
 */

#include <kunit/test.h>
#include <linux/min_heap.h>

DEFINE_MIN_HEAP(int, min_heap_test);

static bool less_than(const void *lhs, const void *rhs, void __always_unused *args)
{
	return *(int *)lhs < *(int *)rhs;
}

static bool greater_than(const void *lhs, const void *rhs, void __always_unused *args)
{
	return *(int *)lhs > *(int *)rhs;
}

static void pop_verify_heap(struct kunit *test,
				bool min_heap,
				struct min_heap_test *heap,
				const struct min_heap_callbacks *funcs)
{
	int *values = heap->data;
	int last;

	last = values[0];
	min_heap_pop_inline(heap, funcs, NULL);
	while (heap->nr > 0) {
		if (min_heap) {
			KUNIT_EXPECT_LE(test, last, values[0]);
		} else {
			KUNIT_EXPECT_GE(test, last, values[0]);
		}
		last = values[0];
		min_heap_pop_inline(heap, funcs, NULL);
	}
}

static const int static_values[] = {
	3, 1, 2, 4, 0x8000000, 0x7FFFFFF, 0,
	-3, -1, -2, -4, 0x8000000, 0x7FFFFFF,
};

typedef void (*min_heap_init_func)(struct kunit *test);

struct min_heap_test_case {
	const char *str;
	bool min_heap;
	min_heap_init_func init;
};

static int min_heap_test_init(struct kunit *test)
{
	const struct min_heap_test_case *params = test->param_value;
	int *values;
	size_t nr = ARRAY_SIZE(static_values);

	values = kunit_kcalloc(test, nr, sizeof(int), GFP_KERNEL);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, values);
	test->priv = values;
	if (params && params->init)
		params->init(test);

	return 0;
}

static void min_heap_test_init_known(struct kunit *test)
{
	int *values = test->priv;
	memcpy(values, static_values, sizeof(static_values));
}

static void min_heap_test_init_random(struct kunit *test)
{
	int *values = test->priv;
	size_t nr = ARRAY_SIZE(static_values);
	int i;

	for (i = 0; i < nr; i++)
		values[i] = get_random_u32();
}

static struct min_heap_test_case min_heapify_all_test_ranges[] = {
	{
		.str = "heapify_all min known values",
		.min_heap = true,
		.init = min_heap_test_init_known,
	},
	{
		.str = "heapify_all max known values",
		.min_heap = false,
		.init = min_heap_test_init_known,
	},
	{
		.str = "heapify_all min random values",
		.min_heap = true,
		.init = min_heap_test_init_random,
	},
	{
		.str = "heapify_all max random values",
		.min_heap = false,
		.init = min_heap_test_init_random,
	},
};

KUNIT_ARRAY_PARAM_DESC(min_heap_heapify_all, min_heapify_all_test_ranges, str);

static void min_heap_test_verify(struct kunit *test)
{
	const struct min_heap_test_case *params = test->param_value;
	int *values = test->priv;
	size_t nr = ARRAY_SIZE(static_values);

	struct min_heap_test heap = {
		.data = values,
		.nr = nr,
		.size =  nr,
	};
	struct min_heap_callbacks funcs = {
		.less = params->min_heap ? less_than : greater_than,
		.swp = NULL,
	};

	min_heapify_all_inline(&heap, &funcs, NULL);
	pop_verify_heap(test, params->min_heap, &heap, &funcs);
}

static struct kunit_case min_heap_test_cases[] = {
	KUNIT_CASE_PARAM(min_heap_test_verify, min_heap_heapify_all_gen_params),
	{},
};

static struct kunit_suite min_heap_test_suite = {
	.name = "min_heap",
	.init = min_heap_test_init,
	.test_cases = min_heap_test_cases,
};

kunit_test_suite(min_heap_test_suite);

MODULE_DESCRIPTION("Test cases for the min max heap");
MODULE_LICENSE("GPL");
