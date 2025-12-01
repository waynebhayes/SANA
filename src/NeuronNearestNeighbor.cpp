#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>
#include <memory>
#include <iomanip>
#include "nanoflann.hpp"

// C-based includes
#include <unistd.h>
#include <cstring>
#include <cassert>

using IntVector = std::vector<int>;
using IntVector2D = std::vector<IntVector>;

constexpr int POINT_DEFAULT_PARENT = -1;
constexpr int POINT_DEFAULT_ID = -1;

constexpr uint64_t DISTANCE_BIN_COUNT = 1000;
constexpr uint64_t ANGLE_BIN_COUNT = 1000;
constexpr uint64_t MATRIX_THETA_SCALING_FACTOR = 1000000;

constexpr const char *INVALID_COMB_ERR_MSG = "invalid option combination: -%s and -%s\n";
constexpr const char *REQ_ARG_ERR_MSG = "option -%c requires an argument\n";
constexpr const char *INVALID_OPT_ERR_MSG = "invalid option: -%c\n";
constexpr const char *INVALID_ARG_ERR_MSG = "invalid argument: %s\n";
constexpr const char *OUT_OF_RANGE_ERR_MSG = "argument out of range: %s\n";
constexpr const char *FILEPATH_EMPTY_ERR_MSG = "%s filepath empty\n";
constexpr const char *FILE_OPENING_ERR_MSG = "cannot open file stream, file: %s";
constexpr const char *USAGE_MSG = 
"USAGE: ./nblast++ ... followed by one of the following:\n"
"    -q queryFile targetFile1 [targetFile2 ...]     # pair the query against all listed targets, produces .score files |\n"
"    -g swcFile1 [swcFile2 ...]                     # generate a p-value matrix for the swc files, prints a .matrix file to stdout |\n"
"    -n N swcFile2 [swcFile2 ...]                   # produce random pairs, ad infinitum if number of random pairs == -1, prints a .sin file to stdout |\n"
"    -s sinFile                                     # turn a sin file into a p-value matrix, produces a .matrix file |\n"
"    -r randomPairMatrixFile                        # read in the random pair matrix file\n"
"    -m matchPairMatrixFile                         # read in the match pair matrix file\n"
"    -c                                             # Calculate cosine angle measure instead of sine\n"
"    -h                                             # print usage message";

enum class option : int {
    Query,
    GenerateMatrices,
    Random,
    ComputeMatrix,
    MatrixSpecified,
    TimeSpecified,
    DefaultMode
};
std::string optToString(option m) {
    switch (m)
    {
        case option::Query: return "q";
        case option::GenerateMatrices: return "g";

        case option::Random: return "r";
        case option::ComputeMatrix: return "s";
        case option::MatrixSpecified: return "m";
        case option::TimeSpecified: return "t";
        case option::DefaultMode: return "default";
        default: return "unknown";
    }
}
void printUsage(std::ostream& out) { out << USAGE_MSG; }
void invalidCombinationError(option m1, option m2) {
    fprintf(stderr, INVALID_COMB_ERR_MSG, optToString(m1).c_str(), optToString(m2).c_str());
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void invalidArgumentError(const std::string& arg) {
    fprintf(stderr, INVALID_ARG_ERR_MSG, arg.c_str()); 
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void outOfRangeError(const std::string& arg) {
    fprintf(stderr, OUT_OF_RANGE_ERR_MSG, arg.c_str());
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void requiredArgumentError(char option) {
    fprintf(stderr, REQ_ARG_ERR_MSG, option);
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void invalidOptionError(char option) {
    fprintf(stderr, INVALID_OPT_ERR_MSG, option);
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void filepathEmptyError(const std::string& type) {
    fprintf(stderr, FILEPATH_EMPTY_ERR_MSG, type.c_str());
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void fileOpeningError(const std::string& type) {
    fprintf(stderr, FILE_OPENING_ERR_MSG, type.c_str());
    printUsage(std::cerr);
    exit(EXIT_FAILURE);
}
void stripExtension(std::string& path) {
    size_t lastDot = path.rfind('.');
    if (lastDot == std::string::npos) { return; }
    path = path.substr(0, lastDot);
}

struct Point {
    int id, parent;
    double x, y, z;

    Point(int id, double x, double y, double z, int parent) 
        : id(id), x(x), y(y), z(z), parent(parent) {}
    Point() : id(POINT_DEFAULT_ID), x(0.0), y(0.0), z(0.0), 
        parent(POINT_DEFAULT_PARENT) {}
    
    inline void parse(std::string line) {
        std::istringstream sin{line};
        std::string ignore;
        sin >> this->id 
            >> ignore 
            >> this->x >> this->y >> this->z 
            >> ignore 
            >> this->parent;
    }

    inline double magnitude() const {
        return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    }

    inline Point computeMidpoint(const Point& other) const {
        return *this + 0.5 * (other - *this);
    }

    inline double computeDistance(const Point& other) const {
        return (*this - other).magnitude();
    }

    inline double computeAngleMeasure(const Point& other, bool do_cosine) const {
        double selfMagnitude = this->magnitude();
        double otherMagnitude = other.magnitude();
        if (selfMagnitude == 0 || otherMagnitude == 0) return -1;
        double angleMeasure = std::abs(normedDotProduct(*this, other) / (selfMagnitude * otherMagnitude));
        if(angleMeasure > 1) angleMeasure = 1;
        if (do_cosine) return angleMeasure; 
        else return sin(acos(angleMeasure));
    }

    friend double normedDotProduct(const Point& lhs, const Point& rhs) {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    friend std::ostream& operator<<(std::ostream& out, const Point& p) {
        out << std::fixed << std::setprecision(4)
            << "id: '" << p.id << "' "
            << " x: '" << p.x << "' "
            << " y: '" << p.y << "' "
            << " z: '" << p.z << "' "
            << "parent: '" << p.parent << "'\n";
        return out;
    }

    friend Point operator-(const Point& lhs, const Point& rhs) {
        return Point(POINT_DEFAULT_ID, lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, POINT_DEFAULT_PARENT);
    }

    friend Point operator+(const Point& lhs, const Point& rhs) {
        return Point(POINT_DEFAULT_ID, lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, POINT_DEFAULT_PARENT);
    }

    friend Point operator*(const Point& lhs, double rhs) {
        return Point(lhs.id, lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.parent);
    }
    friend Point operator*(double lhs, const Point& rhs) {
        return Point(rhs.id, lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, rhs.parent);
    }

    Point& operator=(const Point& other) {
        if (this != &other)
        {
            this->id = other.id;
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            this->parent = other.parent;
        }
        return *this;
    }
};
using PointVector = std::vector<Point>;

struct PointMatch {
    int queryPointID, targetPointID;
    double distance, angleMeasure, score;

    PointMatch(int queryPointID, int targetPointID, 
               double distance, double angleMeasure)
        : queryPointID(queryPointID), 
        targetPointID(targetPointID), 
        distance(distance), 
        angleMeasure(angleMeasure), 
        score(0) {}
    PointMatch()
        : queryPointID(-1), 
        targetPointID(-1), 
        distance(0), 
        angleMeasure(0), 
        score(0) {}
    PointMatch(const PointMatch& other)
        : queryPointID(other.queryPointID),
        targetPointID(other.targetPointID),
        distance(other.distance),
        angleMeasure(other.angleMeasure),
        score(other.score) {}

    inline double getPValue(const IntVector2D& pMatrix) const {
        if (this->distance < 0 || this->angleMeasure < 0) {
            std::cerr << "distance or angleMeasure negative: dist=" << this->distance << " angle=" << this->angleMeasure << std::endl;
            exit(EXIT_FAILURE);
        }
        int scaledDistance = (int) (sqrt(this->distance));
        int scaledAngleMeasure = (int) (sqrt(MATRIX_THETA_SCALING_FACTOR * this->angleMeasure));
        if (scaledDistance > pMatrix.size() || scaledAngleMeasure > pMatrix[0].size()) {
            std::cerr << "scaled distance or scaled angleMeasure out of bounds: dist=" << scaledDistance << " angle=" << scaledAngleMeasure << std::endl;
            exit(EXIT_FAILURE);
        }
        return pMatrix[scaledDistance][scaledAngleMeasure] / pMatrix.back().back();
    }

    inline void computeRawScore(const IntVector2D& pMatchMatrix, const IntVector2D& pRandomMatrix) {
        double epsilon = 1e-6;
        double pMatch = this->getPValue(pMatchMatrix);
        double pRandom = this->getPValue(pRandomMatrix);
        this->score = std::log2((pMatch + epsilon) / (pRandom + epsilon));
    }

    inline void printDifference(std::ostream& out) const {
        out << this->queryPointID << " " 
            << this->targetPointID << " " 
            << this->distance << " " 
            << this->angleMeasure << "\n";
    }

    inline void printScore(std::ostream& out) const {
        out << this->queryPointID << " "
            << this->targetPointID << " "
            << this->score << "\n";
    }

    friend PointMatch operator+(const PointMatch& lhs, const PointMatch& rhs) {
        PointMatch pm = PointMatch();
        pm.score = lhs.score + rhs.score;
        return pm;
    }
};
using PMVector = std::vector<PointMatch>;

// KD-tree cloud (const)
struct MidpointCloud
{
    const PointVector& pts;

    MidpointCloud(const PointVector& points) : pts(points) {}

    // nanoflann interface: number of points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // nanoflann interface: coordinate for point index idx, dimension dim
    inline double kdtree_get_pt(size_t idx, size_t dim) const
    {
        const Point& p = pts[idx];
        if (dim == 0) return p.x;
        if (dim == 1) return p.y;
        return p.z;
    }

    // bounding-box (not used)
    template<class BBOX>
    bool kdtree_get_bbox(BBOX&) const { return false; }
};

PointVector buildMidpoints(const PointVector& pts) {
    PointVector mp;
    mp.reserve(pts.size());

    for (const auto& pt : pts) {
        if (pt.parent == -1) continue;
        Point m = pt.computeMidpoint(pts[pt.parent]);

        // midpoint: id = original id, parent = -1
        mp.emplace_back(pt.id, m.x, m.y, m.z, -1);
    }

    return mp;
}

unsigned computePointCount(std::ifstream& fin) {
    fin.seekg(0, std::ios::beg);
    const size_t bufferSize = 1 << 20;
    std::vector<char> buffer(bufferSize);
    unsigned lineCount = 0;
    while (fin) {
        fin.read(buffer.data(), buffer.size());
        std::streamsize bytes_read = fin.gcount();
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') ++lineCount;
        }
    }
    fin.clear();
    fin.seekg(0, std::ios::beg);
    return lineCount + 1; // in case no comments, no newline at end.
}

int loadPoints(const std::string& filepath, PointVector& vec) {
    std::ifstream fin{filepath};
    if (!fin) { fileOpeningError(filepath); }
    
    int pointCount = computePointCount(fin);
    vec.reserve(pointCount);

    std::string line;
    unsigned id = -1;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line[0] == '#') continue;
        std::istringstream sin(line);
        sin >> id;
        Point p;
        p.parse(line);
        vec[id] = p;
    }
    vec.shrink_to_fit();
    fin.close();
    return 0;
}

PMVector nearestNeighborKDTree(const PointVector& query, const PointVector& target, bool doCosine, bool doPrint) {
    // Build midpoints for query / target
    PointVector queryMidpoints  = buildMidpoints(query);
    PointVector targetMidpoints = buildMidpoints(target);

    // Build point cloud for KD-tree
    MidpointCloud cloud(targetMidpoints);

    using KDTree = nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<double, MidpointCloud>,
        MidpointCloud,
        3
    >;

    KDTree index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    index.buildIndex();

    PMVector matchVector;
    // For each query midpoint, perform nearest neighbor search
    for (const auto& qmp : queryMidpoints) {
        double query_pt[3] = { qmp.x, qmp.y, qmp.z };

        size_t nearestIdx = 0;
        double outDistanceSqr = 0;

        nanoflann::KNNResultSet<double> resultSet(1);
        resultSet.init(&nearestIdx, &outDistanceSqr);
        index.findNeighbors(resultSet, query_pt, 0);

        const Point& tmp = targetMidpoints[nearestIdx];

        // original query-side segment r_i
        const Point& qi = query.at(qmp.id);
        if (qi.parent == -1) continue;
        const Point& qj = query.at(qi.parent);
        Point r_i = qj - qi;

        // original target-side segment s_i
        const Point& ti = target.at(tmp.id);
        if (ti.parent == -1) continue;
        const Point& tj = target.at(ti.parent);
        Point s_i = tj - ti;

        // angle measure
        double angleMeasure = r_i.computeAngleMeasure(s_i, doCosine);

        // output: id_i id_j distance angle
        PointMatch pc{ qi.id, ti.id, std::sqrt(outDistanceSqr), angleMeasure };
        matchVector[qi.id] = pc;
        if (doPrint) {
            pc.printDifference(std::cout);
        }
    }
    return matchVector;
}

PMVector nearestNeighborNaive(const PointVector& query, const PointVector& target, bool doCosine, bool doPrint) {
    PMVector matchVector;
    for (const auto& query_i : query) {
        if (query_i.parent == POINT_DEFAULT_PARENT) continue;
        
        const Point& query_j = query[query_i.parent];
        Point r_i = query_j - query_i;
        Point m_i = query_i + 0.5 * r_i;
        
        Point target_min;
        double distance_min = std::numeric_limits<double>::max();
        double angle_diff = 0;
        for (const auto& target_i : target) {
            if (target_i.parent == -1) continue;

            const Point& target_j = target[target_i.parent];
            Point s_i = target_j - target_i;
            Point o_i = target_i + 0.5 * s_i;

            double distance_i = m_i.computeDistance(o_i);
            
            if (distance_i < distance_min) {
                target_min = target_i;
                distance_min = distance_i;

                double angle_diff_tmp = r_i.computeAngleMeasure(s_i, doCosine);
                if (angle_diff_tmp == -1) continue;
                angle_diff = angle_diff_tmp;
            }
        }
        PointMatch pm{ query_i.id, target_min.id, distance_min, angle_diff };
        matchVector[pm.queryPointID] = pm;
        if (doPrint) {
            pm.printDifference(std::cout);
        }
    }
    return matchVector;
}

int countsToPValueMatrix(IntVector2D& matrix) {
    for (size_t i = 0; i < matrix.size(); ++i) {
        int tmp = 0, row_counter = 0;
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            tmp = matrix[i][j];
            matrix[i][j] += row_counter;
            row_counter += tmp;
            if (i > 0) {
                matrix[i][j] += matrix[i - 1][j];
            }
        }
    }
    return 0;
}

int pMatrixFromFile(const std::string& filepath, IntVector2D& matrix) {
    std::ifstream fin{filepath, std::ios::in};
    if (!fin) { fileOpeningError(filepath); }
    std::string line, ignore;
    std::istringstream sin;
    double distance, theta;
    int scaled_dist, scaled_ang;
    std::getline(fin, ignore); // first line is the two filenames
    while (std::getline(fin, line)) {
        sin.clear();
        sin.str(line);
        sin >> ignore >> ignore;
        if (sin.fail()) continue;
        sin >> distance >> theta;
        scaled_dist = (int) (sqrt(distance));
        scaled_ang = (int) (sqrt(MATRIX_THETA_SCALING_FACTOR * theta));
        ++matrix[scaled_dist][scaled_ang];
        sin.str("");
    }
    countsToPValueMatrix(matrix);
    return 0;
}

int pMatrixFromVector(PMVector pointMatchVector, IntVector2D& matrix) {
    double scaledDistance = 0, scaledAngleMeasure = 0;
    for (size_t i = 0; i < pointMatchVector.size(); ++i) {
        scaledDistance = (int) (sqrt(pointMatchVector[i].distance));
        scaledAngleMeasure = (int) (sqrt(MATRIX_THETA_SCALING_FACTOR * pointMatchVector[i].angleMeasure));
        ++matrix[scaledDistance][scaledAngleMeasure];
    }
    countsToPValueMatrix(matrix);
    return 0;
}

// @todo actually print out some useful info about the matrix.
void printMatrixHeader(size_t matrix_i_size, size_t matrix_j_size) {
    printf("# %lu x %lu P-Value Matrix!\n", matrix_i_size, matrix_j_size);
}

void printMatrix(const IntVector2D& matrix) {
    printMatrixHeader(matrix.size(), matrix[0].size());
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            std::cout << matrix[i][j];
            if (j < matrix[i].size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    }
}

int readMatrix(const std::string& filepath, IntVector2D& matrix) {
    std::ifstream fin{filepath, std::ios::in};
    if (!fin) {
        std::cerr << "Error: Cannot open stream with filepath: '" 
            << filepath << "'." << std::endl;
        return -1;
    }
    std::string str;
    size_t i, j;
    while (fin >> str) {
        if (str == "#") {
            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            break;
        }
    }
    matrix[0][0] = std::stoi(str);
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            fin >> matrix[i][j];
        }
    }
    return 0;
}

void computeRawScores(const IntVector2D& pMatchMatrix, const IntVector2D& pRandomMatrix, 
                          PMVector& matchVector) {
    for (auto& pm : matchVector) {
        pm.computeRawScore(pMatchMatrix, pRandomMatrix);
    }
}

double sumRawScores(PMVector vec) {
    double res = 0;
    for (const auto& elem : vec) {
        res += elem.score;
    }
    return res;
}

double scoreNeuronPair(const IntVector2D& pMatchMatrix, const IntVector2D& pRandomMatrix, 
                       const PointVector& queryVector, const PointVector& targetVector, bool doCosine) {
    // compute forward score
    PMVector forwardMatchVector = nearestNeighborKDTree(queryVector, targetVector, doCosine, false);
    computeRawScores(pMatchMatrix, pRandomMatrix, forwardMatchVector);
    double forwardTotalScore = sumRawScores(forwardMatchVector);

    // compute forward self score
    PMVector forwardSelfMatchVector = nearestNeighborKDTree(queryVector, queryVector, doCosine, false);
    computeRawScores(pMatchMatrix, pRandomMatrix, forwardSelfMatchVector);
    double forwardSelfTotalScore = sumRawScores(forwardSelfMatchVector);

    // compute reverse score
    PMVector reverseMatchVector = nearestNeighborKDTree(targetVector, queryVector, doCosine, false);
    computeRawScores(pMatchMatrix, pRandomMatrix, reverseMatchVector);
    double reverseTotalScore = sumRawScores(reverseMatchVector);

    // compute reverse self score
    PMVector reverseSelfMatchVector = nearestNeighborKDTree(targetVector, targetVector, doCosine, false);
    computeRawScores(pMatchMatrix, pRandomMatrix, reverseSelfMatchVector);
    double reverseSelfTotalScore = sumRawScores(reverseSelfMatchVector);
    
    // normalize forward and reverse by self
    // then average for final score
    return ((forwardTotalScore / forwardSelfTotalScore) + (reverseTotalScore / reverseSelfTotalScore)) / 2;
}

int main(int argc, char *argv[]) {
    int opt = 0, rc = 0;
    std::string targetFilepath, queryFilepath;
    std::string matchPairsFilepath, randomPairsFilepath;
    std::string matchMatrixFilepath, randomMatrixFilepath;
    option op = option::DefaultMode;
    uint64_t numRandomPairs = 0, generatorTime = 0;
    bool doCosine = false, doInfinite = false;

    while ((opt = getopt(argc, argv, ":q:gk:r:s:m:n:ch")) != -1) {
        switch (opt) {
            // ===== main modes =====
            // query toolchain, compares one query .swc file to one or more target .swc files
            case 'q': {
                if (op != option::DefaultMode) {
                    invalidCombinationError(op, option::Query);
                }
                op = option::Query;
                queryFilepath = optarg;
                break;
            }
            // generator toolchain, generates match and random p-value matrices
            // given .swc files and known matches file
            case 'g': {
                if (op != option::DefaultMode) { 
                    invalidCombinationError(op, option::GenerateMatrices);
                } else if (!matchMatrixFilepath.empty() || !randomMatrixFilepath.empty()) {
                    invalidCombinationError(option::GenerateMatrices, option::MatrixSpecified);
                }
                op = option::GenerateMatrices;
                queryFilepath = optarg;
                break;
            }
            // Part 2 of the generator toolchain, queries random pairs and prints results to stdout
            case 'n': {
                if (op != option::DefaultMode) { 
                    invalidCombinationError(op, option::Random);
                } else if (!matchMatrixFilepath.empty() || !randomMatrixFilepath.empty()) {
                    invalidCombinationError(option::Random, option::MatrixSpecified);
                }
                op = option::Random;
                try {
                    numRandomPairs = std::stoi(optarg);
                    if (numRandomPairs < 0) throw std::out_of_range("numRandomPairs");
                } catch (const std::invalid_argument& e) {
                    invalidArgumentError(optarg);
                } catch (const std::out_of_range& e) {
                    outOfRangeError(optarg);
                }
                if (numRandomPairs == 0) {
                    doInfinite = true;
                }
                break;
            }
            // Part 3 of the generator toolchain, accepts a .sin file and prints a matrix to stdout
            case 's': {
                if (op != option::DefaultMode) { 
                    invalidCombinationError(op, option::ComputeMatrix);
                } else if (!matchMatrixFilepath.empty() || !randomMatrixFilepath.empty()) {
                    invalidCombinationError(option::ComputeMatrix, option::MatrixSpecified);
                }
                op = option::ComputeMatrix;
                randomPairsFilepath = optarg;
                break;
            }
            // ===== options =====
            // specify the time to run the generator / random pairs for (in hours)
            case 't': {
                if (op != option::GenerateMatrices && op != option::Random) {
                    invalidCombinationError(op, option::TimeSpecified);
                }
                try {
                    generatorTime = std::stoi(optarg);
                    if (generatorTime < 0) throw std::out_of_range("generatorTime");
                } catch (const std::invalid_argument& e) {
                    invalidArgumentError(optarg);
                } catch (const std::out_of_range& e) {
                    outOfRangeError(optarg);
                }
                break;
            }
            // specify the known matches file
            case 'k': {
                matchPairsFilepath = optarg;
                break;                
            }
            // specify the matched pairs matrix
            case 'm': {
                matchMatrixFilepath = optarg;
                break;
            }
            // specify the random pairs matrix
            case 'r': {
                randomMatrixFilepath = optarg;
                break;
            }
            // use cosine instead of sine
            case 'c': {
                doCosine = true;
                break;
            }
            // print usage
            case 'h': {
                printUsage(std::cout);
                exit(EXIT_SUCCESS);
            }
            case ':': {
                requiredArgumentError(static_cast<char>(optopt));
            }
            case '?': {
                invalidOptionError(static_cast<char>(optopt));
            }
            default: {
                invalidOptionError(static_cast<char>(optopt));
            }
        }
    }
    switch (op) {
        // default, no args, print usage to stdout
        case option::DefaultMode: {
            printUsage(std::cout);
            exit(EXIT_SUCCESS);
        }
        // assuming p_rand matrix already generated,
        // outputs query comparison score between query and targets to stdout
        case option::Query: {
            if (matchMatrixFilepath.empty()) { 
                filepathEmptyError("matchMatrix");
            } else if (randomMatrixFilepath.empty()) { 
                filepathEmptyError("randomMatrix");
            } else if (queryFilepath.empty()) { 
                filepathEmptyError("query");
            }
            
            IntVector2D matchMatrix(DISTANCE_BIN_COUNT, IntVector(ANGLE_BIN_COUNT, 1));
            rc = readMatrix(matchMatrixFilepath, matchMatrix);
            assert(rc == 0);
            
            IntVector2D randomMatrix(DISTANCE_BIN_COUNT, IntVector(ANGLE_BIN_COUNT, 1));
            rc = readMatrix(randomMatrixFilepath, randomMatrix);
            assert(rc == 0);

            PointVector queryVector;
            rc = loadPoints(queryFilepath, queryVector);
            assert(rc == 0);
            stripExtension(queryFilepath);

            for(int i = optind; i < argc; i++) {
                std::string targetFilepath = argv[i];
                if (targetFilepath.empty()) { std::cerr << "can't open " << targetFilepath << "; continuing\n"; continue; }
                PointVector targetVector;
                rc = loadPoints(targetFilepath, targetVector);
                if (rc) { std::cerr << "failed to load points from " << targetFilepath << "; continuing\n"; continue; }
                stripExtension(targetFilepath);
                
                std::cerr << "scoring " << queryFilepath << " " << targetFilepath << "\n";
                double score = scoreNeuronPair(matchMatrix, randomMatrix, queryVector, targetVector, doCosine);
                std::cout << queryFilepath << " " << targetFilepath << " " << score << "\n";                
            }
            std::cout.flush();
            return 0;
        }
        // generate a matrix for a given dataset, print it to stdout
        case option::GenerateMatrices: {
            // @todo finish mode
        }
        // first step in generating the matrix
        // randomly selects pairs of .swc files,
        // and outputs nearest neighbor calculations to stdout
        case option::Random: {
            uint64_t n = argc - optind; // number of input SWC files, from which random pairs will be chosen
            srand48(time(0) + getpid()); // seed the random number generator
            while(doInfinite || numRandomPairs > 0) {
                uint64_t i = optind + n * drand48(), j = optind + n * drand48();

                queryFilepath = argv[i];
                PointVector queryVector;
                rc = loadPoints(queryFilepath, queryVector);
                if (rc == -1) continue;

                targetFilepath = argv[j];
                PointVector targetVector;
                rc = loadPoints(targetFilepath, targetVector);
                if (rc == -1) continue;

                stripExtension(targetFilepath);
                stripExtension(queryFilepath);
                std::cout << queryFilepath << " " << targetFilepath << "\n";
                PMVector matchVector = nearestNeighborKDTree(queryVector, targetVector, doCosine, true);
                if (!doInfinite) --numRandomPairs;
            }
            return 0;
        }
        // second step in generating the matrix
        // given a file with nearest neighbor calculations,
        // outputs the associated p-value matrix to stdout
        case option::ComputeMatrix: {
            if (randomMatrixFilepath.empty()) { 
                filepathEmptyError("sin");
            }
            
            IntVector2D matrix(DISTANCE_BIN_COUNT, IntVector(ANGLE_BIN_COUNT, 1));
            // use one of the specified matrix arguments
            std::string matrixFilepath = matchMatrixFilepath.empty() ? randomMatrixFilepath.empty() ? "" : randomMatrixFilepath : matchMatrixFilepath;
            rc = pMatrixFromFile(matrixFilepath, matrix);
            printMatrix(matrix);
            return 0;
        }
        // in case something went wrong in argument parsing
        default: { invalidArgumentError(optToString(op)); }
    }
}
