#ifndef KEYITEM_H
#define KEYITEM_H
#include <vector>
using std::vector;



struct TKeyItem {
    //TKeyItem(int ID, const char* graphic, const char* subscript);
    TKeyItem(int ID, int code, int code_y, int code_x, const char* graphic, const char* subscript, const char* label,vector<int>);

    //int fRow;
    //int fColumn;
    const char* fGraphic; // TODO:
    const char* fSubscript;
    const char* fSuperLabel; // label on top


    //int tag=0;
    int code_y;
    int code_x;
    vector<int> sdl_keys;
};

#endif // KEYPADUNT_H
