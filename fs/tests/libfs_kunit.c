// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/fs.h>
#include <kunit/test.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/uaccess.h>

struct test_case {
	const char *str;
	const char *buf_content;
	size_t count;
	loff_t pos;
	ssize_t ret;
	const char *out;
};

static struct test_case simple_read_cases[] = {
	{
		.str = "normal_read",
		.buf_content = "Hello World",
		.count = 5,
		.pos = 0,
		.ret = 5,
		.out = "Hello",
	},
};

KUNIT_ARRAY_PARAM_DESC(simple_read, simple_read_cases, str);

static void test_simple_read_from_buffer(struct kunit *test)
{
	const struct test_case *params = test->param_value;
	size_t buf_len = strlen(params->buf_content);
	char __user *kbuf_dest;
	unsigned long user_addr;
	loff_t pos = params->pos;
	ssize_t ret;

	char *check_buf;

	user_addr = kunit_vm_mmap(test, NULL, 0, buf_len,
			    PROT_READ | PROT_WRITE | PROT_EXEC,
			    MAP_ANONYMOUS | MAP_PRIVATE, 0);
	kbuf_dest = (char __user *)user_addr;

	ret = simple_read_from_buffer(kbuf_dest,
				      params->count,
				      &pos,
				      params->buf_content,
				      buf_len);
	KUNIT_ASSERT_EQ(test, ret, params->ret);
	KUNIT_ASSERT_EQ(test, pos, params->pos + ret);

	if (ret > 0) {
		check_buf = kunit_kzalloc(test, ret, GFP_KERNEL);
		KUNIT_ASSERT_NOT_ERR_OR_NULL(test, check_buf);

		if (copy_from_user(check_buf, kbuf_dest, ret)) {
            KUNIT_FAIL(test, "Failed to copy from user");
        }

		KUNIT_EXPECT_MEMEQ(test, check_buf, params->out, ret);
	}
}

static struct kunit_case libfs_test_cases[] = {
	KUNIT_CASE_PARAM(test_simple_read_from_buffer, simple_read_gen_params),
	{},
};

static struct kunit_suite libfs_test_suite = {
	.name = "libfs",
	.test_cases = libfs_test_cases,
};

kunit_test_suite(libfs_test_suite);

MODULE_AUTHOR("Ryota Sakamoto <sakamo.ryota@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("libfs testing module");
