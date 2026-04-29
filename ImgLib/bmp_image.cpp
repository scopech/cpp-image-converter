#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <vector>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2];
    uint32_t size;
    uint32_t reserved;
    uint32_t offset;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pels_per_meter;
    int32_t y_pels_per_meter;
    int32_t colors_used;
    int32_t colors_important;
}
PACKED_STRUCT_END

//константы
static const int BYTES_PER_PIXEL = 3;
static const int ALIGNMENT = 4;

static int GetBMPStride(int w) {
    return ALIGNMENT * ((w * BYTES_PER_PIXEL + ALIGNMENT - 1) / ALIGNMENT);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if (!out) {
        return false;
    }

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);

    BitmapFileHeader file_header;
    file_header.signature[0] = 'B';
    file_header.signature[1] = 'M';
    file_header.size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + stride * h; //sizeof
    file_header.reserved = 0;
    file_header.offset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

    BitmapInfoHeader info_header;
    info_header.header_size = sizeof(BitmapInfoHeader);
    info_header.width = w;
    info_header.height = h;
    info_header.planes = 1;
    info_header.bit_count = BYTES_PER_PIXEL * 8; // 24 бита на пиксель
    info_header.compression = 0;
    info_header.image_size = stride * h;
    info_header.x_pels_per_meter = 11811;
    info_header.y_pels_per_meter = 11811;
    info_header.colors_used = 0;
    info_header.colors_important = 0x1000000;

    //sizeof
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    std::vector<char> buff(stride, 0);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * BYTES_PER_PIXEL + 0] = static_cast<char>(line[x].b);
            buff[x * BYTES_PER_PIXEL + 1] = static_cast<char>(line[x].g);
            buff[x * BYTES_PER_PIXEL + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), stride);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        return {};
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    //sizeof
    if (!in.read(reinterpret_cast<char*>(&file_header), sizeof(file_header))) {
        return {};
    }
    if (!in.read(reinterpret_cast<char*>(&info_header), sizeof(info_header))) {
        return {};
    }

    if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M') {
        return {};
    }

    const int w = info_header.width;
    const int h = info_header.height;
    const int stride = GetBMPStride(w);

    Image result(w, h, Color::Black());
    std::vector<char> buff(stride);

    
    for (int y = h - 1; y >= 0; --y) {
        if (!in.read(buff.data(), stride)) {
            return {};
        }

        Color* line = result.GetLine(y);
        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<std::byte>(buff[x * BYTES_PER_PIXEL + 0]);
            line[x].g = static_cast<std::byte>(buff[x * BYTES_PER_PIXEL + 1]);
            line[x].r = static_cast<std::byte>(buff[x * BYTES_PER_PIXEL + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
