#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define FATAL(msg, ...) do { fprintf(stderr, "FATAL: " msg "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); } while(0)

size_t read_u64_file(const char *filename, u_int64_t **data) {
    FILE *f = fopen(filename, "rb");
    if (!f) FATAL("Cannot open file: %s", filename);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0 || size % sizeof(u_int64_t) != 0) {
        FATAL("Invalid file size for u_int64: %s (size: %ld)", filename, size);
    }

    size_t count = size / sizeof(u_int64_t);
    *data = malloc(count * sizeof(u_int64_t));
    if (!*data) FATAL("Memory allocation failed for %s", filename);

    size_t read = fread(*data, sizeof(u_int64_t), count, f);
    fclose(f);

    if (read != count) {
        FATAL("read failed u_int64: %s, expected %zu, got %zu", filename, count, read);
    }

    printf("Read %zu u_int64 from %s\n", count, filename);
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        FATAL("Usage: %s <basename_without_extension>", argv[0]);
    }

    char base[256];
    snprintf(base, sizeof(base), "%s", argv[1]);

    char path[512];
    
    u_int64_t *val = NULL;
    u_int64_t *pts = NULL;
    size_t n_val, n_pts;

    snprintf(path, sizeof(path), "%s.data", base);
    n_val = read_u64_file(path, &val);

    snprintf(path, sizeof(path), "%s_pts.data", base);
    n_pts = read_u64_file(path, &pts);

    if (n_pts != 4) {
        FATAL("Expected 4 pts entries, got %zu", n_pts);
    }

    snprintf(path, sizeof(path), "%s_output.csv", base);
    FILE *csv = fopen(path, "w");
    if (!csv) FATAL("Failed to open CSV for writing: %s", path);

    fprintf(csv, "t1,%llu\nt2,%llu\nt3,%llu\nt4,%llu\n",
            (unsigned long long)pts[0], (unsigned long long)pts[1],
            (unsigned long long)pts[2], (unsigned long long)pts[3]);

    fprintf(csv, "timestamp_ms,pkg_energy,pp0_energy,pp1_energy,drm_energy\n");

    for (size_t i = 0; i < n_val; i += 5) {
        fprintf(csv, "%lu,%lu,%lu,%lu,%lu\n",
                val[i], val[i+1], val[i+2], val[i+3], val[i+4]);
    }

    fclose(csv);
    printf("Wrote CSV: %s\n", path);

    free(val); free(pts);
    return 0;
}
