#include "halfedge.h"

#include <stdio.h>
#include <map>

#include "main.h"
#include "km_math.h"
#include "ogl_base.h"

#define OBJ_LINE_MAX 512

Vec3 ParseVec3(char* str)
{
    Vec3 result = { 0.0f, 0.0f, 0.0f };
    char* el = str;
    int i = 0;
    for (char* c = str; *c != '\0'; c++) {
        if (*c == ' ' || *c == '\n') {
            char* endptr = el;
            *c = '\0';
            result.e[i++] = (float32)strtod(el, &endptr);
            // TODO check error
            el = c + 1;
        }
    }

    return result;
}

HalfEdgeMesh HalfEdgeMeshFromObj(const char* fileName)
{
    const char* MODEL_DIR = "data/models/";

    HalfEdgeMesh mesh;

    char filePath[PATH_MAX_LENGTH];
    char fullPath[PATH_MAX_LENGTH];
    CatStrings(MODEL_DIR, fileName, filePath, PATH_MAX_LENGTH);
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open OBJ file at: %s\n", filePath);
        return mesh;
    }

    char line[OBJ_LINE_MAX];
    DynamicArray<int> faceInds;

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vertex vertex;
                vertex.pos = ParseVec3(&line[2]);
                vertex.halfEdge = 0; // This is set later
                vertex.color = Vec3::one;
                mesh.vertices.Append(vertex);
            }
            else if (line[1] == 'n') {
                printf("vertex normal! UNHANDLED\n");
            }
            else {
                // idk
            }
        }
        else if (line[0] == 'f') {
            char* el = &line[2];
            for (char* c = &line[2]; *c != '\0'; c++) {
                if (*c == ' ' || *c == '\n') {
                    char* endptr = el;
                    *c = '\0';
                    faceInds.Append((int)strtol(el, &endptr, 10));
                    // TODO check error
                    el = c + 1;
                }
            }
            faceInds.Append(-1);
        }
        else {
            // idk
        }
    }

    std::map<std::pair<uint32, uint32>, uint32> edgeMap;

    uint32 faceVerts = 0;
    for (uint32 i = 0; i < faceInds.size; i++) {
        if (faceInds[i] == -1 || i == faceInds.size - 1) {
            // Create face
            Face face;
            face.halfEdge = mesh.halfEdges.size - 1;
            mesh.faces.Append(face);
            
            faceVerts = 0;
            continue;
        }

        uint32 vertSrc = faceInds[i] - 1;
        uint32 vertDst = faceInds[i + 1] - 1;
        if (faceInds[i + 1] == -1) {
            vertDst = faceInds[i - faceVerts] - 1;
        }

        // Create edge, i -> i + 1
        // Last edge will have wrong "next"
        std::pair<int, int> forward(vertSrc, vertDst);
        std::pair<int, int> backward(vertDst, vertSrc);
        HalfEdge he;
        he.next = mesh.halfEdges.size + 1;
        if (faceInds[i + 1] == -1) {
            he.next = mesh.halfEdges.size - faceVerts;
        }
        auto edgeBackward = edgeMap.find(backward);
        if (edgeBackward != edgeMap.end()) {
            he.twin = edgeBackward->second;
            mesh.halfEdges[edgeBackward->second].twin =
                mesh.halfEdges.size;
        }
        else {
            he.twin = 0;
        }
        he.vertex = vertDst;
        he.face = mesh.faces.size;

        // Assign forward edge i -> i + 1 to vertex i
        mesh.vertices[vertSrc].halfEdge = mesh.halfEdges.size;

        // Add edge to edgeMap and mesh.halfEdges array
        auto res = edgeMap.insert(
            std::make_pair(forward, mesh.halfEdges.size));
        if (!res.second) {
            printf("ERROR: Edge already in edgeMap\n");
        }
        mesh.halfEdges.Append(he);
        faceVerts++;
    }

    faceInds.Free();
    ComputeFaceNormals(&mesh);
    ComputeVertexNormals(&mesh);
    ComputeVertexAvgEdgeLengths(&mesh);
    //PrintHalfEdgeMesh(mesh);

    // TODO free mesh after this
    return mesh;
}

HalfEdgeMesh CopyHalfEdgeMesh(const HalfEdgeMesh& mesh)
{
    HalfEdgeMesh newMesh;
    newMesh.vertices = mesh.vertices.Copy();
    newMesh.faces = mesh.faces.Copy();
    newMesh.halfEdges = mesh.halfEdges.Copy();

    return newMesh;
}

void FreeHalfEdgeMesh(HalfEdgeMesh* mesh)
{
    mesh->vertices.Free();
    mesh->faces.Free();
    mesh->halfEdges.Free();
}

void PrintHalfEdgeMesh(const HalfEdgeMesh& mesh)
{
    printf("----- VERTICES -----\n");
    for (uint32 i = 0; i < mesh.vertices.size; i++) {
        printf("%d: (%f, %f, %f) ; edge %d\n",
            i,
            mesh.vertices[i].pos.x,
            mesh.vertices[i].pos.y,
            mesh.vertices[i].pos.z,
            mesh.vertices[i].halfEdge);
    }
    printf("----- HALF EDGES -----\n");
    for (uint32 i = 0; i < mesh.halfEdges.size; i++) {
        printf("%d: next %d, twin: %d, vert %d, face %d\n",
            i,
            mesh.halfEdges[i].next, mesh.halfEdges[i].twin,
            mesh.halfEdges[i].vertex,
            mesh.halfEdges[i].face);
    }
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.faces.size; i++) {
        printf("%d: edge %d\n",
            i,
            mesh.faces[i].halfEdge);
    }
}

void PrintEdgeEndpoints(const HalfEdgeMesh& mesh, uint32 e)
{
    DynamicArray<uint32> verts;
    VerticesOnEdge(mesh, e, verts);
    printf("%d -> %d", verts[0], verts[1]);
}

void PrintHalfEdgeMeshFaces(const HalfEdgeMesh& mesh)
{
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.faces.size; i++) {
        printf("Face %d vertices: ", i);
        uint32 edge = mesh.faces[i].halfEdge;
        uint32 e = edge;
        do {
            printf("%d (e %d: ", mesh.halfEdges[e].vertex, e);
            PrintEdgeEndpoints(mesh, e);
            printf("), ");
            e = mesh.halfEdges[e].next;
        } while (e != edge);
        printf("\n");
    }
}

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh, bool smoothNormals)
{
    HalfEdgeMesh meshTri = CopyHalfEdgeMesh(mesh);
    TriangulateMesh(&meshTri);
    HalfEdgeMeshGL meshGL = {};

    // TODO use indexing for this
    DynamicArray<Vec3> vertices;
    DynamicArray<Vec3> normals;
    DynamicArray<Vec3> colors;

    for (uint32 f = 0; f < meshTri.faces.size; f++) {
        Vec3 normal = meshTri.faces[f].normal;
        uint32 edge = meshTri.faces[f].halfEdge;
        uint32 e = edge;
        do {
            Vertex v = meshTri.vertices[meshTri.halfEdges[e].vertex];
            vertices.Append(v.pos);
            if (smoothNormals) {
                normal = v.normal;
            }
            normals.Append(normal);
            colors.Append(v.color);
            e = meshTri.halfEdges[e].next;
        } while (e != edge);
    }

    glGenVertexArrays(1, &meshGL.vertexArray);
    glBindVertexArray(meshGL.vertexArray);

    glGenBuffers(1, &meshGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size * sizeof(Vec3),
        vertices.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glGenBuffers(1, &meshGL.normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size * sizeof(Vec3),
        normals.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glGenBuffers(1, &meshGL.colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size * sizeof(Vec3),
        colors.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    meshGL.programID = LoadShaders(
        "shaders/model.vert",
        "shaders/model.frag");

    meshGL.vertexCount = (int)vertices.size;

    return meshGL;
}

void FreeHalfEdgeMeshGL(HalfEdgeMeshGL* meshGL)
{
    glDeleteBuffers(1, &meshGL->vertexBuffer);
    glDeleteBuffers(1, &meshGL->normalBuffer);
    glDeleteBuffers(1, &meshGL->colorBuffer);
    glDeleteProgram(meshGL->programID);
    glDeleteVertexArrays(1, &meshGL->vertexArray);
}

void DrawHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL, Mat4 proj, Mat4 view)
{
    GLint loc;
    glUseProgram(meshGL.programID);
    
    Mat4 model = Mat4::one;
    Mat4 mvp = proj * view * model;
    loc = glGetUniformLocation(meshGL.programID, "mvp");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp.e[0][0]);
    loc = glGetUniformLocation(meshGL.programID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &model.e[0][0]);
    loc = glGetUniformLocation(meshGL.programID, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &view.e[0][0]);

    glBindVertexArray(meshGL.vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, meshGL.vertexCount);
    glBindVertexArray(0);
}

// Utility functions
void RemoveVertex(HalfEdgeMesh* mesh, uint32 v)
{
    for (uint32 e = 0; e < mesh->halfEdges.size; e++) {
        if (mesh->halfEdges[e].vertex > v) {
            mesh->halfEdges[e].vertex--;
        }
        else if (mesh->halfEdges[e].vertex == v) {
            printf("ERROR: Removed vertex still in use in mesh (%d)\n", v);
            printf("edge: "); PrintEdgeEndpoints(*mesh, e);
            printf("\n");
        }
    }
    mesh->vertices.Remove(v);
}

// Make a new vertex in halfEdge e. t is a value in the range [0, 1]
// that specifies where in the edge v1 -> v2 the new vertex should be created.
// Returns the new vertex index.
uint32 SplitEdgeMakeVertex(HalfEdgeMesh* mesh, uint32 e, float t)
{
    if (t < 0.0f || t > 1.0f) {
        printf("ERROR: SplitEdgeMakeVertex t not in [0, 1]\n");
        return 0;
    }

    uint32 twin = mesh->halfEdges[e].twin;
    uint32 v1 = mesh->halfEdges[twin].vertex;
    uint32 v2 = mesh->halfEdges[e].vertex;

    // Create new vertex
    Vertex v;
    v.halfEdge = mesh->halfEdges.size;
    v.pos = Lerp(mesh->vertices[v1].pos, mesh->vertices[v2].pos, t);
    v.color = Lerp(mesh->vertices[v1].color, mesh->vertices[v2].color, t);

    // Create new halfEdges
    HalfEdge edgeVOut;
    edgeVOut.next = mesh->halfEdges[e].next;
    edgeVOut.twin = mesh->halfEdges.size + 1;
    edgeVOut.vertex = mesh->halfEdges[e].vertex;
    edgeVOut.face = mesh->halfEdges[e].face;
    HalfEdge edgeVIn;
    edgeVIn.next = twin;
    edgeVIn.twin = mesh->halfEdges.size;
    edgeVIn.vertex = mesh->vertices.size;
    edgeVIn.face = mesh->halfEdges[twin].face;

    // Update original halfEdges
    DynamicArray<uint32> edgesFromV2;
    EdgesOnVertex((const HalfEdgeMesh&)*mesh, v2, edgesFromV2);
    for (uint32 i = 0; i < edgesFromV2.size; i++) {
        uint32 edgeToV2 = mesh->halfEdges[edgesFromV2[i]].twin;
        if (mesh->halfEdges[edgeToV2].next == twin) {
            mesh->halfEdges[edgeToV2].next = mesh->halfEdges.size + 1;
        }
    }
    mesh->halfEdges[e].next = mesh->halfEdges.size;
    mesh->halfEdges[e].vertex = mesh->vertices.size;

    // Ensure the endpoints still have valid halfEdge pointers.
    mesh->vertices[v1].halfEdge = e;
    mesh->vertices[v2].halfEdge = mesh->halfEdges.size + 1;

    mesh->vertices.Append(v);
    mesh->halfEdges.Append(edgeVOut);
    mesh->halfEdges.Append(edgeVIn);

    edgesFromV2.Free();
    return mesh->vertices.size - 1;
}

// Make a new edge v1 <-> v2, splitting face f.
// f will now be v1 -> ... -> v2 -> v1
// new face will be v2 -> ... -> v1 -> v2
// Returns the new face index.
uint32 SplitFaceMakeEdge(HalfEdgeMesh* mesh, uint32 f, uint32 v1, uint32 v2)
{
    uint32 edge = mesh->faces[f].halfEdge;
    uint32 eToV1 = edge;
    while (mesh->halfEdges[eToV1].vertex != v1) {
        eToV1 = mesh->halfEdges[eToV1].next;
    }
    uint32 eToV2 = edge;
    while (mesh->halfEdges[eToV2].vertex != v2) {
        eToV2 = mesh->halfEdges[eToV2].next;
    }

    uint32 e = eToV2;
    while (mesh->halfEdges[e].vertex != v1) {
        e = mesh->halfEdges[e].next;
        // Assign new face to edges v2 -> ... -> v1
        mesh->halfEdges[e].face = mesh->faces.size;
    }

    // Update original face f
    mesh->faces[f].halfEdge = mesh->halfEdges.size + 1; // v2 -> v1
    // Create new face
    Face newFace;
    newFace.halfEdge = mesh->halfEdges.size; // v1 -> v2

    // Create new half edges
    HalfEdge v1v2;
    v1v2.next = mesh->halfEdges[eToV2].next;
    v1v2.twin = mesh->halfEdges.size + 1;
    v1v2.vertex = v2;
    v1v2.face = mesh->faces.size;
    HalfEdge v2v1;
    v2v1.next = mesh->halfEdges[eToV1].next;
    v2v1.twin = mesh->halfEdges.size;
    v2v1.vertex = v1;
    v2v1.face = f;

    // Connect endpoints to new half edges
    mesh->halfEdges[eToV1].next = mesh->halfEdges.size;
    mesh->halfEdges[eToV2].next = mesh->halfEdges.size + 1;

    mesh->halfEdges.Append(v1v2);
    mesh->halfEdges.Append(v2v1);
    mesh->faces.Append(newFace);

    return mesh->faces.size - 1;
}

void TriangulateMesh(HalfEdgeMesh* mesh)
{
    for (uint32 i = 0; i < mesh->faces.size; i++) {
        while (true) {
            uint32 numEdges = 0;
            uint32 edge = mesh->faces[i].halfEdge;
            uint32 e = edge;
            uint32 ePrev = e;
            do {
                numEdges++;
                if (numEdges > 3) {
                    SplitFaceMakeEdge(mesh, i, mesh->halfEdges[ePrev].vertex,
                        mesh->halfEdges[edge].vertex);
                    break;
                }
                ePrev = e;
                e = mesh->halfEdges[e].next;
            } while (e != edge);

            if (numEdges == 3) {
                break;
            }
        }
    }

    ComputeFaceNormals(mesh);
    ComputeVertexNormals(mesh);
    ComputeVertexAvgEdgeLengths(mesh);
    //printf("==> TRIANGULATION FINISHED\n");
    //PrintHalfEdgeMesh((const HalfEdgeMesh&)*mesh);
}

internal Vec3 NormalFromTriangle(const Vec3 triangle[3])
{
    Vec3 a = triangle[1] - triangle[0];
    Vec3 b = triangle[2] - triangle[0];
    return Normalize(Cross(a, b));
}
// Computes normal for the face f, assuming all vertices in f are coplanar.
internal Vec3 ComputeFaceNormal(const HalfEdgeMesh& mesh, uint32 f)
{
    Vec3 triangle[3];
    int numEdges = 0;
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        triangle[numEdges++] = mesh.vertices[mesh.halfEdges[e].vertex].pos;
        if (numEdges >= 3) {
            break;
        }
        e = mesh.halfEdges[e].next;
    } while (e != edge);

    return NormalFromTriangle(triangle);
}
void ComputeFaceNormals(HalfEdgeMesh* mesh)
{
    for (uint32 f = 0; f < mesh->faces.size; f++) {
        Vec3 normal = ComputeFaceNormal((const HalfEdgeMesh&)*mesh, f);
        mesh->faces[f].normal = normal;
    }
}

internal float32 ComputeTriangleArea(Vec3 triangle[3])
{
    Vec3 a = triangle[1] - triangle[0];
    Vec3 b = triangle[2] - triangle[0];
    float magA = Mag(a);
    float magB = Mag(b);
    float cosTheta = Dot(a, b) / (magA * magB);
    float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
    
    return magA * magB * sinTheta / 2.0f;
}
void ComputeFaceAreas(HalfEdgeMesh* mesh)
{
    Vec3 triangle[3];

    for (uint32 f = 0; f < mesh->faces.size; f++) {
        float32 faceArea = 0.0f;

        uint32 edge = mesh->faces[f].halfEdge;
        uint32 e = edge;
        triangle[0] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
        e = mesh->halfEdges[e].next;
        triangle[1] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
        do {
            e = mesh->halfEdges[e].next;
            triangle[2] = mesh->vertices[mesh->halfEdges[e].vertex].pos;
            faceArea += ComputeTriangleArea(triangle);
        } while (e != edge);

        mesh->faces[f].area = faceArea;
    }
}

void ComputeVertexNormals(HalfEdgeMesh* mesh)
{
    DynamicArray<uint32> faces;
    for (uint32 v = 0; v < mesh->vertices.size; v++) {
        Vec3 sumNormals = Vec3::zero;
        FacesOnVertex((const HalfEdgeMesh&)*mesh, v, faces);
        for (uint32 i = 0; i < faces.size; i++) {
            sumNormals += mesh->faces[faces[i]].normal;
        }
        mesh->vertices[v].normal = Normalize(sumNormals);

        faces.Clear();
    }

    faces.Free();
}

void ComputeVertexAvgEdgeLengths(HalfEdgeMesh* mesh)
{
    DynamicArray<uint32> verts;
    for (uint32 v = 0; v < mesh->vertices.size; v++) {
        float sumDists = 0.0f;
        VerticesOnVertex((const HalfEdgeMesh&)*mesh, v, verts);
        for (uint32 i = 0; i < verts.size; i++) {
            Vec3 edgeVec = mesh->vertices[verts[i]].pos - mesh->vertices[v].pos;
            sumDists += Mag(edgeVec);
        }
        mesh->vertices[v].avgEdgeLength = sumDists / (float32)verts.size;
        //mesh->vertices[v].color.r = mesh->vertices[v].avgEdgeLength * 12.0f;

        verts.Clear();
    }

    verts.Free();
}

// Source:
// https://stackoverflow.com/questions/27589796/check-point-within-polygon?rq=1
internal bool PointInsidePolygon(Vec2 p, const DynamicArray<Vec2>& polygon)
{
    int crossNumber = 0;

    for (uint32 i = 0; i < polygon.size; i++) {
        uint32 next = (i + 1) % polygon.size;
        // If there's an upward or downward crossing
        if (((polygon[i].y <= p.y) && (polygon[next].y > p.y))
        || ((polygon[i].y > p.y) && (polygon[next].y <= p.y))) {
            float vt = (float)(p.y - polygon[i].y)
                / (polygon[next].y - polygon[i].y);
            if (p.x < polygon[i].x + vt * (polygon[next].x - polygon[i].x)) {
                crossNumber++;
            }
        }
    }

    return crossNumber % 2 == 1;
}

void MouseCastMeshFaces(const HalfEdgeMesh& mesh, Vec2 mousePos,
    Mat4 proj, Mat4 view,
    int screenWidth, int screenHeight,
    DynamicArray<MouseCastFaceOut>& out)
{
    Mat4 toScreen = HomogeneousToScreen();

    DynamicArray<Vec2> verts;
    for (uint32 f = 0; f < mesh.faces.size; f++) {
        uint32 edge = mesh.faces[f].halfEdge;
        uint32 e = edge;
        float32 avgZ = 0.0f;
        do {
            Vec3 vWorld = mesh.vertices[mesh.halfEdges[e].vertex].pos;
            Vec4 vWorld4 = { vWorld.x, vWorld.y, vWorld.z, 1.0f };
            Vec4 vScreen4 = proj * view * vWorld4;
            vScreen4 /= vScreen4.w;
            vScreen4 = toScreen * vScreen4;
            Vec3 vScreen = { vScreen4.x, vScreen4.y, vScreen4.z };
            verts.Append({ vScreen.x, vScreen.y });
            avgZ += vScreen.z;

            e = mesh.halfEdges[e].next;
        } while (e != edge);
        avgZ /= (float32)verts.size;

        if (PointInsidePolygon(mousePos, verts)) {
            MouseCastFaceOut faceOut = { f, avgZ };
            out.Append(faceOut);
        }

        verts.Clear();
    }
}

// Mesh traversal functions
void VerticesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        out.Append(mesh.halfEdges[e].vertex);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}
void EdgesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        out.Append(e);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}
void FacesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.faces[f].halfEdge;
    uint32 e = edge;
    do {
        uint32 twin = mesh.halfEdges[e].twin;
        out.Append(mesh.halfEdges[twin].face);
        e = mesh.halfEdges[e].next;
    } while (e != edge);
}

void VerticesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<uint32>& out)
{
    uint32 twin = mesh.halfEdges[e].twin;
    out.Append(mesh.halfEdges[twin].vertex);
    out.Append(mesh.halfEdges[e].vertex);
}
void FacesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<uint32>& out)
{
    uint32 twin = mesh.halfEdges[e].twin;
    out.Append(mesh.halfEdges[e].face);
    out.Append(mesh.halfEdges[twin].face);
}

void VerticesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.vertices[v].halfEdge;
    uint32 e = edge;
    do {
        out.Append(mesh.halfEdges[e].vertex);
        uint32 twin = mesh.halfEdges[e].twin;
        e = mesh.halfEdges[twin].next;
    } while (e != edge);
}
void EdgesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.vertices[v].halfEdge;
    uint32 e = edge;
    do {
        out.Append(e);
        uint32 twin = mesh.halfEdges[e].twin;
        e = mesh.halfEdges[twin].next;
    } while (e != edge);
}
void FacesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<uint32>& out)
{
    uint32 edge = mesh.vertices[v].halfEdge;
    uint32 e = edge;
    do {
        out.Append(mesh.halfEdges[e].face);
        uint32 twin = mesh.halfEdges[e].twin;
        e = mesh.halfEdges[twin].next;
    } while (e != edge);
}