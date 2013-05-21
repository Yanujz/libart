#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>

#include <check.h>

#include "art.h"

START_TEST(test_art_init_and_destroy)
{
    art_tree t;
    int res = init_art_tree(&t);
    fail_unless(res == 0);

    fail_unless(art_size(&t) == 0);

    res = destroy_art_tree(&t);
    fail_unless(res == 0);
}
END_TEST

START_TEST(test_art_insert)
{
    art_tree t;
    int res = init_art_tree(&t);
    fail_unless(res == 0);

    int len;
    char buf[512];
    FILE *f = fopen("tests/words.txt", "r");

    uintptr_t line = 1;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';
        fail_unless(NULL == art_insert(&t, buf, len, (void*)line));
        fail_unless(art_size(&t) == line);
        line++;
    }

    res = destroy_art_tree(&t);
    fail_unless(res == 0);
}
END_TEST

START_TEST(test_art_insert_search)
{
    art_tree t;
    int res = init_art_tree(&t);
    fail_unless(res == 0);

    int len;
    char buf[512];
    FILE *f = fopen("tests/words.txt", "r");

    uintptr_t line = 1;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';
        fail_unless(NULL ==
            art_insert(&t, buf, len, (void*)line));
        line++;
    }

    // Seek back to the start
    fseek(f, 0, SEEK_SET);

    // Search for each line
    line = 1;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';

        uintptr_t val = (uintptr_t)art_search(&t, buf, len);
	fail_unless(line == val, "Line: %d Val: %" PRIuPTR " Str: %s\n", line,
	    val, buf);
        line++;
    }

    // Check the minimum
    art_leaf *l = art_minimum(&t);
    fail_unless(l && strcmp((char*)l->key, "A") == 0);

    // Check the maximum
    l = art_maximum(&t);
    fail_unless(l && strcmp((char*)l->key, "zythum") == 0);

    res = destroy_art_tree(&t);
    fail_unless(res == 0);
}
END_TEST

START_TEST(test_art_insert_delete)
{
    art_tree t;
    int res = init_art_tree(&t);
    fail_unless(res == 0);

    int len;
    char buf[512];
    FILE *f = fopen("tests/words.txt", "r");

    uintptr_t line = 1, nlines;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';
        fail_unless(NULL ==
            art_insert(&t, buf, len, (void*)line));
        line++;
    }

    nlines = line - 1;

    // Seek back to the start
    fseek(f, 0, SEEK_SET);

    // Search for each line
    line = 1;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';

        // Search first, ensure all entries still
        // visible
        uintptr_t val = (uintptr_t)art_search(&t, buf, len);
	fail_unless(line == val, "Line: %d Val: %" PRIuPTR " Str: %s\n", line,
	    val, buf);

        // Delete, should get lineno back
        val = (uintptr_t)art_delete(&t, buf, len);
	fail_unless(line == val, "Line: %d Val: %" PRIuPTR " Str: %s\n", line,
	    val, buf);

        // Check the size
        fail_unless(art_size(&t) == nlines - line);
        line++;
    }

    // Check the minimum and maximum
    fail_unless(!art_minimum(&t));
    fail_unless(!art_maximum(&t));

    res = destroy_art_tree(&t);
    fail_unless(res == 0);
}
END_TEST

int iter_cb(void *data, const char* key, uint32_t key_len, void *val) {
    uint64_t *out = data;
    uintptr_t line = (uintptr_t)val;
    uint64_t mask = (line * (key[0] + key_len));
    out[0]++;
    out[1] ^= mask;
    return 0;
}

START_TEST(test_art_insert_iter)
{
    art_tree t;
    int res = init_art_tree(&t);
    fail_unless(res == 0);

    int len;
    char buf[512];
    FILE *f = fopen("tests/words.txt", "r");

    uint64_t xor_mask = 0;
    uintptr_t line = 1, nlines;
    while (fgets(buf, sizeof buf, f)) {
        len = strlen(buf);
        buf[len-1] = '\0';
        fail_unless(NULL ==
            art_insert(&t, buf, len, (void*)line));

        xor_mask ^= (line * (buf[0] + len));
        line++;
    }
    nlines = line - 1;

    uint64_t out[] = {0, 0};
    fail_unless(art_iter(&t, iter_cb, &out) == 0);

    fail_unless(out[0] == nlines);
    fail_unless(out[1] == xor_mask);

    res = destroy_art_tree(&t);
    fail_unless(res == 0);
}
END_TEST

