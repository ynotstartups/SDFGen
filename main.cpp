//SDFGen - A simple grid-based signed distance field (level set) generator for triangle meshes.
//Written by Christopher Batty (christopherbatty@yahoo.com, www.cs.columbia.edu/~batty)
//...primarily using code from Robert Bridson's website (www.cs.ubc.ca/~rbridson)
//This code is public domain. Feel free to mess with it, let me know if you like it.

#include "makelevelset3.h"
#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

extern "C" {
    int SDFGen(const char* _filename, const float dx, int padding) {

    std::string filename(_filename);
    if(filename.size() < 5 || filename.substr(filename.size()-4) != std::string(".obj")) {
      std::cerr << "Error: Expected OBJ file with filename of the form <name>.obj.\n";
      exit(-1);
    }

    if(padding < 1) padding = 1;
    //start with a massive inside out bound box.
    Vec3f min_box(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()), 
      max_box(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max());

    std::cout << "Reading data with name " << _filename << "\n";

    std::ifstream infile(filename);
    if(!infile) {
      std::cerr << "Failed to open. Terminating.\n";
      exit(-1);
    }

    int ignored_lines = 0;
    std::string line;
    std::vector<Vec3f> vertList;
    std::vector<Vec3ui> faceList;
    while(!infile.eof()) {
      std::getline(infile, line);

      //.obj files sometimes contain vertex normals indicated by "vn"
      if(line.substr(0,1) == std::string("v") && line.substr(0,2) != std::string("vn")){
        std::stringstream data(line);
        char c;
        Vec3f point;
        data >> c >> point[0] >> point[1] >> point[2];
        vertList.push_back(point);
        update_minmax(point, min_box, max_box);
      }
      else if(line.substr(0,1) == std::string("f")) {
        std::stringstream data(line);
        char c;
        int v0,v1,v2;
        data >> c >> v0 >> v1 >> v2;
        faceList.push_back(Vec3ui(v0-1,v1-1,v2-1));
      }
      else if( line.substr(0,2) == std::string("vn") ){
        std::cerr << "Obj-loader is not able to parse vertex normals, please strip them from the input file. \n";
        exit(-2); 
      }
      else {
        ++ignored_lines; 
      }
    }
    infile.close();

    if(ignored_lines > 0)
      std::cout << "Warning: " << ignored_lines << " lines were ignored since they did not contain faces or vertices.\n";

    std::cout << "Read in " << vertList.size() << " vertices and " << faceList.size() << " faces." << std::endl;

    //Add padding around the box.
    Vec3f unit(1,1,1);
    min_box -= padding*dx*unit;
    max_box += padding*dx*unit;
    Vec3ui sizes = Vec3ui((max_box - min_box)/dx);

    std::cout << "Bound box size: (" << min_box << ") to (" << max_box << ") with dimensions " << sizes << "." << std::endl;

    std::cout << "Computing signed distance field.\n";
    Array3f phi_grid;
    make_level_set3(faceList, vertList, min_box, dx, sizes[0], sizes[1], sizes[2], phi_grid);

    std::string outname;

      // if VTK support is missing, default back to the original ascii file-dump.
      //Very hackily strip off file suffix.
      outname = filename.substr(0, filename.size()-4) + std::string(".sdf");
      std::cout << "Writing results to: " << outname.c_str() << "\n";

      std::ofstream outfile( outname.c_str(), std::ios::binary);

      outfile.write(reinterpret_cast<const char *>(&phi_grid.ni), sizeof(phi_grid.ni)); // int
      outfile.write(reinterpret_cast<const char *>(&phi_grid.nj), sizeof(phi_grid.nj)); // int
      outfile.write(reinterpret_cast<const char *>(&phi_grid.nk), sizeof(phi_grid.nk)); // int
      outfile.write(reinterpret_cast<const char *>(&min_box), sizeof(min_box[0])*3); // 3 float
      outfile.write(reinterpret_cast<const char *>(&dx), sizeof(dx)); // 1 float

      const auto _size = phi_grid.a.size();
      if (_size > 0)
          outfile.write(
              reinterpret_cast<const char *>(&(phi_grid.a[0])),
              _size*sizeof(phi_grid.a[0]) // 1 float
          );
      outfile.close();

    std::cout << "Processing complete.\n";
    return 0;
    }
}

int main(int argc, char* argv[]) {
  
  if(argc != 4) {
    std::cout << "SDFGen - A utility for converting closed oriented triangle meshes into grid-based signed distance fields.\n";
    std::cout << "\nThe output file format is:";
    std::cout << "<ni> <nj> <nk>\n";
    std::cout << "<origin_x> <origin_y> <origin_z>\n";
    std::cout << "<dx>\n";
    std::cout << "<value_1> <value_2> <value_3> [...]\n\n";
    
    std::cout << "(ni,nj,nk) are the integer dimensions of the resulting distance field.\n";
    std::cout << "(origin_x,origin_y,origin_z) is the 3D position of the grid origin.\n";
    std::cout << "<dx> is the grid spacing.\n\n";
    std::cout << "<value_n> are the signed distance data values, in ascending order of i, then j, then k.\n";

    std::cout << "The output filename will match that of the input, with the OBJ suffix replaced with SDF.\n\n";

    std::cout << "Usage: SDFGen <filename> <dx> <padding>\n\n";
    std::cout << "Where:\n";
    std::cout << "\t<filename> specifies a Wavefront OBJ (text) file representing a *triangle* mesh (no quad or poly meshes allowed). File must use the suffix \".obj\".\n";
    std::cout << "\t<dx> specifies the length of grid cell in the resulting distance field.\n";
    std::cout << "\t<padding> specifies the number of cells worth of padding between the object bound box and the boundary of the distance field grid. Minimum is 1.\n\n";
    
    exit(-1);
  }


return 0;
}
