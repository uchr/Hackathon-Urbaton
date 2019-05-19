#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

const double city_min_lat = 54.870285;
const double city_min_lon = 82.553329;
const double city_max_lat = 55.230589;
const double city_max_lon = 83.299713;

std::vector<std::string> LoadNode(std::ifstream& stream)
{
    std::vector<std::string> result;

    std::string line;
    do
    {
        std::getline(stream, line);
        result.push_back(line);
    }
    while (line.find("</node>") == std::string::npos);

    return result;
}

void cut_nsk()
{

    std::ifstream file_stream("../siberian-fed-district-latest.osm");
    std::ofstream out_file_stream("../nsk_graph.osm");

    if (!file_stream.is_open())
    {
        std::cerr << "Can't open file" << std::endl;
        exit(-1);
    }

    if (!out_file_stream.is_open())
    {
        std::cerr << "Can't open file" << std::endl;
        exit(-1);
    }

    out_file_stream << "<osm>" << std::endl;

    std::string line;

    while (std::getline(file_stream, line))
    {
        if (line.find("<node") != std::string::npos)
        {
//            std::cout << line << std::endl;
            const unsigned long start_lat = line.find("lat") + 5;
            const unsigned long finish_lat = line.find('\"', start_lat);
            const unsigned long start_lon = line.find("lon") + 5;
            const unsigned long finish_lon = line.find('\"', start_lon);

            const double lat = std::stod(line.substr(start_lat, finish_lat - start_lat));
            const double lon = std::stod(line.substr(start_lon, finish_lon - start_lon));

//            std::cout << std::stod(line.substr(start_lat, finish_lat - start_lat)) << std::endl;
//            std::cout << std::stod(line.substr(start_lon, finish_lon - start_lon)) << std::endl;

            if (lat > city_min_lat && lat < city_max_lat && lon > city_min_lon && lon < city_max_lon)
            {
                out_file_stream << line << std::endl;
                if (*(line.end() - 2) != '/')
                {
                    const std::vector<std::string> strings = LoadNode(file_stream);
                    for (const auto& string : strings)
                    {
                        out_file_stream << string << std::endl;
                    }
                }
            }

        }
    }

    out_file_stream << "</osm>" << std::endl;
}

int main()
{
    std::ifstream city_graph("./nsk_graph.osm");
    std::ifstream district_stream("../siberian-fed-district-latest.osm");


}