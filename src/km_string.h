#pragma once

int StringLength(const char* string);
void CatStrings(
    size_t sourceACount, const char* sourceA,
    size_t sourceBCount, const char* sourceB,
    size_t destCount, char* dest);
inline bool32 IsWhitespace(char c);