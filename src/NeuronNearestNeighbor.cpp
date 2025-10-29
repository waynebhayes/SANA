#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <limits>
#include <memory>
#include <iomanip>


// C-based includes
#include <unistd.h>
#include <cstring>
#include <cassert>

constexpr int LINE_MIN_LEN = 34;
constexpr int POINT_DEFAULT_PARENT = -1;
constexpr int POINT_DEFAULT_ID = -1;

constexpr int MATRIX_DISTANCE_BIN_COUNT = 1000;
constexpr int MATRIX_THETA_BIN_COUNT = 1000;
constexpr int MATRIX_THETA_SCALING_FACTOR = 1000000;

struct point
{
    int id, parent;
    double x, y, z;

    point(int id, double x, double y, double z, int parent) 
        : id(id), x(x), y(y), z(z), parent(parent) {}
    point() : id(POINT_DEFAULT_ID), x(0.0), y(0.0), z(0.0), 
        parent(POINT_DEFAULT_PARENT) {}
    
    inline void parse(std::string line)
    {
        std::istringstream sin{line};
        std::string ignore;
        sin >> this->id 
            >> ignore 
            >> this->x >> this->y >> this->z 
            >> ignore 
            >> this->parent;
    }

    inline double magnitude() const
    {
        return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    }

    friend std::ostream& operator<<(std::ostream& out, const point& p)
    {
        out << std::fixed << std::setprecision(4)
            << "id: '" << p.id
            << "' x: '" << p.x 
            << "' y: '" << p.y 
            << "' z: '" << p.z 
            << "' parent: '" << p.parent << "'";
        return out;
    }

    friend double dot(const point& lhs, const point& rhs)
    {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
    }

    point& operator=(const point& other)
    {
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

    friend point operator-(const point& lhs, const point& rhs)
    {
        return point(POINT_DEFAULT_ID, lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, POINT_DEFAULT_PARENT);
    }

    friend point operator+(const point& lhs, const point& rhs)
    {
        return point(POINT_DEFAULT_ID, lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, POINT_DEFAULT_PARENT);
    }

    friend point operator*(const point& lhs, double rhs)
    {
        return point(lhs.id, lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.parent);
    }
    friend point operator*(double lhs, const point& rhs)
    {
        return point(rhs.id, lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, rhs.parent);
    }
};

// overestimates the point count slightly, 
// faster than iterating through and counting the lines.
unsigned estimate_point_count(std::istream& in)
{
    in.seekg(0, in.end);
    uint64_t file_byte_len = in.tellg();
    in.seekg(0, in.beg);
    return file_byte_len / LINE_MIN_LEN;
}

int load_points(const std::string& filepath, std::vector<point>& vec)
{
    std::string line;
    unsigned i = 0, id = -1;
    
    std::ifstream fin{filepath, std::ios::in};
    if (!fin)
    {
        std::cerr << "Error: Cannot open stream with filepath: '" 
            << filepath << "'." << std::endl;
        return -1;
    }
    
    unsigned point_count = estimate_point_count(fin);
    // point count + 1 for 1-indexed files
    vec.resize(point_count + 1);
    
    std::istringstream sin;
    while (std::getline(fin, line))
    {
        if (!line.empty() && line[0] == '#')
        {
            continue;
        }
        sin.str(line);
        sin >> id;
        vec[id].parse(line);
        ++i;
    }
    vec.resize(i + 1);
    vec.shrink_to_fit();
    fin.close();
    return 0;
}

// NATHAN: please name the first argument query (or q) and the second one target (or t)
// done :) - nathan
void nearest_neighbor(const std::vector<point>& query, const std::vector<point>& target)
{
    for (const auto& query_i : query)
    {
        if (query_i.parent == POINT_DEFAULT_PARENT) continue;
        const point& query_j = query[query_i.parent];
        point r_i = query_j - query_i;
        point m_i = query_i + 0.5 * r_i;
        point target_min;
        double distance_min = std::numeric_limits<double>::max();
        double norm_dot_prod_min = 0;
        for (const auto& target_i : target)
        {
            if (target_i.parent == -1) continue;
            const point& target_j = target[target_i.parent];
            point s_i = target_j - target_i;
            point o_i = target_i + 0.5 * s_i;

            double distance_i = (m_i - o_i).magnitude();
            
            if (distance_i < distance_min)
            {
                target_min = target_i;
                distance_min = distance_i;
                double r_magnitude = r_i.magnitude();
                double s_magnitude = s_i.magnitude();
                if (r_magnitude == 0 || s_magnitude == 0) continue;
                norm_dot_prod_min = std::abs(dot(r_i, s_i) / (r_magnitude * s_magnitude));
                if(norm_dot_prod_min>1) norm_dot_prod_min=1;
            }
        }
        std::cout << query_i.id << " " << target_min.id << " " << distance_min << " " << sin(acos(norm_dot_prod_min)) << "\n";
    }
    std::cout.flush();
}

int sin_to_counts_matrix(const std::string& filepath, std::vector<std::vector<int>>& matrix)
{
    std::ifstream fin{filepath, std::ios::in};
    if (!fin) {
        std::cerr << "Error: Cannot open stream with filepath: '" 
            << filepath << "'." << std::endl;
        return -1;
    }
    std::string line, ignore;
    std::istringstream sin;
    double distance, theta;
    int scaled_dist, scaled_ang;
    while (std::getline(fin, line))
    {
        sin.str(line);
        sin >> ignore >> ignore >> distance >> theta;
        scaled_dist = (int) (sqrt(distance));
        scaled_ang = (int) (sqrt(MATRIX_THETA_SCALING_FACTOR * theta));
        ++matrix[scaled_dist][scaled_ang];
        sin.str("");
        sin.clear();
    }
    return 0;
}

int counts_to_pmatrix(std::vector<std::vector<int>>& matrix)
{
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

// @TODO actually print out some useful info about the matrix.
void print_header(size_t matrix_i_size, size_t matrix_j_size)
{
    printf("# %lux%lu Matrix!\n", matrix_i_size, matrix_j_size);
}

void print_matrix(const std::vector<std::vector<int>>& matrix)
{
    print_header(matrix.size(), matrix[0].size());
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

// assumes size of the matrix is given.
int read_matrix(const std::string& filepath, std::vector<std::vector<int>>& matrix)
{
    std::ifstream fin{filepath, std::ios::in};
    if (!fin)
    {
        std::cerr << "Error: Cannot open stream with filepath: '" 
            << filepath << "'." << std::endl;
        return -1;
    }
    std::string ch;
    size_t i, j;
    while (std::cin >> ch) {
        if (ch == "#") {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else {
            break;
        }
    }
    matrix[0][0] = std::stoi(ch);
    for (size_t i = 1; i < matrix.size(); ++i) {
        for (size_t j = 0; i < matrix[i].size(); ++j) {
            std::cin >> matrix[i][j];
        }
    }
    return 0;
}

void str_strip_extension(std::string& path)
{
    size_t last_dot = path.rfind('.');
    if (last_dot == std::string::npos) {
        std::cerr << "Incorrect File Type" << std::endl;
        exit(EXIT_FAILURE);
    }
    path = path.substr(0, last_dot);
}

enum class mode : int
{
    listed,
    random_inf,
    sin_to_matrix,
    read_matrix,
    count
};

#define USAGE_MSG "USAGE: ./sinblast ... followed by one of the following:\n"\
"    -l query.swc [ list of target.swc's ] # pair the query against all listed targets, produces .sin files|\n"\
"    -r [list of swc files] # produce random pairs, ad infinitum, produces .sin files |\n"\
"    -s [sin file] # turn a sin file into a p-value matrix, produces a .matrix file |\n"\
"    -m [matrix file] # read a p-value matrix file";

void usage_and_exit(void)
{
    std::cerr << USAGE_MSG;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt, rc;
    std::string target_filepath, query_filepath, sin_filepath, matrix_filepath;
    mode md = mode::count;
    int argn = 1;

    // NATHAN: feel free to put the optarg() stuff back again... we may be adding more options in the future...
    // done :) - nathan
    while ((opt = getopt(argc, argv, ":l:rs:m:")) != -1) {
        switch (opt) {
            case 'l': {
                if (md != mode::count) { usage_and_exit(); }
                md = mode::listed;
                break;
            }
            case 'r': {
                if (md != mode::count) { usage_and_exit(); }
                md = mode::random_inf;
                break;
            }
            case 's': {
                if (md != mode::count) { usage_and_exit(); }
                md = mode::sin_to_matrix;
                sin_filepath = optarg;
                break;
            }
            case 'm': {
                if (md != mode::count) { usage_and_exit(); }
                md = mode::read_matrix;
                matrix_filepath = optarg;
                break;
            }
            default: {
                usage_and_exit();
            }
        }
    }
    switch (md)
    {
        case mode::listed: {
            if (query_filepath.empty()) { std::cerr << USAGE_MSG; return EXIT_FAILURE;}
            std::vector<point> query_v;
            rc = load_points(query_filepath, query_v);
            if (rc) return rc;

            for(int i=2;i<argc;i++) {
                target_filepath = argv[i];
                if (target_filepath.empty()) {std::cerr << "can't open " << target_filepath << "; continuing\n"; continue;}
                std::vector<point> target_v;
                rc = load_points(target_filepath, target_v);
                if (rc) {std::cerr << "failed to load points from " << target_filepath << "; continuing\n"; continue;}
                str_strip_extension(target_filepath);
                str_strip_extension(query_filepath);
                std::cout << query_filepath << " " << target_filepath << "\n";
                nearest_neighbor(query_v, target_v);
            }
        }
        case mode::random_inf: {
            ++argn;
            int n = argc - argn; // number of input SWC files, from which random pairs will be chosen
            srand48(time(0)+getpid()); // seed the random number generator
            while(1) {
                int i = argn+n*drand48(), j = argn+n*drand48();

                query_filepath = argv[i];
                std::vector<point> query_v;
                rc = load_points(query_filepath, query_v);
                assert(rc==0);

                target_filepath = argv[j];
                std::vector<point> target_v;
                rc = load_points(target_filepath, target_v);
                assert(rc==0);

                // NATHAN: it would be good to strip off the ".swc" from the end of the names before printing them
                // done :) - nathan
                str_strip_extension(target_filepath);
                str_strip_extension(query_filepath);
                std::cout << query_filepath << " " << target_filepath << "\n";
                nearest_neighbor(query_v, target_v);
            }
        }
        case mode::sin_to_matrix: {
            if (sin_filepath.empty()) { usage_and_exit(); }
            
            std::vector<std::vector<int>> matrix(MATRIX_DISTANCE_BIN_COUNT, 
                std::vector<int>(MATRIX_THETA_BIN_COUNT, 0));
            rc = sin_to_counts_matrix(sin_filepath, matrix);
            assert(rc == 0);
            rc = counts_to_pmatrix(matrix);
            assert(rc == 0);
            print_matrix(matrix);
        }
        case mode::read_matrix: {
            // @TODO NOT IMPLEMENTED
        }
    }
}
