//
// Created by jhone on 12/08/2025.
//

#ifndef TEXTURE_PACKER_H
#define TEXTURE_PACKER_H
#include <string>

class TexturePacker {
    const int ATLAS_WIDTH = 2048;
    const int ATLAS_HEIGHT = 2048;
    const int MARGIN = 1;

public:
    TexturePacker() = default;

    ~TexturePacker() = default;

    void packer(
    const std::string &inputDir,
    const std::string &outputAtlas,
    const std::string &outputJson) const;

};


#endif //TEXTURE_PACKER_H
