#include "communicator.hpp"

void Communicator::send_msg(std::string msg) {
	if (nullptr == coms_obj_m) {
		std::cout << "No CommunicationObj set, unable to send msg.";
		return;
	}
	coms_obj_m->send_msg(msg);
}

void Communicator::set_coms_obj(std::shared_ptr<CommunicationObj> coms_obj) {
	coms_obj_m = coms_obj;
}

void Communicator::report_error(std::string reporter, std::string error) {
	std::string error_msg = "ERROR: [" + reporter + "] " + error;
	std::cerr << error_msg << std::endl;
	this->send_msg(error_msg);
	//TODO report to log when loggin gets implemented
}