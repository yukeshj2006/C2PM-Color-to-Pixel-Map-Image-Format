#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// RGB structure
typedef struct {
    uint8_t r, g, b;
} Color;

// Header structure
typedef struct {
    char magic[4];      // "C2PM"
    uint32_t width;
    uint32_t height;
    uint32_t color_count;
} C2PMHeader;

// Color entry
typedef struct {
    Color color;
    uint32_t count;
    uint32_t *indices;
} ColorEntry;

// Load C2PM
ColorEntry* load_c2pm(const char *filename, C2PMHeader *header) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fread(header, sizeof(C2PMHeader), 1, f);
    if (strncmp(header->magic, "C2PM", 4) != 0) {
        printf("Invalid C2PM file!\n");
        fclose(f);
        return NULL;
    }

    ColorEntry *entries = malloc(sizeof(ColorEntry) * header->color_count);

    for (uint32_t i = 0; i < header->color_count; i++) {
        fread(&entries[i].color, sizeof(Color), 1, f);
        fread(&entries[i].count, sizeof(uint32_t), 1, f);

        entries[i].indices = malloc(sizeof(uint32_t) * entries[i].count);
        fread(entries[i].indices, sizeof(uint32_t), entries[i].count, f);
    }

    fclose(f);
    return entries;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.c2pm> <output.png>\n", argv[0]);
        return 1;
    }

    C2PMHeader header;
    ColorEntry *entries = load_c2pm(argv[1], &header);
    if (!entries) return 1;

    uint32_t total_pixels = header.width * header.height;
    uint8_t *image = calloc(total_pixels * 3, 1);

    // Reconstruct image
    for (uint32_t i = 0; i < header.color_count; i++) {
        Color c = entries[i].color;
        for (uint32_t j = 0; j < entries[i].count; j++) {
            uint32_t idx = entries[i].indices[j] * 3;
            image[idx]     = c.r;
            image[idx + 1] = c.g;
            image[idx + 2] = c.b;
        }
    }

    // Save as PNG
    if (!stbi_write_png(argv[2], header.width, header.height, 3, image, header.width * 3)) {
        printf("Failed to write PNG\n");
    } else {
        printf("Decoded and saved successfully: %s\n", argv[2]);
    }

    // Cleanup
    for (uint32_t i = 0; i < header.color_count; i++)
        free(entries[i].indices);

    free(entries);
    free(image);

    return 0;
}
