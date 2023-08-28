#pragma once
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <time.h>
#include <string>

#include <playerInteractionObject.h>

namespace PBO {
    class PlaceBreakObject : public PIO::PlayerInteractionObject {
        private:
            bool breaking;         // true: breaking, false: placing
            std::string blockName;
        public:
            PlaceBreakObject(std::string string);
            PlaceBreakObject(PlaceBreakObject *other);
            std::string toString();
            int64_t getbreaking() const {return breaking;}
            std::string getblockname() const {return blockName;}
            std::string getworldname() const {return worldName;}
    };
}
