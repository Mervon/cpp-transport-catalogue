#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <map>

#include "transport_catalogue.h"
#include "stat_reader.h"

namespace TransportCatalogue {

namespace InputFunctions {

struct ParsedLines {
    std::vector<TransportCatalogue::Stop> stops;
    std::vector<std::pair<std::string, std::deque<std::string>>> buses;
    std::map<std::pair<std::string, std::string>, double> real_distances;
};

void RemovePrefix(std::string& s);

void RemoveSuffix(std::string& s);

std::vector<std::string> GetLines(std::istream& is);

ParsedLines ParseLines(std::vector<std::string>& strings);

std::vector<Response> ParseRequests(std::vector<std::string>& requests);
}
}
