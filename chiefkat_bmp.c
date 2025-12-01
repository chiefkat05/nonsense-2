#include "chiefkat_bmp.h"

bmp_data read_bmp(const char *path)
{
    FILE *input = fopen(path, "rb");
    verify(input, "failed to open bmp file", __LINE__);
    bmp_data image = {};

    if (fgetc(input) != 'B')
    {
        verify(false, "not bmp file", __LINE__);
    }
    if (fgetc(input) != 'M')
    {
        verify(false, "not bmp file", __LINE__);
    }
    const int bmp_header_length = 14;
    fseek(input, bmp_header_length, SEEK_SET);

    int dib_header_length;
    fread(&dib_header_length, 1, 4, input);
    fread(&image.width, 1, 4, input);
    fread(&image.height, 1, 4, input);

    fseek(input, 14 + dib_header_length, SEEK_SET);

    image.data = (unsigned char *)malloc(image.width * image.height * 4);
    verify(image.data, "failed to allocate buffer memory", __LINE__);
    fread(image.data, 4, image.width * image.height, input);

    return image;
}

void free_image(bmp_data *image)
{
    free(image->data);
}