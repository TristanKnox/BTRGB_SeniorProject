#include "ImageWriter.hpp"

namespace btrgb {

    void ImageWriter::write(image* im) {
        write(im, im->filename());
    }

    void ImageWriter::write(image* im, std::string filename) {

        std::string fname = filename;
        if (!fname.ends_with(this->file_extension))
            fname += this->file_extension;

        this->_write(im, fname);

    }

}