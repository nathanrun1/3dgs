#include "texture2ds.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>
#include <vector>

namespace Assets {
    static const std::vector WHITE_DATA(1, static_cast<std::byte>(0xFF));
    static const std::vector BLACK_DATA(1, static_cast<std::byte>(0x00));
    
    struct Texture2DRange {
        size_t offset;
        size_t count;
    };

    std::vector<std::byte> g_texture2d_data;
    std::vector<Texture2DRange> g_texture2d_ranges;
    
    /**
     * Saves and creates Texture2D instance with given data and dimensions. Scales data to meet width/height dimensions
     * by looping (1-dimensionally)
     */
    Texture2D _save_texture2d(const std::vector<std::byte>& tex_data, const int width, const int height, const int n_channels) {
        const Texture2D new_texture2d(
            g_texture2d_ranges.size(),
            width,
            height,
            n_channels
        );
        
        const size_t data_offset = g_texture2d_data.size();
        
        g_texture2d_data.reserve(g_texture2d_data.size() + new_texture2d.size() + 1);
        if (tex_data.size() == new_texture2d.size())
            g_texture2d_data.insert(g_texture2d_data.end(), tex_data.begin(), tex_data.end());
        else {
            // Raw data and texture size differ, loop through raw data until full texture size is uploaded.
            auto data_it = tex_data.begin();
            for (int i = 0; i < new_texture2d.size(); ++i) {
                if (data_it == tex_data.end())
                    data_it = tex_data.begin();
                g_texture2d_data.push_back(*data_it);
                ++data_it;
            }
        }
        
        const Texture2DRange new_texture_range{data_offset, data_offset + new_texture2d.size()};
        g_texture2d_ranges.push_back(new_texture_range);

        return new_texture2d;
    }

    Texture2D create_texture2d(const std::string& texture_path) {
        int width, height, n_channels;
        stbi_set_flip_vertically_on_load(1);
        auto raw = reinterpret_cast<std::byte*>(stbi_load(texture_path.c_str(), &width, &height, &n_channels, 0));
        if (!raw) {
            stbi_image_free(raw);
            throw texture_error("Failed to load texture (data is invalid): " + texture_path);
        }
        
        std::vector tex_data(raw, raw + width * height * n_channels);

        return _save_texture2d(tex_data, width, height, n_channels);
    }
    
    Texture2D white_texture2d(const int width, const int height, const int n_channels) {
        static const Texture2D white_tex = [&] {
            return _save_texture2d(WHITE_DATA, width, height, n_channels);
        }();
        return white_tex;
    }
    
    Texture2D black_texture2d(const int width, const int height, const int n_channels) {
        static const Texture2D black_tex = [&] {
            return _save_texture2d(BLACK_DATA, width, height, n_channels);
        }();
        return black_tex;
    }

    std::span<const std::byte> get_texture_data(const Texture2D &texture_2d) {
        const auto&[offset, count] = g_texture2d_ranges.at(texture_2d.id);
        return std::span(
          g_texture2d_data.begin() + offset,
          g_texture2d_data.begin() + offset + count
        );
    }
}

// "Nightmare difficulty bug"
