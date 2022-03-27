#include <string_view>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <utility>

#include "input_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace TransportCatalogue {

namespace InputFunctions {

void RemovePrefix(string& s) {
    s = s.substr(s.find_first_not_of(' '));
}

void RemoveSuffix(string& s) {
    while(*(s.end() - 1) == ' ') {
        s = s.substr(0, s.end() - s.begin() - 1);
    }
}

vector<string> GetLines(std::istream& is) {
    string buffer;
    getline(is, buffer, '\n');
    int counter = stoi( buffer );
    vector<string> result;
    for (int i = 0; i < counter; ++i) {
        getline(is, buffer, '\n');
        RemovePrefix(buffer);
        result.push_back(buffer);
    }
    return result;
}

ParsedLines ParseLines(vector<string>& strings) {
    ParsedLines requests;
    for (auto& str : strings) {
        if (str[0] == 'S') {
            str = str.substr(str.find_first_of(' '));
            auto it_1 = str.find_first_not_of(' ');
            auto it_2 = str.find_first_of(':');
            string stop_name = str.substr(it_1, it_2 - it_1);
            RemoveSuffix(stop_name);
            str = str.substr(str.find_first_of(':') + 1);
            it_1 = str.find_first_not_of(' ');
            it_2 = str.find_first_of(',');
            string coord_1 = str.substr(it_1, it_2 - it_1);
            str = str.substr(str.find_first_of(',') + 1);
            it_1 = str.find_first_not_of(' ');
            it_2 = str.find_first_of(',');
            string coord_2 = str.substr(it_1, it_2 - it_1);
            requests.stops.push_back(TransportCatalogue::Stop{stop_name, stod(coord_1), stod(coord_2)});
            if (str.find_first_of(',') != string::npos) {
                str = str.substr(str.find_first_of(',') + 1);
                while (str.find(',') != string::npos) {
                    it_1 = str.find_first_not_of(' ');
                    it_2 = str.find_first_of('m');
                    string stop_real_distance = str.substr(it_1, it_2 - it_1);
                    str = str.substr(str.find_first_of('m') + 1);
                    RemovePrefix(str);
                    str = str.substr(str.find_first_of(' '));
                    it_1 = str.find_first_not_of(' ');
                    it_2 = str.find_first_of(',');
                    string to_name = str.substr(it_1, it_2 - it_1);
                    RemovePrefix(to_name);
                    RemoveSuffix(to_name);
                    str = str.substr(str.find_first_of(',') + 1);
                    requests.real_distances[{stop_name, to_name}] = stod(stop_real_distance);
                }
                it_1 = str.find_first_not_of(' ');
                it_2 = str.find_first_of('m');
                string stop_real_distance = str.substr(it_1, it_2 - it_1);
                str = str.substr(str.find_first_of('m') + 1);
                RemovePrefix(str);
                str = str.substr(str.find_first_of(' '));
                it_1 = str.find_first_not_of(' ');
                it_2 = str.find_first_of('\n');
                string to_name = str.substr(it_1, it_2 - it_1);
                RemovePrefix(to_name);
                RemoveSuffix(to_name);
                requests.real_distances[{stop_name, to_name}] = stod(stop_real_distance);
            }
        } else if (str[0] == 'B') {
            str = str.substr(str.find_first_of(' '));
            auto it_1 = str.find_first_not_of(' ');
            auto it_2 = str.find_first_of(':');
            string bus_name = str.substr(it_1, it_2 - it_1);
            RemoveSuffix(bus_name);
            str = str.substr(str.find_first_of(':') + 1);
            deque<string> stops_names;
            if (str.find('>') != string::npos) {
                while (str.find('>') != string::npos) {
                    it_1 = str.find_first_not_of(' ');
                    it_2 = str.find_first_of('>');
                    string stop_name = str.substr(it_1, it_2 - it_1);
                    RemovePrefix(stop_name);
                    RemoveSuffix(stop_name);
                    stops_names.push_back(stop_name);
                    str = str.substr(str.find_first_of('>') + 1);
                }
                it_1 = str.find_first_not_of(' ');
                it_2 = str.find_first_of('\n');
                string stop_name = str.substr(it_1, it_2 - it_1);
                RemovePrefix(stop_name);
                RemoveSuffix(stop_name);
                stops_names.push_back(stop_name);
            } else {
                while (str.find('-') != string::npos) {
                    it_1 = str.find_first_not_of(' ');
                    it_2 = str.find_first_of('-');
                    string stop_name = str.substr(it_1, it_2 - it_1);
                    RemovePrefix(stop_name);
                    RemoveSuffix(stop_name);
                    stops_names.push_back(stop_name);
                    str = str.substr(str.find_first_of('-') + 1);
                }
                it_1 = str.find_first_not_of(' ');
                it_2 = str.find_first_of('\n');
                string stop_name = str.substr(it_1, it_2 - it_1);
                RemovePrefix(stop_name);
                RemoveSuffix(stop_name);
                stops_names.push_back(stop_name);
                deque<string> tmp_vector = stops_names;
                tmp_vector.pop_back();
                reverse(tmp_vector.begin(), tmp_vector.end());
                for (auto& item : tmp_vector) {
                    stops_names.push_back(item);
                }
            }
            requests.buses.push_back({bus_name, stops_names});
        }
    }
    return requests;
}

vector<Response> ParseRequests(vector<string>& requests) {
    vector<Response> result;
    for (auto& request : requests) {
        RemovePrefix(request);
        if (request[0] == 'B') {
            string tmp = request.substr(request.find_first_of(' '));
            auto it_1 = tmp.find_first_not_of(' ');
            auto it_2 = tmp.find_first_of('\n');
            tmp = tmp.substr(it_1, it_2 - it_1);
            RemoveSuffix(tmp);
            result.push_back({"" , tmp, true});
        } else if (request[0] == 'S') {
            string tmp = request.substr(request.find_first_of(' '));
            auto it_1 = tmp.find_first_not_of(' ');
            auto it_2 = tmp.find_first_of('\n');
            tmp = tmp.substr(it_1, it_2 - it_1);
            RemoveSuffix(tmp);
            result.push_back({tmp, "", false});
        }
    }
    return result;
}
}
}
