#pragma once

#include "km_types.h"
#include "km_lib.h"
#include "gui.h"

struct FilterEntry;

typedef void (*FilterUpdateFunc)(FilterEntry*, Vec3,
    RectGL, TextGL, const FontFace&,
    Vec2, int, KeyEvent*, uint32);

typedef void (*FilterApplyFunc)(FilterEntry*, SharedState*);

struct FilterEntry
{
    int idx;
    const char* name;
    FilterUpdateFunc updateFunc;
    FilterApplyFunc applyFunc;
    Button removeButton;
    void* data; // data struct, malloc'd
};

void FilterUpdateModelLoad(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize);

// Button press (add to filter list)
void FilterButtonNone(Button* button, void* data);

void FilterButtonSelect(Button* button, void* data);

void FilterButtonTranslate(Button* button, void* data);
void FilterButtonRotate(Button* button, void* data);
void FilterButtonScale(Button* button, void* data);

void FilterButtonTwist(Button* button, void* data);
void FilterButtonInflate(Button* button, void* data);
void FilterButtonWacky(Button* button, void* data);

void FilterButtonNoise(Button* button, void* data);
void FilterButtonULSmooth(Button* button, void* data);
void FilterButtonUSharpen(Button* button, void* data);
void FilterButtonCFLSmooth(Button* button, void* data);
void FilterButtonSDmooth(Button* button, void* data);

void FilterButtonTriangulate(Button* button, void* data);
void FilterButtonTruncate(Button* button, void* data);

void FilterButtonTriangleTopology(Button* button, void* data);
void FilterButtonLoopSubdivision(Button* button, void* data);

// Filter actions (bread & butter)
struct ModelLoadData
{
    InputField inputField;
};
void FilterModelLoad(FilterEntry* entry, SharedState* state);

struct SelectData
{
    InputField inputField;
};

struct SingleUIntData
{
    InputField inputValue;
    uint32 value;
};
struct SingleFloatData
{
    InputField inputValue;
    float value;
};
struct UIntFloatData
{
    InputField inputFields[2];
    uint32 uintValue;
    float floatValue;
};
struct Vec3Data
{
    InputField inputCoords[3];
    Vec3 vector;
};

struct FilterInfo {
    const char* name;
    ButtonCallback callback;
} filterInfo_[] = {
    { "Select",             FilterButtonSelect },
    { "",                   FilterButtonNone },
    { "Translate",          FilterButtonTranslate },
    { "Rotate",             FilterButtonRotate },
    { "Scale",              FilterButtonScale },
    { "",                   FilterButtonNone },
    { "Twist",              FilterButtonTwist },
    { "Inflate",            FilterButtonInflate },
    { "Wacky",              FilterButtonWacky },
    { "",                   FilterButtonNone },
    { "Noise",              FilterButtonNoise },
    { "U.L. Smooth",        FilterButtonULSmooth },
    { "U. Sharpen",         FilterButtonUSharpen },
    { "",                   FilterButtonNone },
    { "Triangulate",        FilterButtonTriangulate },
    { "Truncate",           FilterButtonTruncate },
    { "",                   FilterButtonNone },
    { "Triangle Topology",  FilterButtonTriangleTopology },
    //{ "Loop Subdivision",   FilterButtonLoopSubdivision },
};

const int numFilters_ = sizeof(filterInfo_) / sizeof(filterInfo_[0]);

global_var DynamicArray<FilterEntry> filters_;
global_var DynamicArray<int> filtersToDelete_;