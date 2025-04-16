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
    
    u_int64_t *pkg = NULL, *pp0 = NULL, *pp1 = NULL, *drm = NULL;
    u_int64_t *tim = NULL, *pts = NULL;
    size_t n_pkg, n_pp0, n_pp1, n_drm, n_tim, n_pts;

    snprintf(path, sizeof(path), "%spkg.data", base);
    n_pkg = read_u64_file(path, &pkg);

    snprintf(path, sizeof(path), "%spp0.data", base);
    n_pp0 = read_u64_file(path, &pp0);

    snprintf(path, sizeof(path), "%spp1.data", base);
    n_pp1 = read_u64_file(path, &pp1);

    snprintf(path, sizeof(path), "%sdrm.data", base);
    n_drm = read_u64_file(path, &drm);

    snprintf(path, sizeof(path), "%stim.data", base);
    n_tim = read_u64_file(path, &tim);

    snprintf(path, sizeof(path), "%spts.data", base);
    n_pts = read_u64_file(path, &pts);

    if (n_pts != 4) {
        FATAL("Expected 4 pts entries, got %zu", n_pts);
    }

    if (n_pkg != n_pp0 || n_pkg != n_pp1 || n_pkg != n_drm || n_pkg != n_tim) {
        FATAL("Mismatched lengths: pkg=%zu, pp0=%zu, pp1=%zu, drm=%zu, tim=%zu",
              n_pkg, n_pp0, n_pp1, n_drm, n_tim);
    }

    snprintf(path, sizeof(path), "%soutput.csv", base);
    FILE *csv = fopen(path, "w");
    if (!csv) FATAL("Failed to open CSV for writing: %s", path);

    fprintf(csv, "t1,%llu\nt2,%llu\nt3,%llu\nt4,%llun",
            (unsigned long long)pts[0], (unsigned long long)pts[1],
            (unsigned long long)pts[2], (unsigned long long)pts[3]);

    fprintf(csv, "index,timestamp_ms,pkg_energy,pp0_energy,pp1_energy,drm_energy\n");

    for (size_t i = 0; i < n_tim; ++i) {
        fprintf(csv, "%zu,%llu,%lu,%lu,%lu,%lu\n",
                i, (unsigned long long)tim[i],
                pkg[i], pp0[i], pp1[i], drm[i]);
    }

    fclose(csv);
    printf("Wrote CSV: %s\n", path);

    free(pkg); free(pp0); free(pp1); free(drm);
    free(tim); free(pts);
    return 0;
}
