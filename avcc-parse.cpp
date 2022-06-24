#include <sstream>
#include <iostream>
#include <stdio.h>
#include <memory>
#include <string.h>

using namespace std;

/* AV_RBxx AV_WBxx is come from FFMPEG */
#define AV_RB8(x)     (((const uint8_t*)(x))[0])
#define AV_WB8(p, d)  do { ((uint8_t*)(p))[0] = (d); } while(0)

#define AV_RL8(x)     AV_RB8(x)
#define AV_WL8(p, d)  AV_WB8(p, d)

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif
#ifndef AV_WB16
#   define AV_WB16(p, val) do {                 \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif

#ifndef AV_RL16
#   define AV_RL16(x)                           \
    ((((const uint8_t*)(x))[1] << 8) |          \
      ((const uint8_t*)(x))[0])
#endif
#ifndef AV_WL16
#   define AV_WL16(p, val) do {                 \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

#ifndef AV_RB24
#   define AV_RB24(x)                           \
    ((((const uint8_t*)(x))[0] << 16) |         \
     (((const uint8_t*)(x))[1] <<  8) |         \
      ((const uint8_t*)(x))[2])
#endif
#ifndef AV_WB24
#   define AV_WB24(p, d) do {                   \
        ((uint8_t*)(p))[2] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[0] = (d)>>16;           \
    } while(0)
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])
#endif
#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef AV_RL32
#   define AV_RL32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])
#endif
#ifndef AV_WL32
#   define AV_WL32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
    } while(0)
#endif

#define AV_INPUT_BUFFER_PADDING_SIZE 64

/**
 * @brief convert avcc buffer to annexb buffer
 *
 * @param[in] in_avcc, avcc buffer
 * @param[in] size, avcc buffer size
 * @param[out] sps, sps buffer
 * @param[out] sps_size sps buffer size
 * @param[out] pps pps buffer
 * @param[out] pps_size pps buffer size
 * @return int
 */
int avcc_to_annexb(const uint8_t *in_avcc, int *size,
uint8_t **sps, int *sps_size, uint8_t **pps, int *pps_size) {
    uint8_t *out;
    int out_size;
    int read_sps_size = 0, read_pps_size = 0;

    if (*size >= 4 && (AV_RB32(in_avcc) == 0x00000001
         || AV_RB24(in_avcc) == 0x000001)) {
        return 0;
    }

    if (*size < 11 || in_avcc[0] != 1)
        return -1;

    read_sps_size = AV_RB16(&in_avcc[6]);
    if (11 + read_sps_size > *size) {
        return -1;
    }
    read_pps_size = AV_RB16(&in_avcc[9 + read_sps_size]);
    if (11 + read_sps_size + read_pps_size > *size) {
        return -1;
    }
    out_size = 4 + read_sps_size;
    out = (uint8_t *)malloc(out_size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!out) {
        return -1;
    }
    AV_WB32(&out[0], 0x00000001);
    memcpy(out + 4, &in_avcc[8], read_sps_size);
    *sps = out;
    *sps_size = read_sps_size + 4;

    out_size = 4 + read_pps_size;
    out = (uint8_t *)malloc(out_size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!out) {
        return -1;
    }
    AV_WB32(&out[0], 0x00000001);
    memcpy(out + 4, &in_avcc[11 + read_sps_size], read_pps_size);
    *pps = out;
    *pps_size = read_pps_size + 4;
    return 0;
}

/**
 * @brief read a binary file to buffer
 *
 * @param path input file path
 * @param buffer output buffer
 * @param size output buffer size
 */
void read_file(const char*path, uint8_t **buffer, int *size) {
    FILE* fp;
    fp = fopen(path, "rb");

    // seek to end of file
    fseek(fp, 0, SEEK_END);

    // get current file size
    int out_size = ftell(fp);

    // seek back to beginning of file
    fseek(fp, 0, SEEK_SET);

    uint8_t *out;
    out = (uint8_t *)malloc(out_size);
    if (fp) {
        fread(out, out_size, 1, fp);
        fprintf(stderr, "Readed from file %s.\n", path);
    }
    *buffer = out;
    *size = out_size;

    fclose(fp);
}

/**
 * @brief write buffer to binary file
 *
 * @param path output file path
 * @param buffer input buffer
 * @param size input buffer size
 */
void dump_buffer(const char* path, uint8_t* buffer, int size) {
    FILE* fp;

    fp = fopen(path, "wb");
    if (fp) {
        fwrite(buffer, 1, size, fp);
        fprintf(stderr, "dump buffer done.\n");
    }

   fclose(fp);
}


int main(int argc, char **argv) {
    int size, sps_size=0, pps_size=0;
    uint8_t *buffer;
    uint8_t *sps, *pps;

    read_file("./csd.bin", &buffer, &size);

    avcc_to_annexb(buffer, &size, &sps, &sps_size, &pps, &pps_size);

    cout << "annexb sps size " << sps_size << endl;
    cout << "annexb pps size " << pps_size << endl;

    // write sps pps to binary file
    dump_buffer("./sps.bin", sps, sps_size);
    dump_buffer("./pps.bin", pps, pps_size);

    return 0;
}
