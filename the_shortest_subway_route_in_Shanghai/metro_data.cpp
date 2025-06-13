#include <Poco/Net/Net.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Query.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/ParseHandler.h"
#include "Poco/JSON/PrintHandler.h"
#include "Poco/JSON/Template.h"
#include "Poco/StreamCopier.h"
#include <unordered_map>
#include <iostream>

using namespace std;

using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::DateTime;
using Poco::DateTimeFormatter;
using Poco::DynamicStruct;

using namespace Poco::Net;
using Poco::StreamCopier;

class Station
{
public:
    std::string name;
    std::set<std::string> lines;

    Station(const std::string &stationName) : name(stationName) {};

    void addLine(std::string lineNumber)
    {
        lines.insert(lineNumber);
    }
};

class StationManager
{
private:
    std::map<std::string, Station> stations;

public:
    void addStation(const std::string &name, std::string line)
    {
        auto it = stations.find(name);
        if (it != stations.end())
        {
            it->second.addLine(line);
        }
        else
        {
            Station newStation(name);
            newStation.addLine(line);
            // stations[name] = newStation;
            stations.insert({name, newStation});
        }
    }

    void printStationInfo(const std::string &name) const
    {
        auto it = stations.find(name);
        if (it != stations.end())
        {
            const Station &station = it->second;
            std::cout << " Station: " << name << " -> line: ";
            for (auto iter = station.lines.begin(); iter != station.lines.end(); ++iter)
            {
                if (iter != station.lines.begin())
                {
                    std::cout << ", ";
                }
                std::cout << *iter;
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Station not found: " << name << std::endl;
        }
    }

    void printAllInfo() const
    {
        for (const auto &pair : stations)
        {
            printStationInfo(pair.first);
        }
        std::cout << "Station count: " << stations.size() << std::endl;
    }

    std::vector<std::string> getSameLines(const Station &from, const Station &to) const
    {
        std::vector<std::string> commonLines;
        for (std::string line : from.lines)
        {
            if (to.lines.count(line) > 0)
            {
                commonLines.push_back(line);
            }
        }
        return commonLines;
    }

    const Station *getStation(const std::string &name) const
    {
        auto it = stations.find(name);
        if (it != stations.end())
        {
            return &(it->second);
        }
        return nullptr;
    }

    const std::map<std::string, Station> &getStations() const
    {
        return stations;
    }
};

class Route
{
public:
    std::string from_stop;
    std::string to_stop;
    int stops;
    std::string line_number;

    Route() : from_stop(""), to_stop(""), stops(9999), line_number("") {};
};

class Line
{
public:
    std::string line_number;
    std::vector<std::string> stations;

    Line(std::string ln) : line_number(ln) {};

    void add_station(const std::string &station_name)
    {
        stations.push_back(station_name);
    }
};

class LineManager
{
private:
    std::map<std::string, Line> lines;

public:
    void add_line(std::string line_number, const std::string &station_name)
    {
        auto it = lines.find(line_number);
        if (it != lines.end())
        {
            // lines.find(line_number) != lines.end()

            // lines[line_number].add_station(station_name);

            it->second.add_station(station_name);
        }
        else
        {
            // Line new_line(line_number);
            // new_line.add_station(station_name);
            //  lines[line_number] = new_line;
            // lines.insert({line_number, new_line});

            lines.emplace(line_number, Line(line_number)).first->second.add_station(station_name);
        }
    }

    void print_line_info(std::string line_number) const
    {
        std::cout << "Line: " << line_number << std::endl;
        std::cout << "Lines: ";

        auto it = lines.find(line_number);
        if (it != lines.end())
        {
            const Line &line = it->second;
            for (size_t i = 0; i < line.stations.size(); i++)
            {
                std::cout << line.stations[i];
                if (i < line.stations.size())
                {
                    std::cout << " -> ";
                }
            }
        }
        std::cout << std::endl;
    }

    void print_all_info() const
    {
        for (const auto &pair : lines)
        {
            print_line_info(pair.first);
            std::cout << std::endl;
        }
        std::cout << "Line count: " << lines.size() << std::endl;
    }

    Route get_best_route(const std::string &from_station, const std::string &to_station, const std::vector<std::string> &line_numbers) const
    {
        Route best_route;
        best_route.from_stop = from_station;
        best_route.to_stop = to_station;
        best_route.stops = 9999;
        best_route.line_number = "";

        for (std::string ln : line_numbers)
        {
            auto it = lines.find(ln);
            if (it != lines.end())
            {
                const Line &line = it->second;
                int start_index = -1;
                int stop_index = -1;
                for (size_t i = 0; i < line.stations.size(); i++)
                {
                    if (line.stations[i] == from_station)
                    {
                        start_index = static_cast<int>(i);
                    }
                    else if (line.stations[i] == to_station)
                    {
                        stop_index = static_cast<int>(i);
                    }
                }
                if (start_index != -1 && stop_index != -1)
                {
                    int stops = std::abs(start_index - stop_index);
                    if (stops < best_route.stops)
                    {
                        best_route.stops = stops;
                        best_route.line_number = ln;
                    }
                }
            }
        }
        return best_route;
    }

    void print_stops(std::string line_number, const std::string &from_stop, const std::string &to_stop) const
    {
        auto it = lines.find(line_number);

        if (it == lines.end())
        {
            std::cout << "Line not found: " << line_number << std::endl;
            return;
        }
        const Line &line = it->second;
        const std::vector<std::string> &stations = line.stations;

        // Find start and end indices
        int start_index = -1;
        int end_index = -1;
        for (size_t i = 0; i < stations.size(); i++)
        {
            if (stations[i] == from_stop)
            {
                start_index = static_cast<int>(i);
            }
            else if (stations[i] == to_stop)
            {
                end_index = static_cast<int>(i);
            }
        }

        if (start_index == -1 || end_index == -1)
        {
            std::cout << "One or both stop not found on line " << line_number << std::endl;
            return;
        }

        bool reverse_order = start_index > end_index;

        if (!reverse_order)
        {
            bool start_printing = false;

            for (const auto &each : stations)
            {
                if (each == from_stop)
                {
                    std::cout << each << " -> ";
                    start_printing = true;
                }
                else if (each == to_stop)
                {
                    std::cout << each;
                    start_printing = false;
                }
                else if (start_printing)
                {
                    std::cout << each << " -> ";
                }
            }
            std::cout << std::endl;
        }
        else
        {
            // Reverse order
            bool start_printing = false;
            for (auto rit = stations.rbegin(); rit != stations.rend(); ++rit)
            {
                // 把 const std::string &each = rit; 改成 const std::string &each = *rit;
                const std::string &each = *rit;
                if (each == from_stop)
                {
                    std::cout << each << " -> ";
                    start_printing = true;
                }
                else if (each == to_stop)
                {
                    std::cout << each;
                    start_printing = false;
                }
                else if (start_printing)
                {
                    std::cout << each << " -> ";
                }
            }
            std::cout << std::endl;
        }
    }
};

void getData()
{
    // get the context
    HTTPSClientSession s("m.shmetro.com");
    HTTPRequest request(HTTPRequest::HTTP_GET, "/core/shmetro/mdstationinfoback_new.ashx?act=getAllStations");
    s.sendRequest(request);
    X509Certificate cert = s.serverCertificate();
    HTTPResponse response;
    std::istream &rs = s.receiveResponse(response);
    std::ostringstream ostr;
    StreamCopier::copyStream(rs, ostr);
    std::string str(ostr.str());

    std::cout << "str: " << str << std::endl;
    //  trans to json
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;

    std::map<string, string> keyvalueMap;
    std::map<std::string, std::vector<std::string>> updatedMap;

    StationManager stationManager;
    LineManager lineManager;

    try
    {
        result = parser.parse(str);
    }
    catch (JSONException &jsone)
    {
        std::cout << jsone.message() << std::endl;
        // assertTrue(false);
    }
    Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

    for (int i = 0; i < arr->size(); i++)
    {
        Object::Ptr obj = arr->getObject(i);
        std::string key = obj->getValue<std::string>("key");
        std::string value = obj->getValue<std::string>("value");

        // 获取线路信息
        auto keyToline = [](const std::string &input) -> std::string
        {
            try
            {
                std::string truncated = input.substr(0, 2);
                int value = Poco::NumberParser::parse(truncated);
                return Poco::NumberFormatter::format(value);
            }
            catch (const Poco::Exception &)
            {
                return input.substr(0, 2);
            }
        };
        auto line = keyToline(key);

        keyvalueMap[key] = value;
        updatedMap[line].push_back(value);

        stationManager.addStation(value, line);
        lineManager.add_line(line, value);
    }

    std::cout << "打印所有地铁站信息" << std::endl;
    stationManager.printAllInfo();

    std::cout << "打印所有线路信息" << std::endl;
    lineManager.print_all_info();
    const Station *stationA = stationManager.getStation("上海松江站");
    const Station *stationB = stationManager.getStation("曹路");
    if (stationA && stationB)
    {
        std::vector<std::string> commonLines = stationManager.getSameLines(*stationA, *stationB);
        std::cout << "Common lines between 上海松江站 and 曹路: ";
        for (std::string line : commonLines)
        {
            if (!line.empty())
            {
                std::cout << line << " ";
            }
            else
            {
                std::cout << "not found common line...";
            }
        }
        std::cout << std::endl;

        Route best = lineManager.get_best_route("上海松江站", "曹路", commonLines);
        std::cout << "Best route from " << best.from_stop << " to " << best.to_stop
                  << " on line " << best.line_number << " with " << best.stops << " stops." << std::endl;
        lineManager.print_stops(best.line_number, "上海松江站", "曹路");
    }
}

std::pair<StationManager, LineManager> getData2()
{
    StationManager stationManager;
    LineManager lineManager;

    // get the context
    HTTPSClientSession s("m.shmetro.com");
    HTTPRequest request(HTTPRequest::HTTP_GET, "/core/shmetro/mdstationinfoback_new.ashx?act=getAllStations");
    s.sendRequest(request);
    X509Certificate cert = s.serverCertificate();
    HTTPResponse response;
    std::istream &rs = s.receiveResponse(response);
    std::ostringstream ostr;
    StreamCopier::copyStream(rs, ostr);
    std::string str(ostr.str());

    // std::cout << "str: " << str << std::endl;
    //   trans to json
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result;

    try
    {
        result = parser.parse(str);
    }
    catch (JSONException &jsone)
    {
        std::cout << jsone.message() << std::endl;
        // assertTrue(false);
    }
    Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();

    for (int i = 0; i < arr->size(); i++)
    {
        Object::Ptr obj = arr->getObject(i);
        std::string key = obj->getValue<std::string>("key");
        std::string value = obj->getValue<std::string>("value");

        // 获取线路信息
        auto keyToline = [](const std::string &input) -> std::string
        {
            try
            {
                std::string truncated = input.substr(0, 2);
                int value = Poco::NumberParser::parse(truncated);
                return Poco::NumberFormatter::format(value);
            }
            catch (const Poco::Exception &)
            {
                return input.substr(0, 2);
            }
        };
        auto line = keyToline(key);

        stationManager.addStation(value, line);
        lineManager.add_line(line, value);
    }

    return {stationManager, lineManager};
}

int main()
{
    std::cout << "开始获取地铁数据..." << std::endl;
    // getData();

    auto [station_manager, line_manager] = getData2();

    std::vector<Station> stations;
    std::map<std::string, int> station_index;
    std::vector<std::vector<Route>> v_matrix;

    std::vector<int> book;
    std::vector<std::pair<int, std::vector<Route>>> dis;

    int n = 0;
    int index = 0;
    for (const auto &pair : station_manager.getStations())
    {
        station_index[pair.first] = index++;
        stations.push_back(pair.second);
    }

    n = stations.size();

    for (size_t i = 0; i < static_cast<size_t>(n); i++)
    {
        v_matrix.push_back(std::vector<Route>(n, Route()));
        for (size_t j = 0; j < static_cast<size_t>(n); j++)
        {
            if (i != j)
            {
                auto same_lines = station_manager.getSameLines(stations[i], stations[j]);
                v_matrix[i][j] = line_manager.get_best_route(stations[i].name, stations[j].name, same_lines);
            }
        }
    }

    std::string start_station, terminal_station;
    std::cout << "起始站\n";
    std::getline(std::cin, start_station);
    std::cout << "终点站\n";
    std::getline(std::cin, terminal_station);

    int start_index = station_index[start_station];
    int terminal_index = station_index[terminal_station];

    std::cout << "通过 Dijkstra 统计最短路线\n";
    dis.resize(n, {std::numeric_limits<int>::max(), {}});
    book.resize(n, 0);

    for (size_t i = 0; i < static_cast<size_t>(n); i++)
    {
        dis[i] = {v_matrix[start_index][i].stops, {v_matrix[start_index][i]}};
    }

    book[start_index] = 1;

    int bias = 30;
    for (int i = 0; i < n - 1; i++)
    {
        int min_stops = std::numeric_limits<int>::max();
        int u = 0;
        for (int j = 0; j < n; j++)
        {
            if (book[j] == 0 && dis[j].first < min_stops)
            {
                min_stops = dis[j].first;
                u = j;
            }
        }
        book[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (v_matrix[u][v].stops < std::numeric_limits<int>::max())
            {
                int new_dist = dis[u].first + v_matrix[u][v].stops + (dis[u].first != 0 ? bias : 0);
                if (book[v] == 0 && dis[v].first > new_dist)
                {
                    std::vector<Route> new_path = dis[u].second;
                    if (v_matrix[u][v].stops != 9999)
                    {
                        new_path.push_back(v_matrix[u][v]);
                    }
                    dis[v] = {new_dist, new_path};
                }
            }
        }
    }

    std::cout << "打印解决方案\n";
    const auto &solution = dis[terminal_index].second;
    for (const auto &route : solution)
    {
        if (route.stops != 9999)
        {
            std::cout << "在 " << route.from_stop << " 乘坐 " << route.line_number << " 号线 到 " << route.to_stop << " ( " << route.stops << " 站) " << std::endl;
            line_manager.print_stops(route.line_number, route.from_stop, route.to_stop);
        }
    }

    std::cin.get();
    return 0;
}
// 顾村公园 东方体育中心