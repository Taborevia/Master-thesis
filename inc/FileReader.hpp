#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <IGraph.hpp>

std::shared_ptr<IGraph> readGraphFromFile(const std::string& filename);
std::vector<std::pair<int,int>> readSequenceFromFile(const std::string& filename);
