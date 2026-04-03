#ifndef WATERENGINE_CONFIG_H
#define WATERENGINE_CONFIG_H
#include <stdexcept>

namespace Config {
    class config_error : public std::runtime_error {
    public:
        explicit config_error(const std::string& msg) : std::runtime_error(msg) {}
    };

    enum class ConfigGroup {
        Shaders
    };

    /**
     * Retrieves config value from given config group, section, key and optionally a default value.
     * @param group Config group, synonymous to a config file
     * @param section Section within the config group
     * @param key Key within the section
     * @param def Default value. If unspecified or set to "", an exception is thrown when the value is not found.
     * @return Configured string value, or default value if not found
     */
    std::string get_value(ConfigGroup group, const std::string& section, const std::string& key, const std::string& def = "");
}

#endif //WATERENGINE_CONFIG_H