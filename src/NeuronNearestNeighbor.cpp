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

#include <unistd.h>

#define USAGE_MSG "NeuronNearestNeighbor [-o OUTFILE] -t TARGET_PATH | -q QUERY_PATH\n"\
                  "\n  -t TARGET_PATH   Path to the source SWC."\
                  "\n  -q QUERY_PATH    Path to the query SWC."\
                  "\n  -h               Prints the usage statement to STDOUT. All other arguments are ignored.\n"

constexpr int MIN_LINE_LEN = 34;
constexpr int POINT_DEFAULT_PARENT = -1;
constexpr int POINT_DEFAULT_ID = -1;

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
    return file_byte_len / MIN_LINE_LEN;
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

void nearest_neighbor(const std::vector<point>& p, const std::vector<point>& q)
{
    for (const auto& p_i : p)
    {
        if (p_i.parent == POINT_DEFAULT_PARENT)
        {
            continue;
        }
        const point& p_j = p[p_i.parent];
        point r_i = p_j - p_i;
        point m_i = p_i + 0.5 * r_i;
        point q_min;
        double d_min = std::numeric_limits<double>::max();
        double norm_dot_prod_min = 0;
        for (const auto& q_i : q)
        {
            if (q_i.parent == -1)
            {
                continue;
            }
            const point& q_j = q[q_i.parent];
            point s_i = q_j - q_i;
            point o_i = q_i + 0.5 * s_i;

            double d_i = (m_i - o_i).magnitude();
            
            if (d_i < d_min)
            {
                q_min = q_i;
                d_min = d_i;
                double r_magnitude = r_i.magnitude();
                double s_magnitude = s_i.magnitude();
                if (r_magnitude == 0 || s_magnitude == 0)
                {
                    continue;
                }
                norm_dot_prod_min = std::abs(dot(r_i, s_i)) / (r_magnitude * s_magnitude);
            }
        }
        std::cout << p_i.id << " " << q_min.id << " " << d_min << " " << norm_dot_prod_min << "\n";
    }
    std::cout.flush();
}

int main(int argc, char *argv[])
{
    int opt, rc;
    std::string target_filepath, query_filepath;

    while ((opt = getopt(argc, argv, ":t:q:h")) != -1)
    {
        switch (opt)
        {
            case 't':
            {
                target_filepath = optarg;
                break;
            }
            case 'q':
            {
                query_filepath = optarg;
                break;
            }
            case 'h':
            {
                std::cout << USAGE_MSG;
                return EXIT_SUCCESS;
            }
            default:
            {
                std::cerr << USAGE_MSG;
                return EXIT_FAILURE;
            }
        }
    }

    if (target_filepath.empty() || query_filepath.empty()) 
    {
        std::cerr << "Error: both -t and -q must be provided.\n";
        return EXIT_FAILURE;
    }

    std::vector<point> target_v;
    rc = load_points(target_filepath, target_v);
    if (rc)
    {
        return rc;
    }

    std::vector<point> query_v;
    rc = load_points(query_filepath, query_v);
    if (rc)
    {
        return rc;
    }

    nearest_neighbor(target_v, query_v);
}