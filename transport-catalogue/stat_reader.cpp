#include "stat_reader.h"

#include <iomanip>

using namespace std;

namespace TransportCatalogue {

namespace OutputFunctions{

void PrintStopResponse(ResponseForStop& r, ostream& os) {
    if (r.is_stop_exists) {
        if (r.is_buses_exists) {
            os << "Stop "s << r.stop_name << ": buses "s;
            for (auto& bus_name : r.bus_names) {
                if (bus_name != *(--r.bus_names.end())) {
                    os << bus_name << " "s;
                }
            }
            os << *(--r.bus_names.end()) << std::endl;
        } else {
            os << "Stop "s << r.stop_name << ": no buses"s << std::endl;
        }
    } else {
        os << "Stop "s << r.stop_name << ": not found"s << std::endl;
    }
}

void PrintBusResponse(ResponseForBus& r, ostream& os) {
    if (r.is_bus_exist) {
        os << std::setprecision(6) << "Bus "s << r.bus_name << ": "s << r.stops_count << " stops on route, "s << r.unique_stops_count << " unique stops, "s << r.route_real_lenght << " route length, "s << r.curvature << " curvature"s << std::endl;
    } else {
        os << "Bus "s << r.bus_name << ": not found"s << std::endl;
    }
}
}
}
