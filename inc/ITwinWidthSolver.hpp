#pragma once
#include <vector>
#include <utility>

class ITwinWidthSolver {
public:
    virtual ~ITwinWidthSolver() = default;
    
    virtual void findSequence(float resources, float c_parameter, float D_parameter = 0) = 0;
    virtual int getBestTwinWidth() const = 0;
    virtual std::vector<std::pair<int,int>> getBestContractionSequence() const = 0;
};
