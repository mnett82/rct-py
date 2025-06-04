#include <memory>
#include <vector>
#include <random>
#include <functional>
#include <cstdio>

#include "DistData.h"
#include "rct.h"

struct Vec final : public DistData
{
    std::vector<float> values;

    Vec(
        std::function<float()>&& gen,
        std::size_t len
    ) {
        values.reserve(len);
        for (int i = 0; i < len; ++i) {
            values.push_back(gen());
        }
    }

    float distanceTo(DistData *other) override
    {
        const Vec *p = static_cast<Vec *>(other);

        float d = 0.0f;
        for (int i = 0; i < values.size(); ++i)
        {
            const float s = values[i] - p->values[i];
            d += s * s;
        }
        return sqrtf(d);
    }
};


 // 1. Random number engine setup
    // // Use std::random_device to seed the Mersenne Twister engine
    // std::random_device rd;
    // std::mt19937 gen(rd()); // Seed the generator

    // // 2. Define a distribution for floats
    // // Example: Generate floats between 0.0 and 1.0 (inclusive)
    // std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // // If you want a different range, e.g., between -1.0 and 1.0:
    // // std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    // // If you want the full range of float values (less common for random data):
    // // std::uniform_real_distribution<float> dis(
    // //     std::numeric_limits<float>::min(),
    // //     std::numeric_limits<float>::max()
    // // );

    // // 3. Create a vector to store the floats
    // std::vector<float> randomFloats;
    // randomFloats.reserve(784); // Optional: Pre-allocate memory for efficiency

    // // 4. Generate 784 random floats
    // for (int i = 0; i < 784; ++i) {
    //     randomFloats.push_back(dis(gen));
    // }

    // // Optional: Print the first few and last few generated floats to verify
    // std::cout << "Generated " << randomFloats.size() << " random floats.\n";
    // std::cout << "First 5 floats:\n";
    // for (int i = 0; i < 5; ++i) {
    //     std::cout << randomFloats[i] << std::endl;
    // }
    // std::cout << "...\n";
    // std::cout << "Last 5 floats:\n";
    // for (int i = randomFloats.size() - 5; i < randomFloats.size(); ++i) {
    //     std::cout << randomFloats[i] << std::endl;
    // }

    // return 0;

int main() {
    // Generate 60,000 784-dim points (like MNIST)
    printf("Generating points...\n");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    auto gen_point = [&]() {
        return Vec([&]() { return dis(gen); }, 784);
    };

    std::vector<Vec> points;
    points.reserve(60000);
    for (int i = 0; i < 60000; ++i) {
        points.push_back(gen_point());
    }
    std::vector<DistData*> point_ptrs;
    point_ptrs.reserve(points.size());
    for (Vec& point : points) {
        point_ptrs.push_back(&point);
    }

    // Build index.
    auto rct = std::make_unique<RCT>();
    rct->setVerbosity(2);
    rct->setCoverageParameter(8.0);
    rct->setSampleRate(pow(60000.0, 1.0 / 3.0));
    rct->build(point_ptrs.data(), point_ptrs.size());
    
    rct->setCoverageParameter(1000000.0);

    // Try single query.
    auto query = gen_point();
    int num_found = rct->findNearest(&query, 10);
    std::vector<int> indices(num_found);
    rct->getResultIndices(indices.data(), num_found);
    printf("%d found:", num_found);
    for (int i = 0; i < num_found; ++i) {
        printf(" %d", indices[i]);
    }
    printf("\n");
    num_found = rct->findNear(&query, 10);
    indices.resize(num_found);
    rct->getResultIndices(indices.data(), num_found);
    printf("%d found:", num_found);
    for (int i = 0; i < num_found; ++i) {
        printf(" %d", indices[i]);
    }
    printf("\n");

    return 0;
}