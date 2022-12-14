
#ifndef BEYOND_RGB_BACKEND_PIXELREGESTOR_H
#define BEYOND_RGB_BACKEND_PIXELREGESTOR_H

#include "image_processing/header/LeafComponent.h"
#include "ImageUtil/Image.hpp"

class PixelRegestor : public LeafComponent{
private: std::string RegistrationFactor;
public:
    ~PixelRegestor() {};
    PixelRegestor(std::string RegistrationFactor) : LeafComponent("Registering"), RegistrationFactor(RegistrationFactor) {};
    void execute(CommunicationObj* comms, btrgb::ArtObject* images) override;
    void appy_regestration(CommunicationObj* comms, btrgb::Image *img1, btrgb::Image *img2, int cycle, int cycle_count);
    float calc_progress(float progress, float cycle, float cycle_count);
};

#endif //BEYOND_RGB_BACKEND_PIXELREGESTOR_H