#include "ComputeGraphletsWrapper.hpp"
#include "computeGraphlets.hpp"
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

vector<vector<uint>> ComputeGraphletsWrapper::loadGraphletDegreeVectors(const Graph& G, uint maxGraphletSize) {
    string graphletSizeStr = to_string(maxGraphletSize);
    string subfolder = "gdv"+graphletSizeStr+"/";
    createFolder(AUTOGENEREATED_FILES_FOLDER + subfolder);
    string gdvsFileName = AUTOGENEREATED_FILES_FOLDER+subfolder+"/"+G.getName()+"_gdv"+graphletSizeStr+".bin";
    uint n = G.getNumNodes();
    if (fileExists(gdvsFileName) && !newerGraphAvailable(G.getFilePath().c_str(), gdvsFileName.c_str())) {
        uint degVecLen;
        if (maxGraphletSize == 4) degVecLen = 15;
        else if (maxGraphletSize == 5) degVecLen = 73;
        else throw runtime_error("only 4 or 5 max graphlet size supported");
        vector<vector<uint>> gdvs(n, vector<uint> (degVecLen));
        readMatrixFromBinaryFile(gdvs, gdvsFileName);
        return gdvs;
    }
    cout << "Computing " << gdvsFileName << " ... ";
    Timer T;
    T.start();
    vector<vector<uint>> gdvs = computeGraphletDegreeVectors(G, maxGraphletSize);
    cout << "loadGraphletDegreeVectors done (" << T.elapsedString() << ")" << endl;
    writeMatrixToBinaryFile(gdvs, gdvsFileName);
    string readeableVersionFile = AUTOGENEREATED_FILES_FOLDER+subfolder+G.getName()+"_gdv"+graphletSizeStr+".txt";
    writeMatrixToFile(gdvs, readeableVersionFile);
    return gdvs;
}

bool ComputeGraphletsWrapper::newerGraphAvailable(const char* graphDir, const char* binaryDir) {
    struct stat st;
    stat(graphDir, &st);
    time_t graphTime = st.st_mtime;
    if (stat(binaryDir, &st) != 0) return true;
    return (graphTime > st.st_mtime);
}

vector<vector<uint>> ComputeGraphletsWrapper::computeGraphletDegreeVectors(const Graph& G, uint maxGraphletSize) {
    cout<<"Computing Graphlet Degree Vectors... "<<endl;
    FILE *fp = tmpfile();
    fprintf(fp, "%d %d\n", G.getNumNodes(), G.getNumEdges()); 
    for (const auto& edge : *(G.getEdgeList())) {
        fprintf(fp, "%d %d\n", edge[0], edge[1]);
    }
    rewind(fp); //because computeGraphlets starts reading the file from the start
    vector<vector<uint>> gdvs = computeGraphletsSource::computeGraphlets(maxGraphletSize, fp);
    fclose(fp);
    return gdvs;
}

void ComputeGraphletsWrapper::saveGraphletsAsSigs(const Graph& G, uint maxGraphletSize, const string& outFile) {
    vector<vector<uint>> graphlets = computeGraphletDegreeVectors(G, maxGraphletSize);
    ofstream ofs;
    ofs.open(outFile.c_str());
    for (unsigned int i = 0; i < G.getNumNodes(); i++) {
        ofs << G.getNodeName(i) << "\t";
        for (int j = 0; j < 73; j++) { //why 73? doesn't it depend on maxGraphletSize? -Nil
            ofs << graphlets[i][j] << "\t";
        }
        ofs<<endl;
    }
    ofs.close();
}

void ComputeGraphletsWrapper::writeMatrixToFile(const vector<vector<uint>>& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ofstream fout(fileName.c_str());
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            fout << matrix[i][j] << " ";
        }
        fout << endl;
    }
    fout.close();
}

void ComputeGraphletsWrapper::writeMatrixToBinaryFile(const vector<vector<uint>>& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ofstream fout(fileName.c_str(), ios::out | ios::binary);
    for (uint i = 0; i < n1; i++) {
        fout.write((char*)&matrix[i][0], n2*sizeof(uint));
    }
    fout.close();
}

void ComputeGraphletsWrapper::readMatrixFromBinaryFile(vector<vector<uint>>& matrix, const string& fileName) {
    uint n1 = matrix.size();
    uint n2 = matrix[0].size();
    ifstream fin(fileName.c_str(), ios::in | ios::binary);
    for (uint i = 0; i < n1; i++) {
        fin.read((char*)&matrix[i][0], n2*sizeof(uint));
    }
    fin.close();
}
