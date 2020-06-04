#include "mwpm.h"
#include <PerfectMatching.h>
#include "stabiliser_graph.h"
#include "unweighted_stabiliser_graph.h"
#include "weighted_stabiliser_graph.h"
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <stdexcept>


py::array_t<std::uint8_t> Decode(const IStabiliserGraph& sg, const py::array_t<int>& defects){
    auto d = defects.unchecked<1>();
    int num_nodes = d.shape(0);
    int num_edges = num_nodes*(num_nodes-1)/2;
    PerfectMatching *pm = new PerfectMatching(num_nodes, num_edges);
    for (py::size_t i = 0; i<num_nodes; i++){
        for (py::size_t j=i+1; j<num_nodes; j++){
            pm->AddEdge(i, j, sg.SpaceTimeDistance(d(i), d(j)));
        }
    };
    pm->options.verbose = false;
    pm->Solve();
    int N = sg.GetNumQubits();
    auto correction = new std::vector<int>(N, 0);
    for (py::size_t i = 0; i<num_nodes; i++){
        int j = pm->GetMatch(i);
        if (i<j){
            std::vector<int> path = sg.SpaceTimeShortestPath(d(i), d(j));
            for (std::vector<int>::size_type k=0; k<path.size()-1; k++){
                int qid = sg.QubitID(path[k], path[k+1]);
                if ((qid != -1) && (qid >= 0) && (qid < N)){
                    (*correction)[qid] = ((*correction)[qid] + 1) % 2;
                }
            }
        }
    }
    delete pm;
    auto capsule = py::capsule(correction, [](void *correction) { delete reinterpret_cast<std::vector<int>*>(correction); });
    return py::array_t<int>(correction->size(), correction->data(), capsule);
}


// py::array_t<std::uint8_t> Decode(WeightedStabiliserGraph& sg, const py::array_t<std::uint8_t>& syndrome, int num_neighbours){
//     auto z = syndrome.unchecked<1>();
//     int z_size = z.shape(0);
//     int num_nodes = 0;
//     for (int i=0; i<z_size; i++){
//         if (z(i)==1){
//             num_nodes++;
//         }
//     }

//     std::vector<int> defect_id;
//     int id=0;
//     for (int i=0; i<z_size; i++){
//         if (z(i)==0){
//             defect_id.push_back(-1);
//         } else if (z(i)==1) {
//             defect_id.push_back(id);
//             id++;
//         } else {
//             throw std::runtime_error("Expected syndrome vector to contain either 0 or 1.");
//         }
//     }

//     num_neighbours = std::min(num_neighbours, num_nodes-1);
//     int num_edges = (num_nodes * num_neighbours)/2;
//     PerfectMatching *pm = new PerfectMatching(num_nodes, num_edges);

//     std::vector<std::pair<int, double>> neighbours;
//     for (int i=0; i<defect_id.size(); i++){
//         if (defect_id[i] != -1){
//             neighbours = sg->GetNearestNeighbours(i, defect_id);
//         }
//     }


//     delete pm;
// }