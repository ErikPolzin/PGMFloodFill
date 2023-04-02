#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <limits>
#include "PGMMetadata.h"
#include "PGMImageProcessor.h"


namespace PLZERI001{
PGMImageProcessor::PGMImageProcessor(const PGMMetadata &md): mdata(md)
{
    data = new std::tuple<unsigned char, bool>*[mdata.height];
    for (int i = 0; i < mdata.height; ++i) {
        data[i] = new std::tuple<unsigned char, bool>[mdata.width];
    }
}

int PGMImageProcessor::extractComponents(int threshold, int minValidSize)
{
    if (data == nullptr) return -1;
    int width = mdata.width; int height = mdata.height;
    for (int y = 0; y < mdata.height; ++y) {
        for (int x = 0; x < mdata.width; ++x) {
            if (std::get<1>(data[y][x])) continue;
            if (std::get<0>(data[y][x]) > threshold) {
                ConnectedComponent component(mdata.width, mdata.height, data);
                component.seedAt(x, y, threshold);
                if (component.getSize() > minValidSize) {
                    components.insert(component);
                }
            } else {
                std::get<0>(data[y][x]) = 0;
            }
        }
    }
    return components.size();
}

int PGMImageProcessor::filterComponentsBySize(int m, int M)
{
    // Couldn't get this to work using the STL :(
    // std::remove_if(components.begin(), components.end(),
    //     [m, M](ConnectedComponent& c)->bool{return c.getSize() < m || c.getSize() > M;});
    std::multiset<ConnectedComponent> componentsCopy;
    for (const ConnectedComponent &c: components) {
        if (c.getSize() > m && c.getSize() < M) componentsCopy.insert(c);
    }
    components = componentsCopy;
    return components.size();
}
bool PGMImageProcessor::writeComponents(const std::string &outFileName)
{
    std::ofstream outFile;
    outFile.open(outFileName, std::ios::binary);
    if (!outFile) {
        std::cout << "Unable to open output file." << std::endl;
        return false;
    }
    outFile << mdata;
    int i = 0;
    for (int y = 0; y < mdata.height; ++y) {
        for (int x = 0; x < mdata.width; ++x) {
            outFile << std::get<0>(data[y][x]);
        }
    }
    std::cout << "Wrote components to file at " << outFileName << std::endl;
    return true;
}
int PGMImageProcessor::getComponentCount(void) const { return components.size(); }

int PGMImageProcessor::getLargestSize(void) const {
    int largest = -1;
    for (const ConnectedComponent &c: components) {
        if (c.getSize() > largest) largest = c.getSize();
    }
    return largest;
}

int PGMImageProcessor::getSmallestSize(void) const {
    int smallest = std::numeric_limits<int>::max();
    for (const ConnectedComponent &c: components) {
        if (c.getSize() < smallest) smallest = c.getSize();
    }
    return smallest;
}

void PGMImageProcessor::printComponentData(const ConnectedComponent &c) const
{
    std::cout << c.to_string() << std::endl;
}

void PGMImageProcessor::printComponents(void) const
{
    for (ConnectedComponent c: components) printComponentData(c);
}

std::istream &operator>>(std::istream &stream, PGMImageProcessor &processor)
{
    for (int i = 0; i < processor.mdata.height; ++i) {
        for (int j = 0; j < processor.mdata.width; ++j) {
            processor.data[i][j] = std::make_tuple(stream.get(), false);
        }
    }
    return stream;
}
}