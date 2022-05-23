#pragma once

#include "router.h"

#include <memory>

namespace graph {

class TransportRouter : public Router<double> {
public:

    TransportRouter() = default;

    void ProcessRouter(const graph::DirectedWeightedGraph<double>& graph);

    const std::unique_ptr<graph::Router<double>>& GetRouter() const;
private:
    std::unique_ptr<graph::Router<double>> router_;
};

}
