#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// RGB color structure
typedef struct {
    uint8_t r, g, b;
} Color;

// C2PM header structure
typedef struct {
    char magic[4];      // "C2PM"
    uint32_t width;
    uint32_t height;
    uint32_t color_count;
} C2PMHeader;

// Color entry structure
typedef struct {
    Color color;
    uint32_t count;
    uint32_t *indices;
} ColorEntry;

// Compare two colors
int color_equal(Color a, Color b) {
    return (a.r == b.r && a.g == b.g && a.b == b.b);
}

// Find existing color index
int find_color(ColorEntry *table, uint32_t count, Color c) {
    for (uint32_t i = 0; i < count; i++) {
        if (color_equal(table[i].color, c)) return i;
    }
    return -1;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.png> <output.c2pm>\n", argv[0]);
        return 1;
    }

    int width, height, channels;
    unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 3);
    if (!img) {
        printf("Failed to load image\n");
        return 1;
    }

    ColorEntry *colors = NULL;
    uint32_t color_count = 0;
    uint32_t total_pixels = width * height;

    for (uint32_t i = 0; i < total_pixels; i++) {
        Color c = { img[i*3], img[i*3+1], img[i*3+2] };
        int idx = find_color(colors, color_count, c);
        if (idx == -1) {
            colors = realloc(colors, (color_count + 1) * sizeof(ColorEntry));
            colors[color_count].color = c;
            colors[color_count].count = 1;
            colors[color_count].indices = malloc(sizeof(uint32_t));
            colors[color_count].indices[0] = i;
            color_count++;
        } else {
            colors[idx].count++;
            colors[idx].indices = realloc(colors[idx].indices, colors[idx].count * sizeof(uint32_t));
            colors[idx].indices[colors[idx].count - 1] = i;
        }
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        printf("Failed to open output file\n");
        return 1;
    }

    C2PMHeader header = { "C2PM", width, height, color_count };
    fwrite(&header, sizeof(header), 1, out);

    for (uint32_t i = 0; i < color_count; i++) {
        fwrite(&colors[i].color, sizeof(Color), 1, out);
        fwrite(&colors[i].count, sizeof(uint32_t), 1, out);
        fwrite(colors[i].indices, sizeof(uint32_t), colors[i].count, out);
    }

    fclose(out);
    stbi_image_free(img);

    printf("C2PM encoded successfully! Unique colors: %u\n", color_count);
    return 0;
}
       