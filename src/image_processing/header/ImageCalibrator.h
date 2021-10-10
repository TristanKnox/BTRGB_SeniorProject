//
// Created by ThinkPad41 on 10/10/2021.
//

#ifndef BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H
#define BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H


#include "ImgProcessingComponent.h"

class ImageCalibrator : public ImgProcessingComponent{
private:
    vector<shared_ptr<ImgProcessingComponent>> components;
public:
    ImageCalibrator(const vector<shared_ptr<ImgProcessingComponent>>& components);
    void execute(CallBackFunction func) override;

};


#endif //BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H
