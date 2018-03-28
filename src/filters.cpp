#include "filters.h"

#define FILTER_BOX_WIDTH 200.0f

// ========== Filter implementations ==========
void FilterModelLoad(FilterEntry* entry, SharedState* state)
{
    ModelLoadData* data = (ModelLoadData*)entry->data;

    FreeHalfEdgeMesh(&state->mesh);
    state->mesh = HalfEdgeMeshFromObj(data->inputField.text);
    if (state->mesh.vertices.size == 0) {
        printf("ERROR: model not loaded\n");
        return;
    }

    //ComputeVertexAvgEdgeLengths(&state->mesh);

    printf("Loaded model: %s\n", data->inputField.text);
}

void FilterSelect(FilterEntry* entry, SharedState* state)
{
    SelectData* data = (SelectData*)entry->data;

    state->selectedVerts.Clear();
    if (strcmp(data->inputField.text, "ALL") != 0) {
        char selectStr[INPUT_BUFFER_SIZE];
        uint32 len = (uint32)strnlen(data->inputField.text,
            INPUT_BUFFER_SIZE - 1);
        strncpy(selectStr, data->inputField.text, len);
        selectStr[len] = '\0';

        int iStr = 0;
        bool done = false;
        char* number = selectStr;
        while (!done) {
            if (selectStr[iStr] == ',' || selectStr[iStr] == '\0') {
                if (selectStr[iStr] == '\0') {
                    done = true;
                }

                selectStr[iStr] = '\0';
                uint32 v = (uint32)strtol(number, nullptr, 10);
                state->selectedVerts.Append(v);
                number = &selectStr[iStr + 1];
            }

            iStr++;
        }
    }

    printf("Selected: %s\n", data->inputField.text);
}

void FilterTranslate(FilterEntry* entry, SharedState* state)
{
    Vec3Data* data = (Vec3Data*)entry->data;
    Vec3 offset = data->vector;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        state->mesh.vertices[v].pos += offset;
    }

    printf("Translated by (%f, %f, %f)\n", offset.x, offset.y, offset.z);
}
void FilterRotate(FilterEntry* entry, SharedState* state)
{
    Vec3Data* data = (Vec3Data*)entry->data;
    Vec3 rot = data->vector;
    Quat rotQuat =
        QuatFromAngleUnitAxis(rot.z, Vec3::unitZ)
        * QuatFromAngleUnitAxis(rot.y, Vec3::unitY)
        * QuatFromAngleUnitAxis(rot.x, Vec3::unitX);

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        state->mesh.vertices[v].pos = rotQuat * state->mesh.vertices[v].pos;
    }

    printf("Rotated by (%f, %f, %f)\n", rot.x, rot.y, rot.z);
}
void FilterScale(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float scale = data->value;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        state->mesh.vertices[v].pos *= scale;
    }

    printf("Scaled by %f\n", scale);
}

void FilterTwist(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float value = data->value;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        float y = state->mesh.vertices[v].pos.y;
        Quat rot = QuatFromAngleUnitAxis(y * value, Vec3::unitY);
        state->mesh.vertices[v].pos = rot * state->mesh.vertices[v].pos;
    }

    printf("Twisted by %f\n", value);
}

void FilterInflate(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float value = data->value;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        state->mesh.vertices[v].pos += state->mesh.vertices[v].normal
            * state->mesh.vertices[v].avgEdgeLength * value;
    }

    printf("Inflated/Deflated by %f\n", value);
}

void FilterWacky(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float mag = data->value;
    float period = 0.25f;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        Vec3 pos = state->mesh.vertices[v].pos;
        float offset = mag * sinf(pos.y * 2.0f * PI_F / period);
        Vec3 newPos = { pos.x * (1.0f + offset), pos.y, pos.z };
        state->mesh.vertices[v].pos = newPos;
    }

    printf("Wackyness level: %f\n", mag);
}

void FilterNoise(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float value = data->value;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        float random = (float)rand() / RAND_MAX;
        random = (random * 2.0f - 1.0f) * value;
        state->mesh.vertices[v].pos += state->mesh.vertices[v].normal
            * state->mesh.vertices[v].avgEdgeLength * random;
    }

    printf("Applied noise amount %f\n", value);
}

void FilterULSmooth(FilterEntry* entry, SharedState* state)
{
    const uint32 MAX_ITERS = 100000;
    UIntFloatData* data = (UIntFloatData*)entry->data;
    uint32 iterations = data->uintValue;
    if (iterations > MAX_ITERS) {
        iterations = MAX_ITERS;
    }
    float32 delta = data->floatValue;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    DynamicArray<uint32> verts;
    for (uint32 it = 0; it < iterations; it++) {
        HalfEdgeMesh meshCopy = CopyHalfEdgeMesh(state->mesh);
        for (uint32 i = 0; i < vertices.size; i++) {
            uint32 v = vertices[i];
            Vec3 avg = Vec3::zero;
            VerticesOnVertex(meshCopy, v, verts);
            for (uint32 j = 0; j < verts.size; j++) {
                avg += meshCopy.vertices[verts[j]].pos;
            }
            avg /= (float32)verts.size;

            Vec3 offset = (avg - meshCopy.vertices[v].pos) * delta;
            state->mesh.vertices[v].pos += offset;

            verts.Clear();
        }

        FreeHalfEdgeMesh(&meshCopy);
    }

    verts.Free();
    printf("U.L. Smooth, iter: %d, delta: %f\n", iterations, delta);
}

void FilterUSharpen(FilterEntry* entry, SharedState* state)
{
    const uint32 MAX_ITERS = 100000;
    UIntFloatData* data = (UIntFloatData*)entry->data;
    uint32 iterations = data->uintValue;
    if (iterations > MAX_ITERS) {
        iterations = MAX_ITERS;
    }
    float32 delta = data->floatValue;

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }

    DynamicArray<uint32> verts;
    for (uint32 it = 0; it < iterations; it++) {
        HalfEdgeMesh meshCopy = CopyHalfEdgeMesh(state->mesh);
        for (uint32 i = 0; i < vertices.size; i++) {
            uint32 v = vertices[i];
            Vec3 avg = Vec3::zero;
            VerticesOnVertex(meshCopy, v, verts);
            for (uint32 j = 0; j < verts.size; j++) {
                avg += meshCopy.vertices[verts[j]].pos;
            }
            avg /= (float32)verts.size;

            Vec3 offset = (avg - meshCopy.vertices[v].pos) * delta;
            state->mesh.vertices[v].pos -= offset;

            verts.Clear();
        }

        FreeHalfEdgeMesh(&meshCopy);
    }

    verts.Free();
    printf("U. Sharpen, iter: %d, delta: %f\n", iterations, delta);
}

void FilterTriangulate(FilterEntry* entry, SharedState* state)
{
    TriangulateMesh(&state->mesh); // Implemented in halfedge.cpp

    printf("Triangulated mesh\n");
}

void FilterTruncate(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float param = data->value;
    if (param < 0.0f || param > 0.5f) {
        printf("ERROR: Truncate parameter not in range [0, 0.5]\n");
        return;
    }

    DynamicArray<uint32> vertices = state->selectedVerts;
    if (state->selectedVerts.size == 0) {
        vertices = state->allVerts;
    }
    vertices = vertices.Copy();

    /*DynamicArray<uint32> verticesNew = vertices.Copy();
    for (uint32 i = 0; i < verticesNew.size; i++) {
        for (uint32 j = i + 1; j < verticesNew.size; j++) {
            if (verticesNew[j] > verticesNew[i]) {
                verticesNew[i]--;
            }
        }
    }
    HalfEdgeMesh meshCopy = CopyHalfEdgeMesh(state->mesh);*/

    DynamicArray<uint32> edges;
    DynamicArray<uint32> newVerts;
    for (uint32 i = 0; i < vertices.size; i++) {
        uint32 v = vertices[i];
        EdgesOnVertex(state->mesh, v, edges);
        for (uint32 j = 0; j < edges.size; j++) {
            newVerts.Append(SplitEdgeMakeVertex(&state->mesh, edges[j], param));
        }

        Face newFace;
        newFace.halfEdge = edges[0];

        uint32 lastTwinNext = state->mesh.halfEdges[edges[0]].next;
        for (uint32 j = 0; j < edges.size; j++) {
            uint32 prevInd = j - 1;
            if (j == 0) {
                prevInd = edges.size - 1;
            }
            uint32 nextInd = (j + 1) % edges.size;
            uint32 twin = state->mesh.halfEdges[edges[j]].twin;
            uint32 twinFace = state->mesh.halfEdges[twin].face;

            // Modify incoming edge
            state->mesh.halfEdges[twin].next =
                state->mesh.halfEdges[edges[nextInd]].next;
            state->mesh.halfEdges[twin].vertex = newVerts[nextInd];
            // Ensure twin face points to an appropriate half edge
            state->mesh.faces[twinFace].halfEdge = twin;

            // Modify outgoing edge
            state->mesh.halfEdges[edges[j]].next = edges[prevInd];
            state->mesh.halfEdges[edges[j]].face = state->mesh.faces.size;
        }
        uint32 lastTwin = state->mesh.halfEdges[edges[edges.size - 1]].twin;
        state->mesh.halfEdges[lastTwin].next = lastTwinNext;

        state->mesh.faces.Append(newFace);
        RemoveVertex(&state->mesh, v);
        for (uint32 j = i + 1; j < vertices.size; j++) {
            if (vertices[j] > v) {
                vertices[j]--;
            }
        }

        edges.Clear();
        newVerts.Clear();
    }

    edges.Free();
    newVerts.Free();
    
    vertices.Free();

    //PrintHalfEdgeMeshFaces(state->mesh);

    ComputeFaceNormals(&state->mesh);
    ComputeVertexNormals(&state->mesh);
    ComputeVertexAvgEdgeLengths(&state->mesh);

    printf("Truncated with parameter %f\n", param);
}

void FilterTriangleTopology(FilterEntry* entry, SharedState* state)
{
    SingleUIntData* data = (SingleUIntData*)entry->data;
    uint32 iterations = data->value;

    TriangulateMesh(&state->mesh);

    for (uint32 it = 0; it < iterations; it++) {
        uint32 numVertsOriginal = state->mesh.vertices.size;
        uint32 numEdgesOriginal = state->mesh.halfEdges.size;
        uint32 numFacesOriginal = state->mesh.faces.size;
        bool* isSplit = (bool*)malloc(sizeof(bool) * numEdgesOriginal);
        for (uint32 e = 0; e < numEdgesOriginal; e++) {
            isSplit[e] = false;
        }
        for (uint32 e = 0; e < numEdgesOriginal; e++) {
            if (!isSplit[e] && !isSplit[state->mesh.halfEdges[e].twin]) {
                SplitEdgeMakeVertex(&state->mesh, e, 0.5f);
                isSplit[e] = true;
                isSplit[state->mesh.halfEdges[e].twin] = true;
            }
        }

        for (uint32 f = 0; f < numFacesOriginal; f++) {
            DynamicArray<uint32> edges;
            EdgesOnFace(state->mesh, f, edges);
            int prevMidVertex = -1;
            int firstMidVertex = -1;
            for (uint32 i = 0; i < edges.size; i++) {
                uint32 v = state->mesh.halfEdges[edges[i]].vertex;
                if (v >= numVertsOriginal) {
                    if (firstMidVertex == -1) {
                        firstMidVertex = (int)v;
                    }
                    if (prevMidVertex != -1) {
                        SplitFaceMakeEdge(&state->mesh, f, v, prevMidVertex);
                    }
                    prevMidVertex = (int)v;
                }
            }
            SplitFaceMakeEdge(&state->mesh, f, firstMidVertex, prevMidVertex);
        }
    }

    ComputeFaceNormals(&state->mesh);
    ComputeVertexNormals(&state->mesh);
    ComputeVertexAvgEdgeLengths(&state->mesh);

    printf("Performed triangle topology, iterations: %d\n", iterations);
}
void FilterLoopSubdivision(FilterEntry* entry, SharedState* state)
{
    //printf("loop subdivision\n");
}

internal void DrawFilterBase(FilterEntry* entry, Vec3 pos, Vec2 size,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState)
{
    DrawRect(rectGL,
        pos, Vec2::zero, size,
        { 1.0f, 1.0f, 1.0f, 0.2f });
    Vec3 titlePos = pos;
    titlePos.y += size.y;
    DrawText(textGL, font, entry->name, titlePos,
        { 0.0f, 0.95f }, { 1.0f, 1.0f, 1.0f, 0.6f });

    if (entry->idx == -1) {
        // No remove button on ModelLoad filter
        return;
    }

    entry->removeButton.box.origin = {
        pos.x + size.x - (float32)font.height,
        pos.y + size.y - (float32)font.height
    };
    entry->removeButton.box.size = {
        (float32)font.height,
        (float32)font.height
    };
    UpdateButtons(&entry->removeButton, 1, mousePos, clickState, entry);
    DrawButtons(&entry->removeButton, 1, rectGL, textGL, font);
}

void FilterUpdateModelLoad(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    ModelLoadData* data = (ModelLoadData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    data->inputField.box.origin = pos2D;
    data->inputField.box.origin.y += 4.0f;
    data->inputField.box.size = { FILTER_BOX_WIDTH, (float32)font.height };
    UpdateInputFields(&data->inputField, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputField, 1, rectGL, textGL, font);
}

void FilterUpdateSelect(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    SelectData* data = (SelectData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    data->inputField.box.origin = pos2D;
    data->inputField.box.origin.y += 4.0f;
    data->inputField.box.size = { FILTER_BOX_WIDTH, (float32)font.height };
    UpdateInputFields(&data->inputField, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputField, 1, rectGL, textGL, font);
}

internal void FilterUpdateNoArgs(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        1.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);
}

internal void FilterUpdateSingleUInt(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    SingleUIntData* data = (SingleUIntData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    Vec2 inputSize = { size.x, (float32)font.height };
    data->inputValue.box.origin = pos2D;
    data->inputValue.box.size = inputSize;
    UpdateInputFields(&data->inputValue, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputValue, 1, rectGL, textGL, font);

    data->value = (uint32)strtol(data->inputValue.text, nullptr, 10);
}

internal void FilterUpdateSingleFloat(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    SingleFloatData* data = (SingleFloatData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    Vec2 inputSize = { size.x, (float32)font.height };
    data->inputValue.box.origin = pos2D;
    data->inputValue.box.size = inputSize;
    UpdateInputFields(&data->inputValue, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputValue, 1, rectGL, textGL, font);

    data->value = (float32)strtod(data->inputValue.text, nullptr);
}

internal void FilterUpdateUIntFloat(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    UIntFloatData* data = (UIntFloatData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    for (int i = 0; i < 2; i++) {
        Vec2 inputPos = pos2D;
        inputPos.x += i * size.x / 2.0f;
        Vec2 inputSize = { size.x / 2.0f, (float32)font.height };
        data->inputFields[i].box.origin = inputPos;
        data->inputFields[i].box.size = inputSize;
    }
    UpdateInputFields(data->inputFields, 2, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(data->inputFields, 2, rectGL, textGL, font);

    data->uintValue = (uint32)strtol(data->inputFields[0].text, nullptr, 10);
    data->floatValue = (float32)strtod(data->inputFields[1].text, nullptr);
}

internal void FilterUpdateVec3(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    Vec3Data* data = (Vec3Data*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    for (int i = 0; i < 3; i++) {
        Vec2 inputPos = pos2D;
        inputPos.x += i * size.x / 3.0f;
        Vec2 inputSize = { size.x / 3.0f, (float32)font.height };
        data->inputCoords[i].box.origin = inputPos;
        data->inputCoords[i].box.size = inputSize;
    }
    UpdateInputFields(data->inputCoords, 3, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(data->inputCoords, 3, rectGL, textGL, font);

    for (int i = 0; i < 3; i++) {
        data->vector.e[i] =
            (float32)strtod(data->inputCoords[i].text, nullptr);
    }
}

internal void RemoveButtonPress(Button* button, void* data)
{
    FilterEntry* entry = (FilterEntry*)data;
    filtersToDelete_.Append(entry->idx);
}

internal FilterEntry FilterButtonBase(Button* button)
{
    FilterEntry entry;
    entry.idx = filters_.size;
    entry.name = button->text;

    entry.removeButton = CreateButton(
        Vec2::zero, Vec2::zero,
        "X", RemoveButtonPress,
        { 1.0f, 0.2f, 0.2f, 0.2f },
        { 1.0f, 0.2f, 0.2f, 0.4f },
        { 1.0f, 0.2f, 0.2f, 0.5f },
        { 1.0f, 0.7f, 0.7f, 1.0f }
    );

    return entry;
}

void FilterButtonNone(Button* button, void* data)
{
    printf("Filter not implemented (%s)\n", button->text);
}

internal void FilterCreateNoArgs(Button* button, void* data,
    FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateNoArgs;
    entry.applyFunc = filterFunc;

    entry.data = nullptr;

    filters_.Append(entry);
}

internal void FilterCreateSingleUInt(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateSingleUInt;
    entry.applyFunc = filterFunc;

    SingleUIntData* d =
        (SingleUIntData*)malloc(sizeof(SingleUIntData));
    d->inputValue = CreateInputField(
        Vec2::zero, Vec2::zero,
        initVal,
        { 1.0f, 1.0f, 1.0f, 0.2f },
        { 1.0f, 1.0f, 1.0f, 0.4f },
        { 1.0f, 1.0f, 1.0f, 0.5f },
        { 1.0f, 1.0f, 1.0f, 0.9f }
    );

    entry.data = (void*)d;

    filters_.Append(entry);
}

internal void FilterCreateSingleFloat(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateSingleFloat;
    entry.applyFunc = filterFunc;

    SingleFloatData* d =
        (SingleFloatData*)malloc(sizeof(SingleFloatData));
    d->inputValue = CreateInputField(
        Vec2::zero, Vec2::zero,
        initVal,
        { 1.0f, 1.0f, 1.0f, 0.2f },
        { 1.0f, 1.0f, 1.0f, 0.4f },
        { 1.0f, 1.0f, 1.0f, 0.5f },
        { 1.0f, 1.0f, 1.0f, 0.9f }
    );

    entry.data = (void*)d;

    filters_.Append(entry);
}

internal void FilterCreateUIntFloat(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateUIntFloat;
    entry.applyFunc = filterFunc;

    UIntFloatData* d = (UIntFloatData*)malloc(sizeof(UIntFloatData));
    for (int i = 0; i < 2; i++) {
        d->inputFields[i] = CreateInputField(
            Vec2::zero, Vec2::zero,
            initVal,
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
    }

    entry.data = (void*)d;

    filters_.Append(entry);
}

internal void FilterCreateVec3(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateVec3;
    entry.applyFunc = filterFunc;

    Vec3Data* d = (Vec3Data*)malloc(sizeof(Vec3Data));
    for (int i = 0; i < 3; i++) {
        d->inputCoords[i] = CreateInputField(
            Vec2::zero, Vec2::zero,
            initVal,
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
    }

    entry.data = (void*)d;

    filters_.Append(entry);
}

void FilterButtonSelect(Button* button, void* data)
{
    SharedState* state = (SharedState*)data;
    char selectionStr[INPUT_BUFFER_SIZE];
    int iStr = 0;
    for (uint32 i = 0; i < state->selectedVerts.size; i++) {
        iStr += sprintf(selectionStr + iStr, "%d", state->selectedVerts[i]);
        if (i != state->selectedVerts.size - 1) {
            iStr += sprintf(selectionStr + iStr, ", ");
        }
    }
    //sprintf(selectionStr + iStr, "\0");

    if (state->selectedVerts.size == 0) {
        sprintf(selectionStr, "ALL");
    }

    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateSelect;
    entry.applyFunc = FilterSelect;

    SelectData* d = (SelectData*)malloc(sizeof(SelectData));
    d->inputField = CreateInputField(
        Vec2::zero, Vec2::zero,
        selectionStr,
        { 1.0f, 1.0f, 1.0f, 0.2f },
        { 1.0f, 1.0f, 1.0f, 0.4f },
        { 1.0f, 1.0f, 1.0f, 0.5f },
        { 1.0f, 1.0f, 1.0f, 0.9f }
    );

    entry.data = (void*)d;

    filters_.Append(entry);
}

void FilterButtonTranslate(Button* button, void* data)
{
    FilterCreateVec3(button, data, "0", FilterTranslate);
}
void FilterButtonRotate(Button* button, void* data)
{
    FilterCreateVec3(button, data, "0", FilterRotate);
}
void FilterButtonScale(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "1", FilterScale);
}

void FilterButtonTwist(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0.2", FilterTwist);
}
void FilterButtonInflate(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0", FilterInflate);
}
void FilterButtonWacky(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0", FilterWacky);
}

void FilterButtonNoise(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0", FilterNoise);
}
void FilterButtonULSmooth(Button* button, void* data)
{
    FilterCreateUIntFloat(button, data, "0", FilterULSmooth);
}
void FilterButtonUSharpen(Button* button, void* data)
{
    FilterCreateUIntFloat(button, data, "0", FilterUSharpen);
}

void FilterButtonTriangulate(Button* button, void* data)
{
    FilterCreateNoArgs(button, data, FilterTriangulate);
}
void FilterButtonTruncate(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0", FilterTruncate);
}

void FilterButtonTriangleTopology(Button* button, void* data)
{
    FilterCreateSingleUInt(button, data, "1", FilterTriangleTopology);
}
void FilterButtonLoopSubdivision(Button* button, void* data)
{
    FilterCreateNoArgs(button, data, FilterLoopSubdivision);
}