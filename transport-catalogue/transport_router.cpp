#include "transport_router.h"

namespace graph {

void TransportRouter::ProcessRouter(const graph::DirectedWeightedGraph<double>& graph) {
    router_ = std::unique_ptr<graph::Router<double>>(new graph::Router<double>(&graph));
}

const std::unique_ptr<graph::Router<double>>& TransportRouter::GetRouter() const {
    return router_;
}
}
