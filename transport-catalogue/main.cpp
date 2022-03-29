#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

#include <iostream>
#include <cassert>

using namespace std;

using namespace TransportCatalogue;
using namespace TransportCatalogue::InputFunctions;
using namespace TransportCatalogue::OutputFunctions;

int main( ) {

    auto add_requests = GetLines(cin);
    auto get_requests = GetLines(cin);
    auto responses = ParseRequests(get_requests);
    auto input = ParseLines(add_requests);

    TransportCatalogue::TransportCatalogue tc(input.stops, input.buses, input.real_distances);

    for (auto& response : responses) {
        if (response.is_for_bus) {
            ResponseForBus r = tc.GetBusResponse(response.responses_for_bus);
            PrintBusResponse(r);
        } else {
            ResponseForStop r = tc.GetStopResponse(response.responses_for_stop);
            PrintStopResponse(r);
        }
    }

    return 0;
}
