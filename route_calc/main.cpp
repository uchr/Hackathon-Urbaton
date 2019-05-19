#include <jsoncpp/json/json.h>

#include <boost/lexical_cast.hpp>

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>

const std::unordered_map<std::string, int> type_to_weight =
        {
            {"motorway", 2},
            {"trunk", 2},
            {"primary", 5},
            {"secondary", 4},
            {"tertiary", 3},
//            {"unclassified", 1},
//            {"residential", 1},
        };

struct Point
{
    double lat = 0.0;
    double lon = 0.0;
};

struct Way
{
    std::string type_;
    std::vector<Point> points_;

};

struct Line
{
    Point start_;
    Point finish_;
    int weight_ = 0;
};

struct PointUtils
{
    static Point Add(const Point& start, const Point& shift)
    {
        Point point;
        point.lat = start.lat + shift.lat;
        point.lon = start.lon + shift.lon;
        return point;
    }

    static Point Substruct(const Point& start, const Point& finish)
    {
        Point point;
        point.lat = start.lat - finish.lat;
        point.lon = start.lon - finish.lon;
        return point;
    }

    static Point Mult(const Point& start, const int n, const int m)
    {
        Point point;
        point.lat = start.lat * n;
        point.lon = start.lon * m;
        return point;
    }

    static double GetDistance(const Point& one, const Point& two)
    {
        double x2 = two.lat;
        double x1 = one.lat;
        double y2 = two.lon;
        double y1 = one.lon;
        return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) );
    }

    static double GetMinLengthFromLineToPoint(const Line& line, const Point& point)
    {
        const double x0 = point.lat;
        const double y0 = point.lon;
        const double x1 = line.start_.lat;
        const double y1 = line.start_.lon;
        const double x2 = line.finish_.lat;
        const double y2 = line.finish_.lon;

        const double diff_y = y2 - y1;
        const double diff_x = x2 - x1;
        const double divide = std::sqrt(diff_y * diff_y + diff_x * diff_x);

        const double mnogo = std::abs(diff_y * x0 - diff_x * y0 + x2 * y1 - y2 * x1);

        const double min_lenght = mnogo / divide;

        double min = std::min(min_lenght, GetDistance(line.start_, point));
        min = std::min(min, GetDistance(line.finish_, point));

        return min;
    }
};

using Ways = std::vector<Way>;

class WaysReader
{
public:
    static Ways Read(std::istream& stream)
    {
        Json::Reader json_reader;
        Json::Value values;
        const bool is_correct = json_reader.parse(stream, values);

        if (!is_correct)
        {
            throw std::runtime_error("Can't read way file");
        }

        Ways ways;


        for (const auto& val : values)
        {
            auto way = Way();
            way.type_ = val["type"].asString();
            for (const auto& point : val["points"])
            {
                Point raw_point;
                raw_point.lat = boost::lexical_cast<double>(point[0].asString());
                raw_point.lon = boost::lexical_cast<double>(point[1].asString());
                way.points_.push_back(raw_point);
            }
            ways.push_back(way);
        }

        return ways;
    }
};

using Lines = std::vector<Line>;

struct CreateTileOptions
{
    Point start_point;
    Point finish_point;
    int n = 0;
    int m = 0;
};

struct Tile
{
    std::array<double, 256*256> weights{};
    int x_index = 0;
    int y_index = 0;
    Point begin_point;
    Point end_point;
    Lines lines;

    bool IsPointInTile(const Point& point) const
    {
        return point.lat > begin_point.lat && point.lat < end_point.lat && point.lon > begin_point.lon && point.lon < end_point.lon;
    }
    
    Point GetPointCenterPixel(const size_t point_number) const
    {
        const Point distance_point = PointUtils::Substruct(end_point, begin_point);

        Point step_point;
        step_point.lat = distance_point.lat / 512;
        step_point.lon = distance_point.lon / 512;

        const Point first_point = PointUtils::Add(begin_point, step_point);

        step_point.lat = distance_point.lat / 256;
        step_point.lon = distance_point.lon / 256;

        const Point full_shift_point = PointUtils::Mult(step_point, point_number / 256, point_number % 256);

        return PointUtils::Add(first_point, full_shift_point);
    }
    
    
};

void CreateTile(const CreateTileOptions& options, const size_t x, const size_t y, Tile &tile)
{

    const Point shift_point = PointUtils::Substruct(options.finish_point, options.start_point);

    Point step_point;
    step_point.lat = shift_point.lat / options.n;
    step_point.lon = shift_point.lon / options.m;

    tile.begin_point = PointUtils::Add(options.start_point, PointUtils::Mult(step_point, x, y));
    tile.end_point = PointUtils::Add(options.start_point, PointUtils::Mult(step_point, x + 1, y + 1));
    tile.x_index = x;
    tile.y_index = y;

    std::cerr << tile.begin_point.lat << " " << tile.begin_point.lon << " ";
    std::cerr << tile.end_point.lat << " " << tile.end_point.lon << std::endl;
}

void DistributeLine(const CreateTileOptions& options, const size_t x, const size_t y, const Ways &ways, std::vector<Tile>& tiles)
{
    auto& current_tile = tiles[y * options.n + x];

    for (const auto& way : ways)
    {
        const auto iter = type_to_weight.find(way.type_);
        if (iter == type_to_weight.end())
        {
            continue;
        }

        for (unsigned j = 0; j < way.points_.size() - 1; ++j)
        {

            const auto& point = way.points_[j];
            if (current_tile.IsPointInTile(point))
            {
                Line line;
                line.start_ = point;
                line.finish_ = way.points_[j + 1];
                line.weight_ = iter->second;
                current_tile.lines.push_back(line);
            }
        }
    }
}


std::pair<double, double> GetWeigth(const Point& center_pixel, const Lines& lines)
{
    double one = 0.0, two = 0.0;
    
    for (const auto& line : lines)
    {
        const double d = PointUtils::GetMinLengthFromLineToPoint(line, center_pixel);
        if (d <= 1e-6)
        {
            return {line.weight_, 1.0};
        }
        one += line.weight_ / d;
//        double okto_norm = 1 / std::pow(d, 8);
//        one += okto_norm * line.weight_;
//        two += okto_norm;
    }

    return {one, two};
}

static const int y_tile_part = 5;


void CalculateWeight(
        const CreateTileOptions& options,
        const size_t x,
        const size_t y,
        std::vector<Tile>& tiles)
{
    
    int current_tile_num = x + y * options.n;
    
    Tile &tile = tiles[current_tile_num];
    std::cerr << tile.lines.size() << std::endl;
    for (int i = 0; i < 256 * 256; ++i)
    {
        const Point &center_pixel = tile.GetPointCenterPixel(i);
        double one = 0.0, two = 0.0;         
        for (const auto& lines : {
            tile.lines,
            tiles[(options.n + 1) * y + x - 1].lines,
            tiles[(options.n + 1) * y + x + 1].lines,
            tiles[(options.n + 1) * y + x].lines,
            tiles[(options.n - 1) * y + x - 1].lines,
            tiles[(options.n - 1) * y + x + 1].lines,
            tiles[(options.n - 1) * y + x].lines,
            tiles[(options.n) * y + x + 1].lines,
            tiles[(options.n) * y + x - 1].lines})
        {
            const auto [one_data, two_data] = GetWeigth(center_pixel, lines);
            one += one_data;
            two += two_data;
        }

        two = 1.0;

        tile.weights[i] = one / two;
    }
}

void CreateTileThread(
        const CreateTileOptions& options,
        std::vector<Tile>& tiles,
        const size_t start_x,
        const size_t finish_x)
{
    for (size_t tile_number_x = start_x; tile_number_x < finish_x; ++tile_number_x)
    {
        for (int tile_number_y = 0; tile_number_y < options.m / y_tile_part; ++tile_number_y)
        {
            std::cerr << "Create tile " << tile_number_x << " " << tile_number_y << std::endl;
            CreateTile(options, tile_number_x, tile_number_y, tiles[tile_number_y * options.n + tile_number_x]);
        }
    }
}

void DistributeLineThread(
        const CreateTileOptions &options,
        const Ways &ways,
        const size_t start_x,
        const size_t finish_x,
        std::vector<Tile>& tiles)
{
    for (size_t tile_number_x = start_x; tile_number_x < finish_x; ++tile_number_x)
    {
        for (int tile_number_y = 0; tile_number_y < options.m / y_tile_part; ++tile_number_y)
        {
            DistributeLine(options, tile_number_x, tile_number_y, ways, tiles);
        }
    }
}

void CalculateWeightThread(
        const CreateTileOptions& options,
        const size_t start_x,
        const size_t finish_x,
        std::vector<Tile>& tiles)
{
    std::cerr << "Start calculate weight " << start_x << " " << finish_x << std::endl;
    for (size_t tile_number_x = start_x; tile_number_x < finish_x; ++tile_number_x)
    {
        for (int tile_number_y = 1; tile_number_y < options.m / y_tile_part - 1; ++tile_number_y)
        {
            std::cerr << "Calculate weight for tile " << tile_number_x << " " << tile_number_y << std::endl;
            CalculateWeight(options, tile_number_x, tile_number_y, tiles);
        }
    }
}

std::vector<Tile> CreateTiles(const CreateTileOptions& options, const Ways& ways)
{
    std::cerr << "Start create tile" << std::endl;
    std::vector<Tile> tiles(options.m * options.n);

    const unsigned int thread_count = std::thread::hardware_concurrency();

    const unsigned x_step = options.n / thread_count;

    {
        std::vector<std::thread> threads;
        for (size_t thread_number = 0; thread_number < thread_count - 1; ++thread_number)
        {
            threads.emplace_back(CreateTileThread,
                                 options,
                                 std::ref(tiles),
                                 x_step * thread_number,
                                 (x_step + 1) * thread_number);
        }
        threads.emplace_back(CreateTileThread, options, std::ref(tiles), x_step * (thread_count - 1), options.n);

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    std::cerr << "Start distribute line" << std::endl;
    std::cerr << ways.size() << std::endl;

    {
        std::vector<std::thread> threads;
        for (size_t thread_number = 0; thread_number < thread_count - 1; ++thread_number)
        {
            threads.emplace_back(DistributeLineThread,
                                 options,
                                 ways,
                                 x_step * thread_number,
                                 (x_step + 1) * thread_number,
                                 std::ref(tiles));
        }
        threads.emplace_back(DistributeLineThread,
                          options,
                          ways,
                          x_step * (thread_count - 1),
                          options.n,
                          std::ref(tiles));

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    {
        std::vector<std::thread> threads;
        for (size_t thread_number = 0; thread_number < thread_count - 1; ++thread_number)
        {
            threads.emplace_back(CalculateWeightThread,
                              options,
                              x_step * thread_number + 1,
                              (x_step + 1) * thread_number,
                              std::ref(tiles));
        }
        threads.emplace_back(CalculateWeightThread,
                             options,
                             x_step * (thread_count - 1),
                             options.n - 1,
                             std::ref(tiles));

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    return tiles;
}

void WriteTile(const Tile &tile, const std::string path_to_result, const int shift_x, const int shift_y)
{
    const std::string file_name = path_to_result + "/" + std::to_string(tile.x_index + shift_x) + "_" + std::to_string(shift_y - tile.y_index) + ".csv";

    std::ofstream stream(file_name);

    stream << "lat;lng;weight;index" << std::endl;
    for (int i = 0; i < 256 * 256; ++i)
    {
        const Point &point = tile.GetPointCenterPixel(i);
        stream << point.lat << ";"
               << point.lon << ";"
               << tile.weights[i] << ";"
               << i << std::endl;
    }
}

int main(int argc, const char *argv[])
{
    /*
     * leftBottomLat: 54.80068486732233 ​
     * leftBottomLng: 82.529296875 ​
     * leftBottomTileX: 5975 ​
     * leftBottomTileY: 2598 ​
     * numTileX: 15 ​
     * numTileY: 16 ​
     * rightUpperLat: 55.254077067072714 ​
     * rightUpperLng: 83.2763671875 ​
     * rightUpperTileX: 5990 ​
     * rightUpperTileY: 2582
     */

    CreateTileOptions options;

    {
        options.start_point.lat = 54.80068486732233;
        options.start_point.lon = 82.529296875;
        options.finish_point.lat = 55.254077067072714;
        options.finish_point.lon = 83.2763671875;
        options.n = 17;
        options.m = 18;
    }

    std::ifstream file("../graph_dump.json");

    if (!file.is_open())
    {
        throw std::runtime_error("Can't open file");
    }

    const auto& ways = WaysReader::Read(file);

    const auto& tiles = CreateTiles(options, ways);
    for (const auto& tile : tiles)
    {
        if (tile.x_index != 0 && tile.x_index != options.n - 1 &&
            tile.y_index != 0 && tile.y_index != options.m - 1)
        {
            WriteTile(tile, "../tiles", 5974, 2599);
        }
    }
}