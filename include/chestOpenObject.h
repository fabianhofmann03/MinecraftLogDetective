#pragma once
#include "playerInteractionObject.h"
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <time.h>
#include <optional>

namespace COO {
    struct Item {
        Item(std::string name, int64_t count) {
            item_name = name;
            item_count = count;
        }
        std::string item_name;
        int64_t item_count;
    };

    class ChestOpenObject : public PIO::PlayerInteractionObject {
        private:
            std::vector<Item> items;
            std::string chest_type;

            std::optional<uint64_t> findItem(std::string item_name ) {
                return findItem(items, item_name);
            }

            static std::optional<uint64_t> findItem(std::vector<Item> localItems, std::string item_name ) {
                uint64_t i = 0;
                for ( Item item : localItems ) {
                    if (item.item_name.compare(item_name) == 0) {
                        return i;
                    }
                    i++;
                }
                return {};
            }
        public:
            ChestOpenObject (ChestOpenObject *other);
            ChestOpenObject (std::string input);

            std::vector<Item> operator- (const ChestOpenObject other);

            std::vector<Item> getItems() const { return items; }
            void printItems() const;
            static void printItems(std::vector<Item> other_items);
            std::string getChestType() const { return chest_type; }
    };
}

