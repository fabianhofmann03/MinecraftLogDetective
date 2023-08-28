#include <playerInteractionObject.h>
#include <iostream>

using namespace PIO;

void PlayerInteractionObject::setTime(std::string input) {
    struct tm t = {0};

    int dS = input.find_first_of('[');

    t.tm_isdst = -1;
    t.tm_year = std::stoi(input.substr(dS + 1, 4)) - 1900;
    t.tm_mon = std::stoi(input.substr(dS + 6, 2)) - 1;
    t.tm_mday = std::stoi(input.substr(dS + 9, 2));
    t.tm_hour = std::stoi(input.substr(dS + 12, + 2));
    t.tm_min = std::stoi(input.substr(dS + 15, 2));
    t.tm_sec = std::stoi(input.substr(dS + 18, 2));

    objectTime = mktime(&t);
}

void PlayerInteractionObject::printBaseData() {
    std::cout << std::to_string(objectTime) << " | " << playerName << " | " << worldName << " | " << std::to_string(this->x) << " | " << std::to_string(this->y) << " | " << std::to_string(this->z);
}