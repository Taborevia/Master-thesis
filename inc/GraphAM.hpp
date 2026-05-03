#pragma once
#include <vector>
#include "IGraph.hpp"

class GraphAM : public IGraph {
public:
    GraphAM(uint32_t vertices);

    void addEdge(int u, int v, EdgeColor color = EdgeColor::Black) override;
    bool isEdge(int u, int v, EdgeColor color = EdgeColor::Black) const override;
    void print() const override;
    void contractVertices(int u, int v) override;
    int estimateTwinWidthAfterContraction(int u, int v) const override;
    int contractGraph(std::vector<std::pair<int,int>>) override;
    int getNumberOfVertices() const override;
    std::unique_ptr<IGraph> clone() const override;
    int getMaxRedDegree() const;

private:
    uint32_t vertices_;
    uint32_t wordsPerRow_;
    uint32_t maxRedDegree_ = 0;
    std::vector<uint64_t> adjBlack_;
    std::vector<uint64_t> adjRed_;
    std::vector<uint64_t> activeVertices_;
    std::vector<uint32_t> redDegrees_;

    inline void set_bit(std::vector<uint64_t>& vec, uint32_t u, uint32_t v);
    inline void clear_bit(std::vector<uint64_t>& vec, uint32_t u, uint32_t v);
    inline bool get_bit(const std::vector<uint64_t>& vec, uint32_t u, uint32_t v) const;
};