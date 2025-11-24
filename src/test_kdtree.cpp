#define TEST_KDTREE
#include "NeuronNearestNeighbor.cpp"
#include "nanoflann.hpp"
#include <iostream>
#include <vector>

int main()
{
    bool do_cosine = false;

    std::vector<point> query = {
        point(0, 0, 0, 0, -1), // root
        point(1, 1, 0, 0, 0),
        point(2, 2, 0, 0, 1),
        point(3, 1, 1, 1, 1),
        point(4, 2, 2, 0, 3)
    };

    std::vector<point> target = {
        point(0, 0, 1, 0, -1), // root
        point(1, 0, 2, 0, 0),
        point(2, 0, 3, 0, 1),
        point(3, 1, 1, 0, 2),
        point(4, 3, 3, 0, 2)
    };

    // brute-force
    std::cout << "Brute-force results:\n";
    auto t1 = std::chrono::high_resolution_clock::now();
    nearest_neighbor(query, target, do_cosine);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Brute-force time: "
              << std::chrono::duration<double, std::milli>(t2 - t1).count()
              << " ms\n\n";

    // KDTree
    std::cout << "KDTree results:\n";
    t1 = std::chrono::high_resolution_clock::now();
    nearest_neighbor_kdtree(query, target, do_cosine);
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "KDTree time: "
              << std::chrono::duration<double, std::milli>(t2 - t1).count()
              << " ms\n";

    return 0;
}
