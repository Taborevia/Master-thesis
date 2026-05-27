#include "readwrite/pace_2023.hpp"

#include "MDAdapter.hpp"
#include "GraphAL.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "FileReader.hpp"

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

MDAdapter::MDAdapter(std::string filename) {
    filename_ = filename;
    graph_ = readwrite::load_pace_2023(filename.c_str());
    mdTree_ = modular::MDTree(graph_, false);
}

modular::MDTree MDAdapter::getMDTree(){
    return mdTree_;
}

std::vector<std::pair<int, int>> MDAdapter::normalizeContractionSequence(std::vector<std::pair<int, int>> const& seq) {
  std::vector<std::pair<int, int>> ret;
  std::unordered_map<int, int> mapping;
  for (int i = 0; i < static_cast<int>(seq.size()); ++i) { mapping[seq[i].second] = seq[i].second; }
  mapping[seq.back().first] = seq.back().first;

  for (auto& p : seq) {
    int i = p.first;
    int j = p.second;
    int u = std::min(mapping[i], mapping[j]);
    int v = std::max(mapping[i], mapping[j]);
    mapping[i] = u;
    mapping[j] = v;
    ret.push_back({u, v});
  }
  return ret;
}

std::shared_ptr<IGraph> MDAdapter::induceAndRelabelGraph(std::vector<int> const& vertices){
    std::unordered_map<int, int> label_map;  // current label to new label
    int32_t graphSize = graph_.number_of_nodes();
    int32_t inducedGraphSize = vertices.size();

    std::vector<bool> activeVertices(graph_.number_of_nodes(), 0);

    for (int i = 0; i < inducedGraphSize; ++i) {
        assert(0 <= vertices[i] && vertices[i] < graphSize);
        activeVertices[vertices[i]] = true;
        label_map.insert({vertices[i], i});
    }

    auto inducedGraph = std::make_shared<GraphAL>(inducedGraphSize);

    for (int i = 0; i < graphSize; ++i) {
        if (!activeVertices[i]) continue;
        for (int j : graph_.neighbors(i)) {
            if (j <= i || !activeVertices[j]) continue;
            inducedGraph->addEdge(label_map.at(i), label_map.at(j));
        }
    }

    return inducedGraph;
}


void MDAdapter::findSequence(float resources, float c_parameter, float D_parameter) {
    auto start_time = std::chrono::steady_clock::now();

    auto const& t = mdTree_.get_tree();
    std::vector<int> nodes;

    for (auto i : t.dfs_reverse_preorder_nodes(mdTree_.get_root())) {
      if (!t[i].is_leaf()) nodes.push_back(i);
    }

    // Traverse from the bottom of the modular decomposition tree.
    std::reverse(nodes.begin(), nodes.end());

    std::unordered_map<int, std::vector<int>> leaders;
    std::unordered_map<int, std::vector<std::pair<int, int>>> partial_results;
    std::vector<std::pair<int, int>> prime_stats;
    int32_t sum_prime_graphs_size = 0;

    for (auto node : nodes) {
      // Given a module, find the leader for each child module.
      // Assuming that vertex v gets merged into vertex u only if u's label is smaller than v's,
      // every leader must have the minimum label among its module.
      std::vector<int> vs;
      for (auto child : t.get_children(node)) {
        int leader = std::numeric_limits<int>::max();
        for (int j = t[child].data.vertices_begin; j < t[child].data.vertices_end; ++j) {
          leader = std::min(leader, mdTree_.get_vertex(j));
        }
        vs.push_back(leader);
      }
      leaders[node] = vs;

      if (t[node].data.is_prime_node()) {
        // prime -> store information and solve later
        prime_stats.push_back({vs.size(), node});
        sum_prime_graphs_size += vs.size();
      } else {
        // non-prime -> immediately create results
        for (int i = 1; i < static_cast<int>(vs.size()); ++i) {
        // any contraction is safe
        partial_results[node].push_back({0, i});
        }
      }
    }

    int32_t num_prime_graphs = prime_stats.size();

    auto end_time = std::chrono::steady_clock::now();
    auto decomposition_duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
    std::cout << "Modular decomposition + prime graph extraction time: " << decomposition_duration << " s\n";
    std::cout << "Number of prime graphs: " << num_prime_graphs << "\n";
    std::cout << "Total prime vertices: " << sum_prime_graphs_size << "\n";
    auto left_time = resources - decomposition_duration;

    std::vector<std::shared_ptr<IGraph>> prime_graphs;

    for (int graph_id = 0; graph_id < num_prime_graphs; ++graph_id) {
      int node_id = prime_stats[graph_id].second;
      auto const& vs = leaders[node_id];

      auto H = induceAndRelabelGraph(vs);
      assert(H->getNumberOfVertices() == prime_stats[graph_id].first);

      //process prime graph H with primeSolver

      float time_for_this_graph = left_time * (static_cast<float>(prime_stats[graph_id].first) / static_cast<float>(sum_prime_graphs_size));
      auto solver = std::make_unique<MonteCarloTreeSearch_v1>(H, 100, 10, 10, 10.0, 0, 100);
      solver->findSequence(time_for_this_graph, c_parameter, D_parameter);
      int tw = solver->getBestTwinWidth();
      auto sequence = solver->getBestContractionSequence();
      partial_results[node_id] = sequence;
    }

    // Lastly, compose the contraction sequence from the bottom
    bestContractionSequence_.clear();
    for (auto node : nodes) {
      assert(partial_results[node].size() == leaders[node].size() - 1);

      // convert vertex labels
      std::vector<std::pair<int, int>> seq;
      for (auto& p : partial_results[node]) seq.push_back({leaders[node][p.first], leaders[node][p.second]});
      
      for (auto& p : normalizeContractionSequence(seq)) bestContractionSequence_.push_back(p);
    }
    auto graph = readGraphFromFile(filename_);
    bestTwinWidth_ = graph->contractGraph(bestContractionSequence_);
}



int MDAdapter::getBestTwinWidth() const {
    return bestTwinWidth_;
}

std::vector<std::pair<int,int>> MDAdapter::getBestContractionSequence() const {
    return bestContractionSequence_;
}
