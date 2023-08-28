#include <chestOpenObject.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace COO;

ChestOpenObject::ChestOpenObject ( std::string input ) {
    int dS = input.find_first_of('[');
    int dStop = input.find_first_of(']', dS);
    setTime(input.substr(dS, dStop - dS));

    int worldStart = input.find_first_of('[', dStop);
    int worldStop = input.find_first_of(']', worldStart);
    worldName = input.substr(worldStart + 1, worldStop - worldStart - 1);

    int nameStart = input.find_first_of('<', worldStop);
    int nameStop = input.find_last_of('>');
    playerName = input.substr(nameStart + 1, nameStop - nameStart - 1); 

    int X_pos = input.find("X=", nameStop) + 3;
    int X_pos_end = input.find_first_of(',', X_pos) - 1;
    x = std::stoll(input.substr(X_pos, X_pos_end - X_pos + 1));

    int Y_pos = input.find("Y=", X_pos_end) + 3;
    int Y_pos_end = input.find_first_of(',', Y_pos) - 1;
    y = std::stoll(input.substr(Y_pos, Y_pos_end - Y_pos + 1));

    int Z_pos = input.find("Z=", Y_pos_end) + 3;
    int Z_pos_end = input.find_first_of(' ', Z_pos) - 1;
    z = std::stoll(input.substr(Z_pos, Z_pos_end - Z_pos + 1));

    int chest_type_pos = input.find("opened", nameStop) + 7;
    int chest_type_pos_end = input.find(" at ", chest_type_pos) - 1;
    chest_type = input.substr(chest_type_pos, chest_type_pos_end - chest_type_pos + 1);

    int containment_pos = input.find_last_of('[');
    int containment_pos_end = input.find_last_of(']');

    if (containment_pos_end - containment_pos < 2) {
        items = std::vector<Item>();
    } else {
        std::string containment_string = input.substr(containment_pos + 1, containment_pos_end - containment_pos - 1);
        containment_string.erase(std::remove(containment_string.begin(), containment_string.end(), ' '), containment_string.end());
        int containment_elements_count = std::count(containment_string.begin(), containment_string.end(), 'x');
        std::stringstream containment_stream(containment_string);

        std::string individual_items;
        while (std::getline(containment_stream, individual_items, ',')) {
            int border = individual_items.find('x');
            std::string item_name = individual_items.substr(0, border);
            uint8_t item_cnt = (uint8_t)(std::stoi(individual_items.substr(border + 1, individual_items.length() - border - 1)));
            

            auto existing_item = findItem(item_name);
            if (existing_item.has_value()) {
                items[existing_item.value()].item_count += item_cnt;
            } else {
                items.push_back(Item(item_name, item_cnt));
            }
        }
    }
}

std::vector<Item> ChestOpenObject::operator- (ChestOpenObject other) {
    std::vector<Item> return_vector = items;
    std::vector<Item> minus_vector = other.getItems();

    for (Item item : minus_vector) {
        auto existing_item = findItem(return_vector, item.item_name);
        if ( existing_item.has_value() ) {
            return_vector[existing_item.value()].item_count -= item.item_count;
        } else {
            return_vector.push_back ( Item (item.item_name, -item.item_count) );
        }
    }

    return_vector.erase(std::remove_if (return_vector.begin(), return_vector.end(), [&](Item item) {
        return (item.item_count == 0);
    }), return_vector.end());

    return return_vector;
}

void ChestOpenObject::printItems(std::vector<COO::Item> other_items) {
    for (COO::Item item : other_items) {
        std::cout << item.item_name << " x " << item.item_count << '\n';
    }
}

void ChestOpenObject::printItems() const {
    printItems(items);
}