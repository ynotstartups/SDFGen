/*
 *  Represents an optionally-indexed vertex in space
 */
inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

struct VertexSTL
{
    VertexSTL() {}
    VertexSTL(float x, float y, float z) : x(x), y(y), z(z) {}

    float x, y, z;
    unsigned int i;

    bool operator!=(const VertexSTL& rhs) const
    {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }
    bool operator<(const VertexSTL& rhs) const
    {
        if      (x != rhs.x)    return x < rhs.x;
        else if (y != rhs.y)    return y < rhs.y;
        else if (z != rhs.z)    return z < rhs.z;
        else                    return false;
    }
};

inline VertexSTL get_vector(std::string& str)
{
    float x, y, z;
    if(sscanf(str.c_str(),"vertex %f %f %f",
        &x,	&y,	&z) != 3) {
        printf("weird format ascii stl exiting\n");
        exit(1);
    }
    VertexSTL v(x, y, z);
    return v;
}

std::vector<VertexSTL> load_binary(const char* filename) {
    printf("loading binary stl\n");
    std::fstream fbin;
    fbin.open(filename, std::ios::in | std::ios::binary);
    fbin.seekg(80);

    int num_faces;
    fbin.read(reinterpret_cast<char *>(&num_faces), 4);

    const unsigned int num_indices = num_faces*3;

    size_t len = num_faces*50;
    char *ret = new char[len];
    fbin.read(ret, len);
    std::vector<VertexSTL> all_vertices(num_indices);

    for (int i=0;i<num_faces;i+=1) {
        for (int j=0;j<3;j++) {
            const int index = i*3+j;
            std::memcpy(&all_vertices[index], &ret[12 + i*50 + j*12], 12);
        }
    }

    fbin.close();

    return all_vertices;
}

std::vector<VertexSTL> load_ascii(const char* filename) {
    printf("loading ascii\n");

    std::vector<VertexSTL> all_vertices;

    std::ifstream file;
    file.open(filename);

    std::string line;
    while (!file.eof()) {
        std::getline(file, line);
        line = trim(line);
        if (line.rfind("vertex", 0) == 0) {
            all_vertices.push_back(get_vector(line));
        }
    }
    file.close();

    return all_vertices;
}

// thanks to https://github.com/mkeeter/fstl/blob/master/src/loader.cpp
std::vector<VertexSTL> load_stl_vertices(const char* filename) {
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    if (line.rfind("solid ", 0) == 0) {
        std::getline(file, line);
        line = trim(line);
        if (line.rfind("facet", 0) == 0)
        {
            file.close();
            return load_ascii(filename);
        }
    }
    file.close();
    return load_binary(filename);
}


void load_stl(const char* filename,
        std::vector<Vec3f>& vertList,
        std::vector<Vec3ui>& faceList) {

    std::vector<VertexSTL> all_vertices = load_stl_vertices(filename);
    const int32_t num_indices = all_vertices.size();
    for (int c=0;c<all_vertices.size();c++)
        all_vertices[c].i = c;

    int32_t *indices;
    indices = (int32_t *) malloc(num_indices * sizeof(int32_t));

    std::sort(all_vertices.begin(), all_vertices.end());

    float minx =  999999;
    float miny =  999999;
    float minz =  999999;
    float maxx = -999999;
    float maxy = -999999;
    float maxz = -999999;

    unsigned int num_vertices = 0;
    for (int i=0;i<all_vertices.size();i++)
    {
        VertexSTL v = all_vertices[i];
        if (!num_vertices || v != all_vertices[num_vertices-1])
        {
            all_vertices[num_vertices++] = v;
            if (v.x < minx) minx = v.x;
            if (v.x > maxx) maxx = v.x;
            if (v.y < miny) miny = v.y;
            if (v.y > maxy) maxy = v.y;
            if (v.z < minz) minz = v.z;
            if (v.z > maxz) maxz = v.z;
        }
        indices[v.i] = num_vertices - 1;
    }
    all_vertices.resize(num_vertices);

    for (int i=0;i<all_vertices.size();i++)
    {
        VertexSTL _v = all_vertices[i];
        vertList.push_back(Vec3f(_v.x, _v.y, _v.z));
    }

    for (int i=0;i<num_indices;i+=3) {
        faceList.push_back(Vec3ui(indices[i], indices[i+1], indices[i+2]));
    }
}
