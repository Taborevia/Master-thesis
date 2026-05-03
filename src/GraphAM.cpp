#include "GraphAM.hpp"
#include <iostream>
#include <iomanip>
#include <memory>
#include <bit>

GraphAM::GraphAM(uint32_t vertices) : vertices_(vertices) {
        wordsPerRow_ = (vertices_ + 63) / 64;
        adjBlack_.assign(vertices_ * wordsPerRow_, 0);
        adjRed_.assign(vertices_ * wordsPerRow_, 0);
        activeVertices_.assign(wordsPerRow_, ~0ULL);
        redDegrees_.assign(vertices_, 0);
        
        if (vertices_ % 64 != 0) {
            activeVertices_.back() &= (1ULL << (vertices_ % 64)) - 1;
        }
    }

    inline void GraphAM::set_bit(std::vector<uint64_t>& vec, uint32_t u, uint32_t v) {
        vec[u * wordsPerRow_ + (v >> 6)] |= (1ULL << (v & 63));
    }

    inline void GraphAM::clear_bit(std::vector<uint64_t>& vec, uint32_t u, uint32_t v) {
        vec[u * wordsPerRow_ + (v >> 6)] &= ~(1ULL << (v & 63));
    }

    inline bool GraphAM::get_bit(const std::vector<uint64_t>& vec, uint32_t u, uint32_t v) const {
        return vec[u * wordsPerRow_ + (v >> 6)] & (1ULL << (v & 63));
    }

void GraphAM::addEdge(int u, int v, EdgeColor color) {
    if (color == EdgeColor::Black) {
        set_bit(adjBlack_, u, v);
        set_bit(adjBlack_, v, u);
    } else {
        set_bit(adjRed_, u, v);
        set_bit(adjRed_, v, u);
        redDegrees_[u]++;
        redDegrees_[v]++;
        if (redDegrees_[u] > maxRedDegree_) maxRedDegree_ = redDegrees_[u];
        if (redDegrees_[v] > maxRedDegree_) maxRedDegree_ = redDegrees_[v];
    }
}

bool GraphAM::isEdge(int u, int v, EdgeColor color) const {
    if (color == EdgeColor::Black) {
        return get_bit(adjBlack_, u, v);
    } else {
        return get_bit(adjRed_, u, v);
    }
}

void GraphAM::print() const {
    for (int i = 0; i < vertices_; ++i) {
        if ((activeVertices_[i >> 6] & (1ULL << (i & 63))) == 0) {
            continue; 
        }
        for (int j = 0; j < vertices_; ++j) {
            if ((activeVertices_[j >> 6] & (1ULL << (j & 63))) == 0) {
                continue; 
            }
            if (get_bit(adjRed_, i, j)) {
                std::cout << "R ";
            } else if (get_bit(adjBlack_, i, j)) {
                std::cout << "B ";
            } else {
                std::cout << "- ";
            }
        }
        std::cout << std::endl;
    }
}

int GraphAM::estimateTwinWidthAfterContraction(int u, int v) const {
    int u_offset = u * wordsPerRow_;
    int v_offset = v * wordsPerRow_;
    
    uint32_t maxRedDegree = maxRedDegree_;
    uint32_t newRedDegree_U = 0;
    
    int u_block = u / 64;
    int v_block = v / 64;
    uint64_t u_bit = 1ULL << (u % 64);
    uint64_t v_bit = 1ULL << (v % 64);

    for (int i = 0; i < wordsPerRow_; ++i) {
        uint64_t b_u = adjBlack_[u_offset + i];
        uint64_t b_v = adjBlack_[v_offset + i];
        uint64_t r_u = adjRed_[u_offset + i];
        uint64_t r_v = adjRed_[v_offset + i];

        uint64_t new_red_u = r_u | r_v | (b_u ^ b_v);
        
        if (i == u_block) new_red_u &= ~u_bit;
        if (i == v_block) new_red_u &= ~v_bit;
        
        newRedDegree_U += std::popcount(new_red_u);

        uint64_t increase_mask = (b_u ^ b_v) & ~r_u & ~r_v;
        
        if (i == u_block) increase_mask &= ~u_bit;
        if (i == v_block) increase_mask &= ~v_bit;

        while (increase_mask != 0) {
            int bit_pos = std::countr_zero(increase_mask);
            int w = (i * 64) + bit_pos;
            
            uint32_t estimated_w_deg = redDegrees_[w] + 1;
            
            if (estimated_w_deg > maxRedDegree) {
                maxRedDegree = estimated_w_deg;
            }
            
            increase_mask &= (increase_mask - 1); // Gaszenie najniższej jedynki
        }
    }
    
    if (newRedDegree_U > maxRedDegree) {
        maxRedDegree = newRedDegree_U;
    }

    return maxRedDegree;
}

// make contraction to u and deactivate v
void GraphAM::contractVertices(int u, int v) {
    uint32_t u_offset = u * wordsPerRow_;
    uint32_t v_offset = v * wordsPerRow_;

    uint32_t u_block = u / 64;
    uint32_t v_block = v / 64;
    uint64_t u_bit = 1ULL << (u % 64);
    uint64_t v_bit = 1ULL << (v % 64);

    uint32_t newRedDegree = 0;
    
    for (size_t i = 0; i < wordsPerRow_; ++i) {
        uint64_t b_u = adjBlack_[u_offset + i];
        uint64_t b_v = adjBlack_[v_offset + i];
        uint64_t r_u = adjRed_[u_offset + i];
        uint64_t r_v = adjRed_[v_offset + i];

        uint64_t new_r = r_u | r_v | (b_u ^ b_v);
        uint64_t new_b = b_u & b_v;

        if (i == u_block) { new_r &= ~u_bit; new_b &= ~u_bit; }
        if (i == v_block) { new_r &= ~v_bit; new_b &= ~v_bit; }

        adjRed_[u_offset + i] = new_r;
        adjBlack_[u_offset + i] = new_b;

        newRedDegree += std::popcount(new_r); 
    }
    
    redDegrees_[u] = newRedDegree;
    if (newRedDegree > maxRedDegree_) {
        maxRedDegree_ = newRedDegree;
    }

    activeVertices_[v_block] &= ~v_bit;
    redDegrees_[v] = 0;

    for (size_t i = 0; i < wordsPerRow_; ++i) {
        uint64_t active_mask = activeVertices_[i];

        if (i == u_block) {
            active_mask &= ~u_bit;
        }

        while (active_mask != 0) {
            uint32_t bit_pos = std::countr_zero(active_mask);
            uint32_t w = (i * 64) + bit_pos;
            
            bool old_w_u_red = get_bit(adjRed_, w, u);
            bool old_w_v_red = get_bit(adjRed_, w, v);
            bool new_w_u_red = get_bit(adjRed_, u, w);

            if (new_w_u_red && !old_w_u_red) {
                redDegrees_[w]++;
            }
            if (old_w_v_red) {
                redDegrees_[w]--;
            }

            if (redDegrees_[w] > maxRedDegree_) {
                maxRedDegree_ = redDegrees_[w];
            }

            if (new_w_u_red) {
                set_bit(adjRed_, w, u);
                clear_bit(adjBlack_, w, u);
            } else if (get_bit(adjBlack_, u, w)) {
                set_bit(adjBlack_, w, u);
                clear_bit(adjRed_, w, u);
            } else {
                clear_bit(adjBlack_, w, u);
                clear_bit(adjRed_, w, u);
            }

            clear_bit(adjBlack_, w, v);
            clear_bit(adjRed_, w, v);

            active_mask &= (active_mask - 1); 
        }
    }
}

int GraphAM::contractGraph(std::vector<std::pair<int,int>> contractionSequnce){
    for (const auto& contraction : contractionSequnce){
        contractVertices(contraction.first, contraction.second);
    }
    return maxRedDegree_;
}

int GraphAM::getMaxRedDegree() const{
    return maxRedDegree_;
};

int GraphAM::getNumberOfVertices() const {
    return vertices_;
}

std::unique_ptr<IGraph> GraphAM::clone() const {
    return std::make_unique<GraphAM>(*this);
}
