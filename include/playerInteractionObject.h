#pragma once
#include <time.h>
#include <string>
#include <stdint.h>

namespace PIO {
    class PlayerInteractionObject {
        protected:
            time_t objectTime;
            std::string playerName;
            std::string worldName;
            
            int64_t x;
            int64_t y;
            int64_t z;
        public:
            virtual ~PlayerInteractionObject() {};
            void setTime (std::string time_string );
            time_t getobjectTime() const {return objectTime;}
            std::string getplayername() const {return playerName;}
            int64_t getx() const {return x;}
            int64_t gety() const {return y;}
            int64_t getz() const {return z;}
            void printBaseData();
    };
}

