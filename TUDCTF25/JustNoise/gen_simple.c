#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void ensure_dir(const char *path){
    // best-effort create; ignore if exists
    mkdir(path, 0755);
}

/* tiny LCG to fill the image with random bytes (state seeded by index) */
static uint32_t lcg_step(uint32_t *s){
    *s = (*s * 1664525u + 1013904223u);
    return *s;
}
static uint8_t rnd8(uint32_t *s){ return (uint8_t)(lcg_step(s) >> 24); }

/* deterministic per-index position (no seed) */
static void choose_pos(size_t idx, int w, int h, int *x, int *y){
    /* use 64-bit math to avoid overflow for larger idx */
    uint64_t i = (uint64_t)idx;
    uint64_t xx = (i*73u + i*i*19u + 17u) % (uint64_t)w;
    uint64_t yy = (i*131u + i*i*7u + 23u) % (uint64_t)h;
    *x = (int)xx;
    *y = (int)yy;
}

/* per-index rotation amount 0..25 (no seed) */
static int rot_for_idx(size_t idx){
    uint64_t i = (uint64_t)idx;
    return (int)((i*7u + i*i*3u + 5u) % 26u);
}

/* Caesar rotate only letters */
static unsigned char rot_char(unsigned char c, int r){
    if (c >= 'A' && c <= 'Z') return (unsigned char)('A' + ((c - 'A' + r) % 26));
    if (c >= 'a' && c <= 'z') return (unsigned char)('a' + ((c - 'a' + r) % 26));
    return c; // non-letters unchanged
}

int main(int argc, char **argv){
    if (argc < 3){
        fprintf(stderr, "Usage: %s \"FLAG_TEXT\" out_dir [W H]\n", argv[0]);
        return 1;
    }
    const char *flag = argv[1];
    const char *outdir = argv[2];
    int W = (argc >= 4) ? atoi(argv[3]) : 128;
    int H = (argc >= 5) ? atoi(argv[4]) : 128;
    if (W <= 0 || H <= 0){ fprintf(stderr, "Bad size\n"); return 1; }

    ensure_dir(outdir);

    size_t n = strlen(flag);
    size_t img_bytes = (size_t)W * (size_t)H * 3;
    unsigned char *buf = (unsigned char*)malloc(img_bytes);
    if (!buf){ fprintf(stderr, "OOM\n"); return 1; }

    for (size_t i = 0; i < n; i++){
        /* per-image random fill (state from index only) */
        uint32_t s = (uint32_t)(i + 1);
        for (size_t k = 0; k < img_bytes; k++) buf[k] = rnd8(&s);

        /* where to place the secret byte for image i */
        int x, y; choose_pos(i, W, H, &x, &y);

        /* rotate and embed into RED channel at (x,y) */
        unsigned char c  = (unsigned char)flag[i];
        int r            = rot_for_idx(i);
        unsigned char rc = rot_char(c, r);

        size_t off = ((size_t)y * (size_t)W + (size_t)x) * 3;
        buf[off + 0] = rc;  // R channel holds the rotated ASCII

        /* write PPM (P6) */
        char path[512];
        snprintf(path, sizeof(path), "%s/img_%03zu.ppm", outdir, i);
        FILE *f = fopen(path, "wb");
        if (!f){ fprintf(stderr, "Failed to open %s\n", path); free(buf); return 1; }
        fprintf(f, "P6\n%d %d\n255\n", W, H);
        fwrite(buf, 1, img_bytes, f);
        fclose(f);
        printf("Wrote %s (secret at x=%d,y=%d)\n", path, x, y);
    }

    free(buf);
    return 0;
}