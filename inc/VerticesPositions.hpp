#pragma once
#include <vector>

struct VerticesPositions {
    std::vector<int> vertices;
    std::vector<int> positions;

    VerticesPositions(int n) : vertices(n), positions(n) {
        for (int i = 0; i < n; ++i) {
            vertices[i] = i;
            positions[i] = i;
        }
    }

    void erase(int vertex) {
        int pos = positions[vertex];
        int lastVertex = vertices.back();
        std::swap(vertices[pos], vertices.back());
        std::swap(positions[vertex], positions[lastVertex]);
        positions[vertex] = -1; // Mark as erased
        vertices.pop_back();
    }

    VerticesPositions clone() const {
        VerticesPositions copy(*this);
        return copy;
    }

};