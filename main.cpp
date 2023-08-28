#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <memory>  // for allocator, __shared_ptr_access
#include <string>  // for char_traits, operator+, string, basic_string
#include <ctime>
#include <iomanip>
#include <math.h>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include "ftxui/util/ref.hpp"  // for Ref
#include "ftxui/component/loop.hpp"
#include "ftxui/dom/table.hpp"

#include "emojis.h"
#include "placeBreakObject.h"
#include "chestOpenObject.h"

#define VERSION "1.2"

#define SMILEY(a) smiley = text(a)

namespace fs = std::filesystem;
using namespace ftxui;

int test_num = 0;
Element smiley;

ScreenInteractive *screenPtr;
Element (*screenFunction)();
Component component;
Loop *loopPtr;

fs::path main_path;

// ------------------ Start screen

Element startscreen_error_text;
Component input_path;
Component button_continue;

Component startScreenComponents;
Element startScreen() {
    return vbox({
            startscreen_error_text,
            hcenter(
                vbox({
                    hbox({
                        text("Pfad zu Log-Ordner eingeben: "),
                        input_path->Render(),
                        filler(),
                        }),
                    hbox({
                        filler(),
                        button_continue->Render(),
                        filler(),
                    }),
                })
            ),
            filler(),
            hcenter(text("AstroSteve111 | " + std::string(VERSION))),
            text(""),
        });
}

// ------------------ Block search screen

Component blockSearchXInput;
Component blockSearchYInput;
Component blockSearchZInput;
Component blockSearchRangeInput;
Component blockSearchButton;
Element blockSearchSearchingTerminal;
Element blockSearchError;

Element blockSearchSearchingElement() { return blockSearchSearchingTerminal; }
Element standardBlockSearchScreen() {
    return vbox({
        blockSearchError,
        vbox({
            hbox({
                text("X: "), blockSearchXInput->Render(), text(", Y: "), blockSearchYInput->Render(), text(", Z: "), blockSearchZInput->Render(),
            }),
            hbox({
                text("Radius: "), blockSearchRangeInput->Render(),
            }),
        }) | border,
        hbox(filler(), blockSearchButton->Render(), filler()),
    });
}

Element (*blockSearchScreen)() = &standardBlockSearchScreen;

// ------------------ Chest search

Component chestSearchXInput;
Component chestSearchYInput;
Component chestSearchZInput;
Component chestSearchRangeInput;
Component chestSearchButton;
Element chestSearchSearchingTerminal;
Element chestSearchError;

Element chestSearchSearchingElement() { return chestSearchSearchingTerminal; }
Element standardChestSearchScreen() {
    return vbox({
        blockSearchError,
        vbox({
            hbox({
                text("X: "), chestSearchXInput->Render(), text(", Y: "), chestSearchYInput->Render(), text(", Z: "), chestSearchZInput->Render(),
            }),
            hbox({
                text("Radius: "), chestSearchRangeInput->Render(),
            }),
        }) | border,
        hbox(filler(), chestSearchButton->Render(), filler()),
    });
}
Element (*chestSearchScreen)() = &standardChestSearchScreen;

// ------------------ Results

std::string resultName = "";
Component printResultsButton;
Elements resultElements;
std::string resultString = "";
Element resultScreen() {
    return vbox({
        text(resultName),
        text(""),
        frame(vbox(resultElements) | vscroll_indicator | focusPositionRelative(0, 1)) | border | size(HEIGHT, EQUAL, screenPtr->dimy() - 8),
        hbox(vbox(text(""), paragraph("Leider kann hier nicht gescrollt werden, jedoch können die Ergebnisse als txt Datei exportiert werden.")), filler(), printResultsButton->Render()),
        
    });
}
void resultsToTxt() {
    if (fs::exists("./results.txt")) {
        fs::remove("./results.txt");
    }
    std::ofstream file("./results.txt");
    file << resultName << "\n\n" << resultString;
    file.close();
    system("start results.txt");
}

// ------------------ Menu screen

# define RESULT_SCREEN_NUM 2

int selectedTab = 0;
int oldSelectedTab = 0;
Component tabToggle;
Component menuScreenComponents;
Element (*menuScreenFunction)();
Element (*menuScreenFunctions[])() {blockSearchScreen, chestSearchScreen, &resultScreen};
Element menuScreen() {
    if (selectedTab != oldSelectedTab) {
        menuScreenFunction = menuScreenFunctions[selectedTab];
    }
    oldSelectedTab = selectedTab;
    return vbox({
        hbox({
            tabToggle->Render(),
            filler(),
            smiley,
        }),
        separator(),
        menuScreenFunction(),
    });
}

// ------------------ Other

void setNewScreen(Element (*newScreenFunction)(), Component newComponent) {
    screenFunction = newScreenFunction;
    component = newComponent;
    screenPtr->Clear();
    screenPtr->Exit();
}

inline void updateScreen() {
    loopPtr->RunOnce();                             // Runs loop
    screenPtr->PostEvent(Event::Custom);            // Without this, display only updates when mouse moves
}

void checkPath(std::string path, Element* error_text ) {
    fs::path new_path = path;
    if (fs::exists(new_path)) {
        main_path = new_path;
        setNewScreen(&menuScreen, menuScreenComponents);
    } else {
        *error_text = color(Color::Red, text("Pfad existiert nicht! " + std::to_string(test_num)));
        test_num++;
    }
}

template <typename T>
std::vector<T> searchObjects ( std::vector<std::string> searchPlaces, std::function< std::optional<T>(std::string line, uint64_t folder_num, std::string file_name, uint64_t file_num, uint64_t max_file_num, uint64_t line_num, uint64_t max_line_num) > selectFunction ) {
    std::vector<T> list = {};
    uint64_t folder_cnt = 0;
    for ( std::string folder : searchPlaces ) {
        uint64_t file_cnt = 0;
        uint64_t max_file_cnt = std::distance(fs::begin(fs::directory_iterator(main_path / folder)), fs::end(fs::directory_iterator(main_path / folder)));
        for (const auto log_file : fs::directory_iterator(main_path / folder)) {
            std::ifstream file (fs::path(log_file.path()));
            if (file.is_open()) {
                uint64_t max_line_cnt = 0;
                std::string line;
                while (std::getline(file, line)) max_line_cnt++;
                file.clear();
                file.seekg(0);
                uint64_t line_cnt = 0;
                while (std::getline(file, line)) {
                    std::optional<T> obj = selectFunction(line, folder_cnt, to_string(fs::path(log_file.path()).filename().replace_extension("")), file_cnt, max_file_cnt, line_cnt, max_line_cnt);
                    if (obj.has_value()) {
                        list.push_back(obj.value());
                    }
                    line_cnt++;
                }
            }
            file_cnt++;
        }
        folder_cnt++;
    }
    return list;
}

void searchBlock(std::string x_str, std::string y_str, std::string z_str, std::string range_str) {
    bool check = true;
    std::string error = "";
    
    int64_t x;
    int64_t y;
    int64_t z;

    uint64_t range;

    {                       // Checking for errors in input
        try{
            x = std::stoll(x_str);
        } catch(std::exception e) {
            check = false;
            error += "X-Koordinate falsch eingegeben! ";
        }
        
        try{
            y = std::stoll(y_str);
        } catch(std::exception e) {
            check = false;
            error += "Y-Koordinate falsch eingegeben! ";
        }

        try{
            z = std::stoll(z_str);
        } catch(std::exception e) {
            check = false;
            error += "Z-Koordinate falsch eingegeben! ";
        }

        try{
            range = std::stoll(range_str);
        } catch(std::exception e) {
            check = false;
            error += "Radius falsch eingegeben! ";
        }
        
        if (!fs::exists(main_path / "Block Place")) {
            check = false;
            error += "Block Place Ordner nicht gefunden! ";
        }
        if (!fs::exists(main_path / "Block Break")) {
            check = false;
            error += "Block Place Ordner nicht gefunden! ";
        }
    }
    if(!check) {
        switch(rand() % 2) {
            case 0:
                SMILEY(EM_EMBARRASSED);
                SMILEY(EM_EMBARRASSED);
            break;
            case 1:
                SMILEY(EM_SAD);
            break;
        }
        
        blockSearchError = color(Color::Red, text(error + std::to_string(test_num)));
        test_num++;
        return;
    }else {
        
        SMILEY(EM_THINKING);
        std::vector<PBO::PlaceBreakObject> placebreakVector;

        {   // ------ Searching all files
            std::string slow_work = "Placing";
            std::string date_work = "01.01.2023";
            std::string time_work = "00:00:00";

            float slow_work_progress = 0;
            float date_work_progress = 0;
            float time_work_progress = 0;

            blockSearchScreen = &blockSearchSearchingElement;
            menuScreenFunction = blockSearchScreen;
            std::string debug_string = "";
            auto updateBlockSearch = [&] {
                blockSearchSearchingTerminal = vbox({
                    text("Search started. Don't press anything!"),
                    text(debug_string),
                    hbox(text(slow_work + " - "), gaugeRight(slow_work_progress)) | border,
                    hbox(text(date_work + " - "), gaugeRight(date_work_progress)) | border,
                    hbox(text(time_work + " - "), gaugeRight(time_work_progress)) | border,
                });
                updateScreen();
            };

            int updateCnt = 0;
            placebreakVector = searchObjects<PBO::PlaceBreakObject>({"Block Place", "Block Break"}, [&](std::string line, uint64_t folder_num, std::string file_name, uint64_t file_num, uint64_t max_file_num, uint64_t line_num, uint64_t max_line_num) -> std::optional<PBO::PlaceBreakObject> {
                PBO::PlaceBreakObject object(line);

                if (updateCnt > max_line_num / screenPtr->dimx()) {
                    slow_work_progress = (float) folder_num / 2;                        // Set terminal-information data
                    slow_work = folder_num ? "Bauen" : "Zerstören";
                    date_work_progress = (float) file_num / max_file_num;
                    date_work = file_name;
                    std::replace(date_work.begin(), date_work.end(), '-', '.');
                    time_work_progress = (float) line_num / max_line_num;
                    time_t obj_time = object.getobjectTime();
                    struct tm time_struct = *localtime(&obj_time);
                    char hours[3];
                    char minutes[3];
                    char seconds[3];
                    sprintf(hours, "%02d", time_struct.tm_hour);
                    sprintf(minutes, "%02d", time_struct.tm_min);
                    sprintf(seconds, "%02d", time_struct.tm_sec);
                    time_work = std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds);

                    updateBlockSearch();
                    updateCnt = 0;
                }
                updateCnt ++;

                if (sqrt(pow(object.getx() - x, 2) + pow(object.gety() - y, 2) + pow(object.getz() - z, 2)) <= range) {
                    return object;
                } else {
                    return {};
                }
            });
        }

        resultElements = Elements({});
        resultString = "";

        if (placebreakVector.size() == 0) {
            SMILEY(EM_DUNNO);
            
            resultName = (range == 0) ? ("Blocksuche an Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " erziehlte kein Ergebnis!") : 
                                        ("Blocksuche um Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " erziehlte kein Ergebnis!");
        } else {
            SMILEY(EM_EXCITED);
            resultName = (range == 0) ? ("Ergebniss der Blocksuche an Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z)) : 
                                        ("Ergebniss der Blocksuche um Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z));

            std::sort(placebreakVector.begin(), placebreakVector.end(), [&](PBO::PlaceBreakObject one, PBO::PlaceBreakObject two) {
                return one.getobjectTime() < two.getobjectTime();
            });

            float printingProgress = 0;
            uint64_t maxPrint = placebreakVector.size();
            uint64_t printNow = 0;
            uint64_t printScreenCnt = 0;
            std::string printOutput = "";

            auto updateBlockSearch = [&] {
                blockSearchSearchingTerminal = vbox({
                    text("Printing results. Don't press anything!"),
                    text(""),
                    hbox(text(printOutput + " - "), gaugeRight(printingProgress)) | border,
                });
                updateScreen();
            };

            for (PBO::PlaceBreakObject obj : placebreakVector) {
                time_t obj_time = obj.getobjectTime();
                struct tm time_struct = *localtime(&obj_time);

                char hours[3];
                char minutes[3];
                char seconds[3];
                char days[3];
                char months[3];

                sprintf(hours, "%02d", time_struct.tm_hour);
                sprintf(minutes, "%02d", time_struct.tm_min);
                sprintf(seconds, "%02d", time_struct.tm_sec);
                sprintf(days, "%02d", time_struct.tm_mday);
                sprintf(months, "%02d", time_struct.tm_mon + 1);

                if (printScreenCnt > maxPrint / screenPtr->dimx()) {
                    printingProgress = (float) printNow / maxPrint;
                    printScreenCnt = 0;
                    printOutput = (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds);

                    updateBlockSearch();
                    // (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)
                }
                printNow ++;
                printScreenCnt ++;

                resultElements.push_back(text((range == 0) ?    (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() +                                                                                                                                      " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) : 
                                                                (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() + " bei X" + std::to_string(obj.getx()) + " Y" + std::to_string(obj.gety()) + " Z" + std::to_string(obj.getz()) +    " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds))));
                resultString +=               (range == 0) ?    (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() +                                                                                                                                      " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) : 
                                                                (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() + " bei X" + std::to_string(obj.getx()) + " Y" + std::to_string(obj.gety()) + " Z" + std::to_string(obj.getz()) +    " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) + "\n";
            }
        }
        
        selectedTab = RESULT_SCREEN_NUM;

        blockSearchScreen = &standardBlockSearchScreen;
    }
}

void searchChest(std::string x_str, std::string y_str, std::string z_str, std::string range_str) {
    bool check = true;
    std::string error = "";
    
    int64_t x;
    int64_t y;
    int64_t z;

    uint64_t range;

    {                       // Checking for errors in input
        try{
            x = std::stoll(x_str);
        } catch(std::exception e) {
            check = false;
            error += "X-Koordinate falsch eingegeben! ";
        }
        
        try{
            y = std::stoll(y_str);
        } catch(std::exception e) {
            check = false;
            error += "Y-Koordinate falsch eingegeben! ";
        }

        try{
            z = std::stoll(z_str);
        } catch(std::exception e) {
            check = false;
            error += "Z-Koordinate falsch eingegeben! ";
        }

        try{
            range = std::stoll(range_str);
        } catch(std::exception e) {
            check = false;
            error += "Radius falsch eingegeben! ";
        }
        
        if (!fs::exists(main_path / "Chest Interaction")) {
            check = false;
            error += "Chest Interaction Ordner nicht gefunden! ";
        }
    }
    if(!check) {
        switch(rand() % 2) {
            case 0:
                SMILEY(EM_EMBARRASSED);
                SMILEY(EM_EMBARRASSED);
            break;
            case 1:
                SMILEY(EM_SAD);
            break;
        }
        
        blockSearchError = color(Color::Red, text(error + std::to_string(test_num)));
        test_num++;
        return;
    }else {
        SMILEY(EM_THINKING);
        std::vector<COO::ChestOpenObject> chestVector;

        {   // ------ Searching all files
            std::string date_work = "01.01.2023";
            std::string time_work = "00:00:00";

            float date_work_progress = 0;
            float time_work_progress = 0;

            blockSearchScreen = &blockSearchSearchingElement;
            menuScreenFunction = blockSearchScreen;
            std::string debug_string = "";
            auto updateBlockSearch = [&] {
                blockSearchSearchingTerminal = vbox({
                    text("Search started. Don't press anything!"),
                    text(debug_string),
                    hbox(text(date_work + " - "), gaugeRight(date_work_progress)) | border,
                    hbox(text(time_work + " - "), gaugeRight(time_work_progress)) | border,
                });
                updateScreen();
            };
            int updateCnt = 0;
            chestVector = searchObjects<COO::ChestOpenObject>({"Chest Interaction"}, [&](std::string line, uint64_t folder_num, std::string file_name, uint64_t file_num, uint64_t max_file_num, uint64_t line_num, uint64_t max_line_num) -> std::optional<COO::ChestOpenObject> {
                COO::ChestOpenObject object(line);

                if (updateCnt > max_line_num / screenPtr->dimx()) {
                    date_work_progress = (float)file_num / max_file_num;
                    date_work = file_name;
                    std::replace(date_work.begin(), date_work.end(), '-', '.');
                    time_work_progress = (float) line_num / max_line_num;
                    time_t obj_time = object.getobjectTime();
                    struct tm time_struct = *localtime(&obj_time);
                    char hours[3];
                    char minutes[3];
                    char seconds[3];
                    sprintf(hours, "%02d", time_struct.tm_hour);
                    sprintf(minutes, "%02d", time_struct.tm_min);
                    sprintf(seconds, "%02d", time_struct.tm_sec);
                    time_work = std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds);

                    updateBlockSearch();
                    updateCnt = 0;
                }
                updateCnt ++;

                if (sqrt(pow(object.getx() - x, 2) + pow(object.gety() - y, 2) + pow(object.getz() - z, 2)) <= range) {
                    return object;
                } else {
                    return {};
                }
            });
        }

        resultElements = Elements({});
        resultString = "";

        if (chestVector.size() == 0) {
            SMILEY(EM_DUNNO);
            
            resultName = (range == 0) ? ("Kistensuche an Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " erziehlte kein Ergebnis!") : 
                                        ("Kistensuche um Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z) + " erziehlte kein Ergebnis!");
        } else {
            SMILEY(EM_EXCITED);
            resultName = (range == 0) ? ("Ergebniss der Kistensuche an Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z)) : 
                                        ("Ergebniss der Kistensuche um Position X" + std::to_string(x) + " Y" + std::to_string(y) + " Z" + std::to_string(z));
            
            std::vector<std::vector<COO::ChestOpenObject>> sortedObjects = std::vector<std::vector<COO::ChestOpenObject>>({});

            {
                std::string chestUpdateName = "";
                float chestUpdateProgress = 0;
                uint64_t chestMax = chestVector.size();
                uint64_t chestNum = 0;
                
                auto updateBlockSearch = [&] {
                    blockSearchSearchingTerminal = vbox({
                        text("Sorting objects. Don't press anything!"),
                        text(""),
                        hbox(text(chestUpdateName + " - "), gaugeRight(chestUpdateProgress)) | border,
                    });
                    updateScreen();
                };

                int updateCnt = 0;
                for ( COO::ChestOpenObject &object : chestVector ) {                 // Sorting individual chests
                    if (updateCnt > chestMax / screenPtr->dimx()) {
                        chestUpdateProgress = (float) chestNum / chestMax;
                        chestUpdateName = object.getChestType();
                        updateCnt = 0;
                        updateBlockSearch();
                    }
                    updateCnt ++;
                    chestNum ++;

                    int64_t chestNum = -1;
                    int64_t chestCounter = 0;
                    for ( std::vector<COO::ChestOpenObject> chest : sortedObjects ) {
                        COO::ChestOpenObject first = chest[0];
                        if ( first.getx() == object.getx() && first.gety() == object.gety() && first.getz() == object.getz() && first.getChestType().compare(object.getChestType()) == 0 ) {
                            chestNum = chestCounter;
                            break;
                        }
                        chestCounter ++;
                    }

                    if (chestNum != -1) {
                        sortedObjects[chestNum].push_back(object);
                    } else {
                        std::vector<COO::ChestOpenObject> newVector = std::vector<COO::ChestOpenObject>();
                        newVector.push_back(object);
                        sortedObjects.push_back(newVector);
                    }
                }
            }

            {
                std::string chestUpdateName = "";
                float chestUpdateProgress = 0;
                uint64_t chestMax = chestVector.size();
                uint64_t chestNum = 0;

                std::string dateUpdateName = "";
                float dateUpdateProgress = 0;
                uint64_t dateMax = 0;
                uint64_t dateNum = 0;
                
                auto updateBlockSearch = [&] {
                    blockSearchSearchingTerminal = vbox({
                        text("Printing results. Don't press anything!"),
                        text(""),
                        hbox(text(chestUpdateName + " - "), gaugeRight(chestUpdateProgress)) | border,
                        hbox(text(dateUpdateName + " - "), gaugeRight(dateUpdateProgress)) | border,
                    });
                    updateScreen();
                };

                bool first = true;
                uint64_t updateCnt = 0;
                for ( std::vector<COO::ChestOpenObject> &chest : sortedObjects ) {                                          // Printing sorted chests
                    std::sort(chest.begin(), chest.end(), [&](COO::ChestOpenObject one, COO::ChestOpenObject two) {
                        return one.getobjectTime() < two.getobjectTime();
                    });

                    if ( first ) {
                        first = false;
                    } else {
                        resultElements.push_back(text(""));
                        resultString += "\n";
                    }
                    resultElements.push_back(hbox({ text("----------------------------------------------------------------------"), filler(), text(chest[0].getChestType() + " bei X" + std::to_string(chest[0].getx()) + " Y" + std::to_string(chest[0].gety()) + " Z" + std::to_string(chest[0].getz())) }));
                    resultElements.push_back(text(""));
                    resultString += "----------------------------------------------------------------------" + chest[0].getChestType() + " bei X" + std::to_string(chest[0].getx()) + " Y" + std::to_string(chest[0].gety()) + " Z" + std::to_string(chest[0].getz()) + "\n";

                    for ( int x = 1; x < chest.size(); x++ ) {
                        time_t obj_time = chest[x].getobjectTime();
                        struct tm time_struct = *localtime(&obj_time);

                        char hours[3];
                        char minutes[3];
                        char seconds[3];
                        char days[3];
                        char months[3];

                        sprintf(hours, "%02d", time_struct.tm_hour);
                        sprintf(minutes, "%02d", time_struct.tm_min);
                        sprintf(seconds, "%02d", time_struct.tm_sec);
                        sprintf(days, "%02d", time_struct.tm_mday);
                        sprintf(months, "%02d", time_struct.tm_mon + 1);

                        if (updateCnt > (chest.size() - 1) / screenPtr->dimx()) {
                            dateMax = chest.size() - 1;
                            chestUpdateProgress = (float) chestNum / chestMax;
                            dateUpdateProgress = (float) dateNum / dateMax;
                            chestUpdateName = chest[x - 1].getChestType();
                            dateUpdateName = (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds);
                            updateCnt = 0;
                            updateBlockSearch();
                        }
                        updateCnt ++;
                        dateNum++;

                        std::string time_string = "am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds);

                        std::vector<COO::Item> changedItems = chest[x] - chest[x - 1];
                        if (changedItems.size() == 0) {
                            resultElements.push_back(paragraph(chest[x].getplayername() + " hat " + time_string + " die Truhe geöffnet, hat jedoch nichts verändert."));
                            resultString += chest[x].getplayername() + " hat " + time_string + " die Truhe geöffnet, hat jedoch nichts verändert.\n";
                        } else {
                            std::vector<COO::Item> addedItems = std::vector<COO::Item>();
                            std::vector<COO::Item> removedItems = std::vector<COO::Item>();
                            for ( COO::Item changedItem : changedItems ) {
                                if (changedItem.item_count < 0) {
                                    removedItems.push_back(changedItem);
                                } else {
                                    addedItems.push_back(changedItem);
                                }
                            }

                            std::string addedItemsString = "";
                            if (addedItems.size() == 1) {
                                addedItemsString = std::to_string(addedItems[0].item_count) + " " + addedItems[0].item_name;
                            } else if (addedItems.size() > 1) {
                                bool first_again = true;
                                for ( int y = 0; y < addedItems.size() - 1; y++ ) {
                                    if (first_again) first_again = false;
                                    else addedItemsString += ", ";
                                    addedItemsString += std::to_string(addedItems[y].item_count) + " " + addedItems[y].item_name;
                                }
                                addedItemsString += " und " + std::to_string(addedItems[addedItems.size() - 1].item_count) + " " + addedItems[addedItems.size() - 1].item_name;
                            }

                            std::string removedItemsString = "";
                            if (removedItems.size() == 1) {
                                removedItemsString = std::to_string(-removedItems[0].item_count) + " " + removedItems[0].item_name;
                            } else if (removedItems.size() > 1) {
                                bool first_again = true;
                                for ( int y = 0; y < removedItems.size() - 1; y++ ) {
                                    if (first_again) first_again = false;
                                    else removedItemsString += ", ";
                                    removedItemsString += std::to_string(-removedItems[y].item_count) + " " + removedItems[y].item_name;
                                }
                                removedItemsString += " und " + std::to_string(-removedItems[removedItems.size() - 1].item_count) + " " + removedItems[removedItems.size() - 1].item_name;
                            }

                            if (removedItems.size() > 0 && addedItems.size() <= 0) {
                                resultElements.push_back(paragraph(chest[x].getplayername() + " hat " + time_string + " " + removedItemsString + " entfernt."));
                                resultString += chest[x].getplayername() + " hat " + time_string + " " + removedItemsString + " entfernt.\n";
                            } else if (removedItems.size() <= 0 && addedItems.size() > 0) {
                                resultElements.push_back(paragraph(chest[x].getplayername() + " hat " + time_string + " " + addedItemsString + " hinzugefügt."));
                                resultString += chest[x].getplayername() + " hat " + time_string + " " + addedItemsString + " hinzugefügt.\n";
                            } else if (removedItems.size() > 0 && addedItems.size() > 0) {
                                resultElements.push_back(paragraph(chest[x].getplayername() + " hat " + time_string + " " + removedItemsString + " entfernt und " + addedItemsString + " hinzugefügt."));
                                resultString += chest[x].getplayername() + " hat " + time_string + " " + removedItemsString + " entfernt und " + addedItemsString + " hinzugefügt.\n";
                            }
                        }
                    }
                    chestNum ++;
                }
            }

            /*
            for (COO::ChestOpenObject obj : chestVector) {
                time_t obj_time = obj.getobjectTime();
                struct tm time_struct = *localtime(&obj_time);

                char hours[3];
                char minutes[3];
                char seconds[3];
                char days[3];
                char months[3];

                sprintf(hours, "%02d", time_struct.tm_hour);
                sprintf(minutes, "%02d", time_struct.tm_min);
                sprintf(seconds, "%02d", time_struct.tm_sec);
                sprintf(days, "%02d", time_struct.tm_mday);
                sprintf(months, "%02d", time_struct.tm_mon + 1);

                resultElements.push_back(text((range == 0) ?    (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() +                                                                                                                                      " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) : 
                                                                (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() + " bei X" + std::to_string(obj.getx()) + " Y" + std::to_string(obj.gety()) + " Z" + std::to_string(obj.getz()) +    " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds))));
                resultString +=               (range == 0) ?    (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() +                                                                                                                                      " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) : 
                                                                (obj.getplayername() + (obj.getbreaking() ? " zerstörte " : " baute ") + "den Block " + obj.getblockname() + " bei X" + std::to_string(obj.getx()) + " Y" + std::to_string(obj.gety()) + " Z" + std::to_string(obj.getz()) +    " am " + (std::string(days) + "." + std::string(months) + "." + std::to_string(time_struct.tm_year + 1900)) + " um " + std::string(hours) + ":" + std::string(minutes) + ":" + std::string(seconds)) + "\n";
            }
            */
            
        }
        
        selectedTab = RESULT_SCREEN_NUM;

        blockSearchScreen = &standardBlockSearchScreen;

    }
}

int main() {
    SMILEY(EM_HAPPY);
    screenFunction = startScreen;

    // ------ Setup Start screen
    std::string path;
    startscreen_error_text = text("");
    input_path = Input(&path, "Pfad");
    button_continue = Button("Continue", [&] {checkPath(path, &startscreen_error_text);});
    startScreenComponents = Container::Vertical({input_path, button_continue});

    // ------ Setup Block Search screen
    std::string blockSearchXInputString = "";
    std::string blockSearchYInputString = "";
    std::string blockSearchZInputString = "";
    std::string blockSearchRangeInputString = "0";
    blockSearchXInput = Input(&blockSearchXInputString, "");
    blockSearchYInput = Input(&blockSearchYInputString, "");
    blockSearchZInput = Input(&blockSearchZInputString, "");
    blockSearchRangeInput = Input(&blockSearchRangeInputString, "");
    blockSearchButton = Button("Suchen!", [&] {searchBlock(blockSearchXInputString, blockSearchYInputString, blockSearchZInputString, blockSearchRangeInputString);});
    blockSearchError = text("");

    // ------ Setup Chest Search screen
    std::string chestSearchXInputString = "";
    std::string chestSearchYInputString = "";
    std::string chestSearchZInputString = "";
    std::string chestSearchRangeInputString = "0";
    chestSearchXInput = Input(&chestSearchXInputString, "");
    chestSearchYInput = Input(&chestSearchYInputString, "");
    chestSearchZInput = Input(&chestSearchZInputString, "");
    chestSearchRangeInput = Input(&chestSearchRangeInputString, "");
    chestSearchButton = Button("Suchen!", [&] {searchChest(chestSearchXInputString, chestSearchYInputString, chestSearchZInputString, chestSearchRangeInputString);});
    chestSearchError = text("");

    // ------ Setup Result screen

    resultElements = Elements();
    printResultsButton = Button("txt generieren", &resultsToTxt);

    // ------ Setup Menu screen
    std::vector<std::string> tab_values {"Block Suche", "Kisten Suche", "Ergebnisse"};
    tabToggle = Toggle(&tab_values, &selectedTab);
    menuScreenComponents = Container::Vertical({
        tabToggle,
        Container::Tab({
            Container::Vertical({
                Container::Horizontal({         // Blocksearch
                    blockSearchXInput,
                    blockSearchYInput,
                    blockSearchZInput,
                    blockSearchRangeInput,
                }),
                blockSearchButton,
            }),
            Container::Vertical({
                Container::Horizontal({         // Chest search
                    chestSearchXInput,
                    chestSearchYInput,
                    chestSearchZInput,
                    chestSearchRangeInput,
                }),
                chestSearchButton,
            }),
            Container::Vertical({
                printResultsButton,
            }),
        }, &selectedTab),
    });
    component = startScreenComponents;
    menuScreenFunction = blockSearchScreen;

    
    
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    while(true) {
        Component renderer = Renderer(component, screenFunction);
        Loop loop(&screen, renderer);
        loopPtr = &loop;
        screenPtr = &screen;
        while (!loop.HasQuitted()) {
            loop.RunOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}