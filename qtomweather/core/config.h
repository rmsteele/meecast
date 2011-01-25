#ifndef CONFIG_H
#define CONFIG_H
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <QTextStream>
#include "parser.h"
#include "stationlist.h"
////////////////////////////////////////////////////////////////////////////////
namespace Core{
    class Config : public Parser{
            #ifdef LIBXML
            void processNode(const xmlpp::Node* node);
            #endif
            std::string *_pathPrefix;
            std::string *_iconset;
            std::string *_temperature_unit;
            std::string *_font_color;
            StationList *_stations;
        public:
            Config(const std::string& filename, const std::string& schema_filename = prefix + schemaPath + "config.xsd");
            Config();
            Config(const Config& config);
            Config& operator=(const Config& config);
            virtual ~Config();
            std::string& prefix_path(void);
            void iconSet(const std::string& text);
            std::string& iconSet(void);
            void TemperatureUnit(const std::string& text);
            std::string& TemperatureUnit(void);
            void FontColor(const std::string& text);
            std::string& FontColor(void);
            StationList& stationList();
            void saveConfig(const std::string& filename);
    };
} // namespace Core
////////////////////////////////////////////////////////////////////////////////
#endif // CONFIG_H
