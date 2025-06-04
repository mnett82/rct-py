#include <sstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <memory>
#include <algorithm>

#include "rct.h"
#include "DistData.h"

struct Vec final : public DistData
{
    std::vector<float> v;

    Vec(std::size_t len, float *values) {
        v.reserve(len);
        for (std::size_t i = 0 ; i < len; ++i) {
            v.push_back(values[i]);
        }
    }

    float distanceTo(DistData *other) override
    {
        const Vec *p = static_cast<Vec *>(other);

        float d = 0.0f;
        for (std::size_t i = 0; i < v.size(); ++i)
        {
            const float s = v[i] - p->v[i];
            d += s * s;
        }
        return sqrtf(d);
    }

    std::string debugString() {
        std::ostringstream out;
        out << "Vec(" << v[0];
        for (std::size_t i = 1; i < v.size(); ++i) {
            out << ", " << v[i];
        }
        out << ")";
        return out.str();
    }
};

struct RctWrapper
{
    int cols = 0;
    std::vector<Vec> points = {};
    std::vector<DistData *> data = {};
    std::unique_ptr<RCT> rct = nullptr;
};

extern "C" void *rct_build(
    unsigned long seed,
    int verbosity,
    float coverage,
    float sample_rate,
    void *data_ptr,
    int rows,
    int cols)
{
    auto *wrapper = new RctWrapper();

    // wrapper->cols = cols;

    wrapper->points.reserve(rows);
    for (int row = 0; row < rows; ++row)
    {
        wrapper->points.emplace_back(
            cols, &static_cast<float *>(data_ptr)[row * cols]);
    }

    // wrapper->data.reserve(rows);
    // for (int row = 0; row < rows; ++row)
    // {
    //     wrapper->data.push_back(static_cast<DistData *>(&wrapper->points[row]));
    // }

    // wrapper->rct = std::make_unique<RCT>(seed);
    // wrapper->rct->setVerbosity(verbosity);
    // wrapper->rct->setCoverageParameter(coverage);
    // wrapper->rct->setSampleRate(sample_rate);
    // wrapper->rct->build(wrapper->data.data(), wrapper->data.size());

    return wrapper;
}

extern "C" void rct_destroy(void *wrapper)
{
    delete static_cast<RctWrapper *>(wrapper);
}

extern "C" int rct_find_near(
    void *_wrapper,
    void *query_ptr,
    int how_many,
    int *result)
{
    auto *wrapper = static_cast<RctWrapper *>(_wrapper);

    Vec query(wrapper->cols, static_cast<float *>(query_ptr));

    std::vector<int> ids;
    ids.reserve(wrapper->points.size());
    for (int i = 0 ; i < static_cast<int>(wrapper->points.size()); ++i) {
        ids.push_back(i);
    }
    std::vector<float> dists(wrapper->points.size(), -1.0);

    std::sort(
        ids.begin(),
        ids.end(),
        [&](int i, int j) {
            if (dists[ids[i]] < 0.0) {
                dists[ids[i]] = wrapper->points[ids[i]].distanceTo(&query);
            }
            if (dists[ids[j]] < 0.0) {
                dists[ids[j]] = wrapper->points[ids[j]].distanceTo(&query);
            }
            return dists[ids[i]] < dists[ids[j]];
        });

    if (static_cast<int>(ids.size()) > how_many) {
        ids.resize(how_many);
    }

    for (std::size_t i = 0; i < ids.size(); ++i) {
        result[i] = ids[i];
    }

    // const int num_found = wrapper->rct->findNearest(&query, how_many);
    // wrapper->rct->getResultIndices(result, num_found);

    // printf("<<%d", result[0]);
    // for (int i = 1; i < num_found; ++i) {
    //     printf(" %d", result[i]);
    // }
    // printf(">>\n");

    // for (int i = 0; i < num_found; ++i) {
    //     printf("#%d --> %s\n", i, wrapper->points[result[i]].debugString().c_str());
    // }

    return static_cast<int>(ids.size());
}