#include "TLG.h"
#include "png.h"
#include "getopt.h"
#include "wchar_util.h"
#include <stdint.h>
#include "file_stream.h"
#include "fileop.h"
#include "str_util.h"
#include <stdexcept>

typedef struct TlgPic {
    uint32_t width;
    uint32_t height;
    uint32_t colors;
    uint8_t* data; // Pointer to the image data
} TlgPic;

bool tlg_pic_size_callback(void* callbackdata, tjs_uint w, tjs_uint h) {
    TlgPic* pic = static_cast<TlgPic*>(callbackdata);
    pic->width = w;
    pic->height = h;
    pic->colors = 4; // Assuming BGRA format
    pic->data = new uint8_t[w * h * 4]; // Assuming 4 bytes per pixel (BGRA)
    return true; // Continue processing
}

void* tlg_pic_buf_callback(void* callbackdata, tjs_int y) {
    TlgPic* pic = static_cast<TlgPic*>(callbackdata);
    if (y < 0) {
        // If y is -1, return nullptr to indicate that the image has been fully processed
        return nullptr;
    }
    // Return a pointer to the scanline buffer for the specified y coordinate
    return pic->data + (y * pic->width * pic->colors);
}

void destory_tlg_pic(TlgPic& pic) {
    if (pic.data) {
        delete[] pic.data;
        pic.data = nullptr;
    }
    pic.width = 0;
    pic.height = 0;
    pic.colors = 0;
}

void convertBgrToRgb(TlgPic& pic) {
    if (pic.colors != 3 && pic.colors != 4) return;
    for (uint32_t y = 0; y < pic.height; ++y) {
        size_t lineOffset = y * pic.width * pic.colors;
        for (uint32_t x = 0; x < pic.width; ++x) {
            uint8_t* pixel = pic.data + lineOffset + (x * pic.colors);
            std::swap(pixel[0], pixel[2]); // Swap B and R channels
        }
    }
}

void savePng(const TlgPic& pic, const std::string& output) {
    FILE* fp = fileop::fopen(output.c_str(), "wb");
    if (!fp) {
        throw std::runtime_error("Failed to open output file: " + output);
    }
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        throw std::runtime_error("Failed to create PNG write struct");
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Failed to create PNG info struct");
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        throw std::runtime_error("Error during PNG creation");
    }
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, pic.width, pic.height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    for (uint32_t y = 0; y < pic.height; ++y) {
        uint8_t* row = pic.data + (y * pic.width * pic.colors);
        png_write_row(png_ptr, row);
    }

    png_write_end(png_ptr, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

TlgPic loadPng(std::string input) {
    TlgPic pic;
    memset(&pic, 0, sizeof(TlgPic));
    FILE* fp = fileop::fopen(input.c_str(), "rb");
    if (!fp) {
        throw std::runtime_error("Failed to open PNG file: " + input);
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        throw std::runtime_error("Failed to create PNG read struct");
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Failed to create PNG info struct");
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Error during PNG reading");
    }
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    pic.width = png_get_image_width(png_ptr, info_ptr);
    pic.height = png_get_image_height(png_ptr, info_ptr);
    if (png_get_bit_depth(png_ptr, info_ptr) != 8) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Unsupported PNG bit depth. Only 8-bit PNGs are supported.");
    }
    int type = png_get_color_type(png_ptr, info_ptr);
    if (type == PNG_COLOR_TYPE_GRAY) {
        pic.colors = 1; // Grayscale
    } else if (type == PNG_COLOR_TYPE_RGB) {
        pic.colors = 3; // RGB
    } else if (type == PNG_COLOR_TYPE_RGBA) {
        pic.colors = 4; // RGBA
    } else {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        throw std::runtime_error("Unsupported PNG color type. Only grayscale, RGB, and RGBA are supported.");
    }
    pic.data = new uint8_t[pic.width * pic.height * pic.colors];
    for (uint32_t y = 0; y < pic.height; ++y) {
        uint8_t* row = pic.data + (y * pic.width * pic.colors);
        png_read_row(png_ptr, row, nullptr);
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(fp);
    //convertBgrToRgb(pic);
    return pic;
}

void printHelp() {
    printf("Usage: tlg [options] <input> [<output>]\n");
    printf("Tools to processing TLG files.\n");
    printf("Options:\n");
    printf("  -h, --help        Show this help message\n");
    printf("  -v, --version [v] Specifiy tlg version when encoding. Default: 5. Available values: 5, 6\n");
}

int main(int argc, char* argv[]) {
    int wargc;
    char** wargv;
    bool haveWargv = wchar_util::getArgv(wargv, wargc);
    if (haveWargv) {
        argc = wargc;
        argv = wargv;
    }
    struct option options[] = {
        {"help", 0, nullptr, 'h'},
        {"version", 1, nullptr, 'v'},
        nullptr,
    };
    int opt;
    const char* shortopt = "-hv:";
    std::string input;
    std::string output;
    // Default TLG version
    int tlgVersion = 5;
    while ((opt = getopt_long(argc, argv, shortopt, options, nullptr)) != -1) {
        switch (opt) {
        case 'h':
            printHelp();
            if (haveWargv) wchar_util::freeArgv(wargv, wargc);
            return 0;
        case 'v':
            if (optarg) {
                tlgVersion = std::stoi(optarg);
                if (tlgVersion < 5 || tlgVersion > 6) {
                    fprintf(stderr, "Invalid TLG version: %d. Available values: 5, 6.\n", tlgVersion);
                    if (haveWargv) wchar_util::freeArgv(wargv, wargc);
                    return 1;
                }
            }
            break;
        case 1:
            if (input.empty()) {
                input = optarg;
            } else if (output.empty()) {
                output = optarg;
            } else {
                fprintf(stderr, "Too many arguments. Expected: <input> <output>\n");
                if (haveWargv) wchar_util::freeArgv(wargv, wargc);
                return 1;
            }
            break;
        case '?':
        default:
            fprintf(stderr, "Unknown option: %c\n", optopt);
            printHelp();
            if (haveWargv) wchar_util::freeArgv(wargv, wargc);
            return 1;
        }
    }
    if (input.empty()) {
        fprintf(stderr, "Input file is required.\n");
        printHelp();
        if (haveWargv) wchar_util::freeArgv(wargv, wargc);
        return 1;
    }
    std::string inputExt = str_util::tolower(fileop::extname(input));
    bool decoding = (inputExt == "tlg" || inputExt == "tlg5" || inputExt == "tlg6");
    if (haveWargv) wchar_util::freeArgv(wargv, wargc);
    try {
        if (decoding) {
            auto f = FileStream(input, "rb");
            if (!TVPCheckTLG(&f)) {
                fprintf(stderr, "Not a valid TLG file: %s\n", input.c_str());
                return 1;
            }
            TlgPic pic;
            memset(&pic, 0, sizeof(TlgPic));
            auto re = TVPLoadTLG(
                &pic,
                tlg_pic_size_callback,
                tlg_pic_buf_callback,
                nullptr, // No tags needed for decoding
                &f
            );
            if (re == -1) {
                throw std::runtime_error("Failed to load TLG file: " + input);
            }
            convertBgrToRgb(pic);
            savePng(pic, output.empty() ? fileop::filename(input) + ".png" : output);
            destory_tlg_pic(pic);
        } else {
            TlgPic pic = loadPng(input);
            if (pic.width == 0 || pic.height == 0 || pic.data == nullptr) {
                throw std::runtime_error("Invalid PNG file: " + input);
            }
            auto f = FileStream(output.empty() ? fileop::filename(input) + ".tlg" : output, "wb");
            auto re = TVPSaveTLG(
                &f,
                tlgVersion == 5 ? 0 : 1, // TLG version
                pic.width,
                pic.height,
                pic.colors,
                &pic,
                tlg_pic_buf_callback,
                nullptr
            );
            if (re == -1) {
                throw std::runtime_error("Failed to save TLG file: " + output);
            }
            destory_tlg_pic(pic);
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    return 0;
}
