#include <placeBreakObject.h>

using namespace PBO;

PlaceBreakObject::PlaceBreakObject(std::string string_inp) {
    int dS = string_inp.find_first_of('[');
    int dStop = string_inp.find_first_of(']');

    setTime(string_inp.substr(0, dStop));

    int X_start = string_inp.find_last_of('X') + 3;
    int X_end = string_inp.find_first_of(',', X_start);
    x = std::stoll(string_inp.substr(X_start, X_end - X_start));

    int Y_start = string_inp.find_last_of('Y') + 3;
    int Y_end = string_inp.find_first_of(',', Y_start);
    y = std::stoll(string_inp.substr(Y_start, Y_end - Y_start));

    int Z_start = string_inp.find_last_of('Z') + 3;
    z = std::stoll(string_inp.substr(Z_start, string_inp.length() - Z_start));

    int player_start = string_inp.find_first_of('<');
    int player_stop = string_inp.find_last_of('>');
    playerName = string_inp.substr(player_start + 1, player_stop - player_start - 1);

    int block_start = string_inp.find_first_of(' ', player_stop + 6);
    int block_stop = string_inp.find_first_of(' ', block_start + 1);
    blockName = string_inp.substr(block_start + 1, block_stop - block_start - 1);

    int world_start = string_inp.find_first_of('[', dS + 1);
    int world_stop = string_inp.find_first_of(']', world_start);
    worldName = string_inp.substr(world_start + 1, world_stop - world_start - 1);

    breaking = string_inp.find("broke", player_stop) != std::string::npos;
}

PlaceBreakObject::PlaceBreakObject(PlaceBreakObject *other) {
    this->x = other->x;
    this->y = other->y;
    this->z = other->z;
    this->breaking = other->breaking;
    this->playerName = other->playerName;
    this->blockName = other->blockName;
    this->objectTime = other->objectTime;
    this->worldName = other->worldName;
}

std::string PlaceBreakObject::toString() {
    return playerName + " | " + blockName + " | " + worldName + " | X" + std::to_string(x) + " | Y" + std::to_string(y) + " | Z" + std::to_string(z) + " | " + asctime(gmtime(&objectTime)) + " | " + std::to_string(breaking);
}