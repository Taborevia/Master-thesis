#pragma once
#include <vector>
#include <list>
#include <utility>
#include <memory>

enum class EdgeColor {
    Black,
    Red
};

class IGraph {
public:
    virtual ~IGraph() = default;

    virtual void addEdge(int u, int v, EdgeColor color = EdgeColor::Black) = 0;
    virtual bool isEdge(int u, int v, EdgeColor color = EdgeColor::Black) const = 0;
    virtual void print() const = 0;
    virtual void contractVertices(int u, int v) = 0;
    virtual int contractGraph(std::vector<std::pair<int,int>>) = 0;
    virtual int getNumberOfVertices() const = 0;
    virtual std::unique_ptr<IGraph> clone() const = 0;
};