#include "json_reader.h"

//#include <stdexcept>

//#include <fstream>
#include <iostream>

using namespace std;

int main() {
    /*
    graph::DirectedWeightedGraph<int> gr(4);
    graph::Edge<int> edge1{0, 1, 5};
    graph::Edge<int> edge2{1, 2, 6};
    graph::Edge<int> edge3{2, 0, 7};
    graph::Edge<int> edge4{2, 3, 8};
    graph::Edge<int> edge5{0, 2, 12};
    gr.AddEdge(edge1);
    gr.AddEdge(edge2);
    gr.AddEdge(edge3);
    gr.AddEdge(edge4);
    gr.AddEdge(edge5);
    graph::Router router(gr);

    auto info = router.BuildRoute(0,2);

    for (auto item : (*info).edges) {
        cout << gr.GetEdge(item).from << ":" << gr.GetEdge(item).to << ":" << gr.GetEdge(item).weight << endl;
    }
    cout << (*info).weight << endl;
    
    //TransportCatalogue::TransportCatalogue tc;
    fstream in("test1.txt"s, ios_base::in);
    fstream out("test1_out.txt"s, ios_base::out);
    try {
        JsonReader jr(in, out);
    } catch(std::exception& exc) {
        cout << exc.what() << endl;
    }*/
    JsonReader jr(cin, cout);

    return 0;
}
