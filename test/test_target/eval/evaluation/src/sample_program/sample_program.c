#include "../../support/support.h"
#include <stdint.h>
#include <stdlib.h>

/* This scale factor will be changed to equalize the runtime of the benchmarks. */
#define SCALE_FACTOR    (REPEAT_FACTOR >> 0)

/* Data and key */
static const uint8_t data[16] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
};

static const uint8_t key[16] = {
    0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8,
    0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
};

static uint8_t result[16];

struct benchmark_ctx {
    uint8_t S[256];
    uint8_t i;
    uint8_t j;
    uint8_t *extra_data;
};

#define SWAP(a, b) do { int _t = a; a = b; b = _t; } while(0)

/* New function to randomly modify a block of memory */
void scramble_memory(uint8_t *buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] ^= (buffer[i] + i) & 0xff;  // Arbitrary scrambling logic
    }
}

/* Set up the key and initialize the context */
void setup_key(struct benchmark_ctx *ctx, const uint8_t *key, int key_len) {
    unsigned i, j, k;

    /* Initialize context */
    for (i = 0; i < 256; i++) {
        ctx->S[i] = i;
    }

    for (i = j = k = 0; i < 256; i++) {
        j += ctx->S[i] + key[k]; j &= 0xff;
        SWAP(ctx->S[i], ctx->S[j]);
        k = (k + 1) % key_len;
    }

    ctx->i = ctx->j = 0;
}

/* Encrypt or decrypt using the context */
void perform_crypt(struct benchmark_ctx *ctx, int length, uint8_t *dst, const uint8_t *src) {
    register uint8_t i, j;
    register int si, sj;

    i = ctx->i; j = ctx->j;
    while (length--) {
        i++; i &= 0xff;
        si = ctx->S[i];
        j += si; j &= 0xff;
        sj = ctx->S[i] = ctx->S[j];
        ctx->S[j] = si;
        *dst++ = *src++ ^ ctx->S[(si + sj) & 0xff];
    }
    ctx->i = i; ctx->j = j;
}

/* Function to modify the state in a different context */
void modify_context(struct benchmark_ctx *ctx, int offset, int length) {
    for (int i = offset; i < offset + length && i < 256; i++) {
        ctx->S[i] ^= (ctx->S[i] + length) & 0xff;  // Arbitrary logic to modify the state
    }
}

/* Perform some extra processing that uses multiple pointers */
void extra_processing(uint8_t *data_ptr, struct benchmark_ctx *ctx_ptr, int length) {
    for (int i = 0; i < length; i++) {
        data_ptr[i] = (data_ptr[i] ^ ctx_ptr->S[i]) & 0xff;  // XOR with context state
    }
}

void initialise_benchmark_sample_program(void) {
    struct benchmark_ctx *ctx = (struct benchmark_ctx*)malloc(sizeof(struct benchmark_ctx));
    ctx->extra_data = (uint8_t*)malloc(16 * sizeof(uint8_t));

    /* Initialize the extra data with some logic */
    for (int i = 0; i < 16; i++) {
        ctx->extra_data[i] = (key[i] + data[i]) & 0xff;
    }

    free(ctx->extra_data);
    free(ctx);
}

int benchmark_sample_program(void) {
    struct benchmark_ctx *ctx = (struct benchmark_ctx*)malloc(sizeof(struct benchmark_ctx));
    ctx->extra_data = (uint8_t*)malloc(16 * sizeof(uint8_t));

    /* Setup key */
    setup_key(ctx, key, 16);

    /* Encrypt */
    perform_crypt(ctx, 16, result, data);

    /* Modify the state in the context */
    modify_context(ctx, 5, 10);

    /* Extra processing using multiple pointers */
    extra_processing(result, ctx, 16);

    /* Decrypt */
    setup_key(ctx, key, 16);
    perform_crypt(ctx, 16, result, result);

    free(ctx->extra_data);
    free(ctx);

    return 0;
}

int verify_benchmark_sample_program(int unused) {
    int expected[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    for (int i = 0; i < 16; i++) {
        if (result[i] != expected[i])
            return 0;
    }
    return 1;
}
