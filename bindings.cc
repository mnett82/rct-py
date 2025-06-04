#include <cmath>
#include <cstdio>
#include <vector>
#include <memory>

#include "rct.h"
#include "DistData.h"

struct Vec final : public DistData
{
    std::size_t len;
    float *values;

    Vec(std::size_t len, float *values) : len(len), values(values) {}

    float distanceTo(DistData *other) override
    {
        const Vec *p = static_cast<Vec *>(other);

        float d = 0.0f;
        for (int i = 0; i < len; ++i)
        {
            const float s = values[i] - p->values[i];
            d += s * s;
        }
        return sqrtf(d);
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

    wrapper->cols = cols;

    wrapper->points.reserve(rows);
    for (int row = 0; row < rows; ++row)
    {
        wrapper->points.emplace_back(
            cols, &static_cast<float *>(data_ptr)[row * cols]);
    }

    wrapper->data.reserve(rows);
    for (int row = 0; row < rows; ++row)
    {
        wrapper->data.push_back(static_cast<DistData *>(&wrapper->points[row]));
    }

    wrapper->rct = std::make_unique<RCT>(seed);
    wrapper->rct->setVerbosity(verbosity);
    wrapper->rct->setCoverageParameter(coverage);
    wrapper->rct->setSampleRate(sample_rate);
    wrapper->rct->build(wrapper->data.data(), wrapper->data.size());

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

    float m = 0.0f;
    for (int i = 0; i < wrapper->cols; ++i) {
        m+= pow(query.values[i], 2.0);
    }
    m = sqrtf(m);
    printf("Query vector length: %f\n", m);

    // printf("Q [[ %f", query.values[0]);
    // for (int i = 1; i < query.len; ++i) {
    //     printf(", %f", query.values[i]);
    // }
    // printf(" ]]\n");

    const int num_found = wrapper->rct->findNearest(&query, how_many);
    wrapper->rct->getResultIndices(result, num_found);

    for (int i = 0; i < num_found; ++i) {
        result[i] += 1;
    }

    // printf("<< %d", result[0]);
    // for (int i = 1; i < num_found; ++i) {
    //     printf(", %d", result[i]);
    // }
    // printf(" >>\n");

    return num_found;
}