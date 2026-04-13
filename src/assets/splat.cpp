#include "splat.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <happly.h>
#include <span>

namespace Assets {
    std::vector<Splat> g_splats;
    
    void load_splats(const std::string& ply_file) {
        happly::PLYData plyIn{ply_file};

        std::vector<float> x = plyIn.getElement("splat").getProperty<float>("x");
        std::vector<float> y = plyIn.getElement("splat").getProperty<float>("y");
        std::vector<float> z = plyIn.getElement("splat").getProperty<float>("z");
        std::vector<float> r0 = plyIn.getElement("splat").getProperty<float>("r_0");
        std::vector<float> r1 = plyIn.getElement("splat").getProperty<float>("r_1");
        std::vector<float> r2 = plyIn.getElement("splat").getProperty<float>("r_2");
        std::vector<float> r3 = plyIn.getElement("splat").getProperty<float>("r_3");
        std::vector<float> s0 = plyIn.getElement("splat").getProperty<float>("s_0");
        std::vector<float> s1 = plyIn.getElement("splat").getProperty<float>("s_1");
        std::vector<float> s2 = plyIn.getElement("splat").getProperty<float>("s_2");
        std::vector<float> r = plyIn.getElement("splat").getProperty<float>("r");
        std::vector<float> g = plyIn.getElement("splat").getProperty<float>("g");
        std::vector<float> b = plyIn.getElement("splat").getProperty<float>("b");
        std::vector<float> a = plyIn.getElement("splat").getProperty<float>("a");
        
        g_splats.clear();
        g_splats.reserve(x.size());
        for (int i = 0; i < x.size(); ++i) {
            g_splats.push_back(Splat{
                {x[i], y[i], z[i]},
                {r0[i], r1[i], r2[i], r3[i]},
                {s0[i], s1[i], s2[i]},
                {r[i], g[i], b[i], a[i]}
            });
            std::cout << "Loading splat:\n\tPosition: " << g_splats.back().position
            << "\n\tRotation: " << g_splats.back().rotation
            << "\n\tScale: " << g_splats.back().scale
            << "\n\tColor: " << g_splats.back().color << std::endl;
        }
    }
    
    std::span<const Splat> get_splats() {
        return std::span(g_splats);
    }
}




