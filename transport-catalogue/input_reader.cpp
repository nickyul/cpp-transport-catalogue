#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>

using namespace catalogue;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 **/
detail::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { nan, nan };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));
    return { lat, lng };
}

/**
* Удаляет пробелы в начале и конце строки
**/
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 **/
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }
    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    return results;
}

input::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void input::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

std::vector<std::pair<std::string_view, int>> ParseDistances(std::string_view str) {
    std::vector<std::pair<std::string_view, int>> result;
    std::vector<std::string_view> lines = Split(str, ',');
    for (size_t i = 2; i < lines.size(); ++i) {
        std::string_view s = lines[i];
        auto m_pos = s.find('m');
        std::string dist{ Trim(s.substr(0, m_pos)) };
        int distance = std::stoi(dist);

        std::string_view stop_name = Trim(s.substr(m_pos + 4));
        result.push_back({ stop_name, distance });
    }
    return result;
}

void input::InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    //commands_ -> catalogue

    // parsing stops
    for (const CommandDescription& command : commands_) {
        if (command.command == "Stop") {
            catalogue.AddStop(command.id, ParseCoordinates(command.description));
        }
        else {
            continue;
        }
    }
    for (const CommandDescription& command : commands_) {
        if (command.command == "Stop") {
            // method to add distance for stops
            const Stop* stop_from = catalogue.GetStop(command.id);
            for (const auto& pair : ParseDistances(command.description)) {
                const Stop* stop_to = catalogue.GetStop(pair.first);
                catalogue.AddDistance(stop_from, stop_to, pair.second);
            }
        }
        else {
            continue;
        }
    }
    // parsing buses
    for (const CommandDescription& command : commands_) {
        if (command.command == "Bus") {
            std::vector<const Stop*> stops;

            for (const std::string_view& stop_name : ParseRoute(command.description)) {
                const Stop* stop = catalogue.GetStop(stop_name);
                if (stop != nullptr) {
                    stops.push_back(stop);
                }
                else {
                    throw std::invalid_argument("Invalid stop on bus " + command.id);
                }
            }
            catalogue.AddBus(command.id, stops);
        }
        else {
            continue;
        }
    }
}
