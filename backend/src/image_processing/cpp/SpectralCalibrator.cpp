#include "../header/SpectralCalibrator.h"

void SpectralCalibrator::execute(CommunicationObj *comms, btrgb::ArtObject* images) {
    comms->send_info("", this->get_name());
    comms->send_progress(0, this->get_name());
    
    btrgb::Image* art1;
    btrgb::Image* art2;
    ColorTarget target1;
    ColorTarget target2;

    // Extract images and RefData from art object
    try {
        art1 = images->getImage(ART(1));
        art2 = images->getImage(ART(2));
        this->ref_data = images->get_refrence_data();
    }
    catch (const btrgb::ArtObj_ImageDoesNotExist& e) {
        comms->send_info("Error: Flatfielding called out of order. Missing at least 1 image assignment.", this->get_name());
        return;
    }
    catch (const std::logic_error& e) {
        std::string error(e.what());
        comms->send_info("Error: " + error, this->get_name());
        return;
    }

    // Init Color Targets
    target1 = images->get_target(TARGET(1), btrgb::TargetType::GENERAL_TARGET);
    target2 = images->get_target(TARGET(2), btrgb::TargetType::GENERAL_TARGET);
    ColorTarget targets[] = { target1, target2 };
    int channel_count = art1->channels();
    int target_count = std::size(targets);

    comms->send_progress(0.2, this->get_name());

    //Init ColorTarget Averages
    this->color_patch_avgs = btrgb::calibration::build_target_avg_matrix(targets, target_count, channel_count);
    
    // Convert RefData to matrix
    cv::Mat ref_data_matrix = this->ref_data->as_matrix();

    // Initialize M_relf starting values
    this->init_M_refl(ref_data_matrix);

    comms->send_progress(0.4, this->get_name());

    // Create Custom WeightedErrorFunction used to minimize Z
    cv::Ptr<cv::MinProblemSolver::Function> ptr_F(new WeightedErrorFunction(
            &ref_data_matrix,
            &this->input_array,
            &this->M_refl,
            &this->color_patch_avgs,
            &this->R_camera
        )
    );

    //Init MinProblemSolver
    cv::Ptr<cv::DownhillSolver> min_solver = cv::DownhillSolver::create();
    min_solver->setFunction(ptr_F);
    
    double initial_stp_value = 0.75;
    cv::Mat step;
    this->init_step(initial_stp_value, step);
    min_solver->setInitStep(step);
    min_solver->setTermCriteria(
        cv::TermCriteria(
            cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, // Term Type
                5000, // max itterations
                1e-10 // epsilon
        )
    );
    
    std::cout << "Running Minimization." << std::endl;
    comms->send_progress(0.5, this->get_name());

    TimeTracker time_tracker;
    time_tracker.start_timeing();
    // Optimize M_refl to minimized Z
    double res = min_solver->minimize(this->input_array);
    time_tracker.end_timeing();

    comms->send_progress(0.9, this->get_name());

    this->store_results(images);  

    this->store_spectral_img(images); 

    std::cout << "SpectralCalibration done" << std::endl;
    comms->send_progress(1, this->get_name());
}

void SpectralCalibrator::init_M_refl(cv::Mat R_ref){
    int row_count = this->color_patch_avgs.rows;
    int col_count = this->color_patch_avgs.cols;
    // Init psudoinvers of ColorPatch Averages
    cv::Mat psudoinvers = cv::Mat_<double>(col_count, row_count, CV_32FC1);
    cv::invert(this->color_patch_avgs, psudoinvers, cv::DECOMP_SVD);

    // Create M_refl
    this->M_refl = R_ref * psudoinvers;
    // Create 1d representation of M_refl, used as input to the MinProblemSolver
    this->input_array = cv::Mat(this->M_refl).reshape(0,1);   

    psudoinvers.release(); 
}

void SpectralCalibrator::init_step(double stp_value, cv::Mat &step){
    int step_len = this->M_refl.rows * this->M_refl.cols;
    step = cv::Mat_<double>(1,step_len);
    for(int i = 0; i < step_len; i++){
        step.at<double>(0,i) = stp_value;
    }
}

void SpectralCalibrator::store_results(btrgb::ArtObject *images){
    CalibrationResults *results_obj = images->get_results_obj(btrgb::ResultType::CALIBRATION);
    cv::Mat R_ref = this->ref_data->as_matrix();
    double RMSE = btrgb::calibration::compute_RMSE(this->R_camera, R_ref);

    // R refercence
    results_obj->store_matrix(SP_R_reference, R_ref);
    // Optimized R camera
    results_obj->store_matrix(SP_R_camera, this->R_camera);
    // Optimized M refl
    results_obj->store_matrix(SP_M_refl, this->M_refl); 
    // RMSE
    results_obj->store_double(SP_RMSE, RMSE);
   
}

void SpectralCalibrator::store_spectral_img(btrgb::ArtObject *images){
    // Build Spectral Image
    btrgb::Image* art1 = images->getImage("art1");
    btrgb::Image* art2 = images->getImage("art2");
    btrgb::Image* art[2] = {art1, art2};
    int height = images->get_results_obj(btrgb::ResultType::GENERAL)->get_int(GI_IMG_ROWS);
    int width = images->get_results_obj(btrgb::ResultType::GENERAL)->get_int(GI_IMG_COLS);
    cv::Mat camra_sigs = btrgb::calibration::build_camra_signals_matrix(art, 2, 6);
    btrgb::Image *spectral_img = btrgb::calibration::camera_sigs_2_image(camra_sigs, height);

    // Save Spectral Image
    spectral_img->setConversionMatrix(BTRGB_M_REFL_OPT, this->M_refl);
    spectral_img->setExifTags(art1->getExifTags());
    images->setImage(SP_IMAGE_KEY, spectral_img);
}



////////////////////////////////////////////////////////////////
//                      WeightedErrorFunction                 //
////////////////////////////////////////////////////////////////

int WeightedErrorFunction::itteration_count = 0;

WeightedErrorFunction::WeightedErrorFunction(cv::Mat *ref_data, cv::Mat *input_array, cv::Mat *M_refl, cv::Mat *cp_carmera_sigs, cv::Mat *R_camera){
    /**
     * NOTE: input_array, M_refl, R_camera are all references
     * When the values of those matracies are updated here they are updated in SpectalCalibrator
     * This menas that once optimization is complete SpectralCalibrator already has the resulting values
     * See doc strings in SpectralCalibrator for details on each of these matracies 
     */
    this->ref_data = ref_data;
    this->input_array = input_array;
    this->M_refl = M_refl;
    this->cp_carmera_sigs = cp_carmera_sigs;
    this->R_camera = R_camera;
}

int WeightedErrorFunction::getDims() const{
    int dim = this->M_refl->rows * this->M_refl->cols;
    // std::cout << "GetDims: " << dim << std::endl;
    return dim;
}

double WeightedErrorFunction::calc(const double *x) const{
    this->itteration_count++;
    // Copy input into M_refl. x represents the changes made to the input data (M_refl)
    // M_refl is a 2d representation of input data
    for (int row = 0; row < this->M_refl->rows; row++){
        for(int col = 0; col < this->M_refl->cols; col++){
            int i = col + row * this->M_refl->cols;
            double val = x[i];
            this->M_refl->at<double>(row,col) = val;
        }
    }
    
    /**
     * Before we can compute Z we need to compute the R_camera values
     *  R_camera  = M_refl * cp_camera_sigs
     * 
     * Where
     *  cp_camera_sigs is a 2d Matrix in the form
     *   (cp_camera_sigs is the average pixel value from the color target in the ColorManaged image)
     *       cp_avg_chan1_patch_1, cp_avg_chan1_patch_2, ..., cp_avg_chan1_patch_k
     *       cp_avg_chan2_patch_1, cp_avg_chan2_patch_2, ..., cp_avg_chan2_patch_k
     *       ...                 , ...                 , ..., ...
     *       cp_avg_chan6_patch_1, cp_avg_chan6_patch_2, ..., cp_avg_chan6_patch_k 
     *    
     *  M_refl is a 2d Matrix in the form
     *      m_1_1,  m_1_2,  ..., m_1_6
     *      m_2_1,  m_2_2,  ..., m_2_6
     *      ...  ,  ...  ,  ..., ...
     *      m_36_1, m_36_2, ..., m_36_6
     * 
     *  R_camera is a 2d Matrix in fthe form
     *      RLamda_1_1,  RLamda_1_2,  ..., RLamda_1_k
     *      RLamda_2_1,  RLamda_2_2,  ..., RLamda_2_k
     *      ...       ,  ...       ,  ..., ...
     *      RLamda_36_1, RLamda_36_2, ..., RLamda_36_k
     *       
     */

    // Calculate R_camera from ColorPatch camera signals and current M_refl
    *this->R_camera = (*this->M_refl) * (*this->cp_carmera_sigs);
    
    // Calculate e values
    double e1 = this->calc_e1();
    double e2 = this->calc_e2();
    double e3 = this->calc_e3();

    // Calculate Z
    double z = this->calc_z(e1,e2,e3);

    // std::cout << "Z: " << z << " e1: " << e1 << " e2: " << e2 << " e3: " << e3 << std::endl; 
    return z;
}

double WeightedErrorFunction::calc_e1()const{
    double sum = 0;
    for(int row = 0; row < this->ref_data->rows; row++){
        for(int col = 0; col < this->ref_data->cols; col++){
            double cp_ref = this->ref_data->at<double>(row, col);
            double cp_camera = this->R_camera->at<double>(row,col);
            double cp_diff = cp_ref - cp_camera;
            sum += pow(cp_diff, 2);
            // std::cout << "cp_ref: " << cp_ref << " cp_camera: " << cp_camera << " cp_diff: " << cp_diff << " sum: " << sum << std::endl;
            // btrgb::calibration::enter_to_continue();
        }
    }
    double sqroot = sqrt(sum);
    // std::cout << "e1 sum: " << sum << " sqroot: " << sqroot << "   ";
    return sqroot;
}

double WeightedErrorFunction::calc_e2()const{
    double e2 = 0;
    for(int row = 0; row < this->ref_data->rows; row++){
        double camera_max = btrgb::calibration::row_max(*this->R_camera, row);
        double ref_max = btrgb::calibration::row_max(*this->ref_data, row);
        e2 += pow((camera_max - ref_max), 2);
    }
    return e2;
}

double WeightedErrorFunction::calc_e3()const{
    double e3 = 0;
    for(int row = 0; row < this->ref_data->rows; row++){
        double camera_min = btrgb::calibration::row_min(*this->R_camera, row);
        double ref_min = btrgb::calibration::row_min(*this->ref_data, row);
        e3 += pow((camera_min - ref_min), 2);
    }
    return e3;
}

double WeightedErrorFunction::calc_z(double e1, double e2, double e3)const{
    double z = e1 + 10 * e2 + 50 * e3;
    return z;
}
