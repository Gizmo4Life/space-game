#pragma once
#include <random>

namespace space {

class Random {
public:
    static int getInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(getEngine());
    }

    static float getFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(getEngine());
    }

private:
    static std::mt19937& getEngine() {
        thread_local std::mt19937 engine(generateSeed());
        return engine;
    }

    static unsigned int generateSeed() {
        std::random_device rd;
        return rd();
    }
};

} // namespace space
