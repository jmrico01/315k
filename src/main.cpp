#include "main.h"

#include <km_common/km_debug.h>
#include <km_common/km_defines.h>
#include <km_common/km_input.h>
#include <km_common/km_log.h>
#include <km_common/km_math.h>
#include <km_common/km_memory.h>
#include <km_common/km_string.h>
#include <km_platform/main_platform.h>
#undef internal
#include <random>
#define internal static
// #include <fftw3.h>

#include "opengl.h"
#include "opengl_funcs.h"
#include "opengl_base.h"
#include "load_png.h"
#include "particles.h"
#include "load_wav.h"

// These must match the max sizes in blur.frag
#define KERNEL_HALFSIZE_MAX 10
#define KERNEL_SIZE_MAX (KERNEL_HALFSIZE_MAX * 2 + 1)

global_var Vec4 backgroundColor_ = Vec4 { 0.05f, 0.05f, 0.05f, 0.0f };
global_var Vec4 backgroundColorBeat_ = Vec4 { 0.1f, 0.1f, 0.1f, 0.0f };
global_var Vec4 lineColor_ = Vec4 { 0.9f, 0.9f, 0.9f, 1.0f };
global_var Vec4 lineDarkColor_ = Vec4 { 0.25f, 0.25f, 0.25f, 1.0f };

//global_var Vec4 circleIdleColor_ = lineColor_;
global_var Vec4 circleSelectedColor_ = Vec4 { 0.6f, 1.0f, 0.7f, 1.0f };

global_var Vec4 snareHitColor_ = Vec4 { 1.0f, 0.7f, 0.6f, 0.5f };
global_var Vec4 noteColor_ = Vec4 { 0.6f, 1.0f, 0.7f, 0.5f };

inline float32 RandFloat32()
{
	return (float32)rand() / RAND_MAX;
}
inline float32 RandFloat32(float32 min, float32 max)
{
	DEBUG_ASSERT(max > min);
	return RandFloat32() * (max - min) + min;
}

template <typename Allocator>
void MarkerGL::Init(Allocator* allocator)
{
	const GLfloat vertices[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
		-0.5f, -0.5f
	};
    
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
    
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
                          0, // match shader layout location
                          2, // size (vec2)
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void*)0 // array buffer offset
                          );
    
	glBindVertexArray(0);
    
	programID = LoadShaders(allocator, "shaders/marker.vert", "shaders/marker.frag");
}

void MarkerGL::Draw(const ScreenInfo& screenInfo, Vec2Int pos, Vec2Int size, Vec4 color)
{
	RectCoordsNDC ndc = ToRectCoordsNDC(pos, size, screenInfo);
    
	GLint loc;
	glUseProgram(programID);
	loc = glGetUniformLocation(programID, "posCenter");
	glUniform3fv(loc, 1, &ndc.pos.e[0]);
	loc = glGetUniformLocation(programID, "size");
	glUniform2fv(loc, 1, &ndc.size.e[0]);
	loc = glGetUniformLocation(programID, "color");
	glUniform4fv(loc, 1, &color.e[0]);
    
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

template <typename Allocator>
void CircleGL::Init(Allocator* allocator)
{
	const GLfloat vertices[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
		-0.5f, -0.5f
	};
    
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
    
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
                          0, // match shader layout location
                          2, // size (vec2)
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0, // stride
                          (void*)0 // array buffer offset
                          );
    
	glBindVertexArray(0);
    
	programID = LoadShaders(allocator, "shaders/circle.vert", "shaders/circle.frag");
}

void CircleGL::Draw(const ScreenInfo& screenInfo, Vec2Int pos, Vec2Int size, Vec4 color)
{
	RectCoordsNDC ndc = ToRectCoordsNDC(pos, size, screenInfo);
    
	GLint loc;
	glUseProgram(programID);
	loc = glGetUniformLocation(programID, "posCenter");
	glUniform3fv(loc, 1, &ndc.pos.e[0]);
	loc = glGetUniformLocation(programID, "size");
	glUniform2fv(loc, 1, &ndc.size.e[0]);
	loc = glGetUniformLocation(programID, "color");
	glUniform4fv(loc, 1, &color.e[0]);
    
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

struct ParticleDeathData {
	GameState* gameState;
	int circleDiameter;
	ScreenInfo screenInfo;
};

internal void InitParticleDeath(ParticleSystem* ps, Particle* particle,
                                void* data)
{
	ParticleDeathData* pdd = (ParticleDeathData*)data;
	GameState* gameState = pdd->gameState;
	float32 slotWidthPix = pdd->screenInfo.size.x / 12.0f;
	Vec3 origin = {
		slotWidthPix * gameState->circlePos + slotWidthPix / 2.0f,
		pdd->screenInfo.size.y / 2.0f,
		0.0f
	};
	float32 radius = pdd->circleDiameter / 2.0f;
    
	particle->life = 0.0f;
	Vec3 circlePos;
	do {
		circlePos.x = RandFloat32(-1.0f, 1.0f);
		circlePos.y = RandFloat32(-1.0f, 1.0f);
		circlePos.z = 0.0f;
	} while (Mag(circlePos) > 1.0f);
	circlePos *= radius;
	particle->pos = origin + circlePos;
    
	//float32 speedX = RandFloat32(-20.0f, 20.0f);
	//float32 speedY = RandFloat32(-400.0f, 400.0f);
	float32 speedXVar = slotWidthPix / 8.0f;
	float32 speedYVar = slotWidthPix;
	std::random_device rd;
	// Mersenne twister PRNG, initialized with seed from previous random device instance
	std::mt19937 gen(rd());
	std::normal_distribution<float32> distX(0.0f, speedXVar);
	std::normal_distribution<float32> distY(0.0f, speedYVar);
	float32 speedX = distX(gen);
	float32 speedY = distY(gen);
	particle->vel = speedX * Vec3::unitX + speedY * Vec3::unitY;
	float32 colorRandT = RandFloat32();
	particle->color = Lerp(circleSelectedColor_, snareHitColor_, colorRandT);
	float32 baseSize = pdd->circleDiameter / 10.0f;
	baseSize = MaxFloat32(baseSize, 2.0f);
	float32 randSize = baseSize * RandFloat32(0.5f, 1.5f);
	particle->size = { randSize, randSize };
	particle->bounceMult = 1.0f;
	particle->frictionMult = 1.0f;
}

internal void HalfBeat(GameState* gameState, ScreenInfo screenInfo)
{
	gameState->halfBeatCount++;
	if (gameState->halfBeatCount >= gameState->levelLength) {
		gameState->halfBeatCount = 0;
	}
	if (!gameState->dead) {
		for (int i = 0; i < 12; i++) {
			if (gameState->snareHits[gameState->halfBeatCount][i]) {
				gameState->audioState.soundSnare.play = true;
				if (gameState->circlePos == i) {
					gameState->dead = true;
					gameState->deadTime = 0.0f;
					gameState->deadHalfBeats = 0;
					gameState->killerHalfBeat = gameState->halfBeatCount;
                    
					ParticleDeathData pdd;
					float32 slotWidthPix = screenInfo.size.x / 12.0f;
					int circleDiameter = (int)(slotWidthPix * 0.6f);
					pdd.gameState = gameState;
					pdd.circleDiameter = circleDiameter;
					pdd.screenInfo = screenInfo;
					ParticleBurst(&gameState->ps, 1000, &pdd);
					gameState->audioState.soundDeath.play = true;
				}
			}
		}
	}
	else {
		gameState->deadHalfBeats++;
		if (gameState->deadHalfBeats >= DEATH_DURATION_HALFBEATS) {
			gameState->dead = false;
			gameState->halfBeatCount = 0;
			gameState->circlePos = gameState->respawn;
		}
	}
	if (!gameState->dead) {
		// Play note sounds after death check to avoid playing
		// note sound without rendering note rect
		for (int i = 0; i < 12; i++) {
			if (gameState->notes[gameState->halfBeatCount][i]
                && gameState->circlePos == i) {
				gameState->audioState.soundNotes[i].play = true;
			}
		}
	}
	if (gameState->halfBeatCount % 2 == 0) {
		gameState->lastBeat = 0.0f;
        
		gameState->audioState.soundKick.play = true;
	}
}

internal char* LoadLevelIntList(char* c, int outList[12], int* listSize)
{
	*listSize = 0;
	if (*c == 'n') {
		c++;
		while (IsWhitespace(*c)) {
			c++;
		}
		return c;
	}
    
	char buf[128];
	char* bufStart = buf;
	int i = 0;
	while (*c != '\n') {
		if (*c == ',') {
			buf[i++] = '\0';
			c++;
			outList[*listSize] = (int)strtol(bufStart, nullptr, 10);
			*listSize += 1;
			bufStart = &buf[i];
			while (IsWhitespace(*c)) {
				if (*c == '\n') {
					break;
				}
				c++;
			}
		}
		else {
			buf[i++] = *c++;
		}
	}
	while (IsWhitespace(*c)) {
		c++;
	}
    
	return c;
}

template <typename Allocator>
internal void LoadLevel(Allocator* allocator, const char* filePath, GameState* gameState)
{
	// Read shader code from files.
	LOG_INFO("Reading level file: %s\n", filePath);
	Array<uint8> levelFile = LoadEntireFile(ToString(filePath), allocator);
	if (levelFile.size == 0) {
		LOG_ERROR("Failed to read level file\n");
		return;
	}
    
	// TODO file reading here is obviously very sloppy.
	// Check for unexpected end of file, etc.
	char* fileData = (char*)levelFile.data;
	char* c = fileData;
	char buf[128];
    
	int i = 0;
	while (!IsWhitespace(*c)) {
		buf[i++] = *c++;
	}
	while (IsWhitespace(*c)) {
		c++;
	}
	buf[i] = '\0';
	int bpm = (int)strtol(buf, nullptr, 10);
    
	i = 0;
	while (!IsWhitespace(*c)) {
		buf[i++] = *c++;
	}
	while (IsWhitespace(*c)) {
		c++;
	}
	buf[i] = '\0';
	int respawn = (int)strtol(buf, nullptr, 10);
    
	i = 0;
	while (!IsWhitespace(*c)) {
		buf[i++] = *c++;
	}
	while (IsWhitespace(*c)) {
		c++;
	}
	buf[i] = '\0';
	int levelLength = (int)strtol(buf, nullptr, 10);
    
	// Load snare hits
	for (int hb = 0; hb < levelLength; hb++) {
		int snareHits[12];
		int snareHitCount;
		c = LoadLevelIntList(c, snareHits, &snareHitCount);
        
		for (int j = 0; j < 12; j++) {
			gameState->snareHits[hb][j] = false;
		}
		for (int j = 0; j < snareHitCount; j++) {
			gameState->snareHits[hb][snareHits[j]] = true;
		}
	}
	// Load notes
	for (int hb = 0; hb < levelLength; hb++) {
		int notes[12];
		int noteCount;
		c = LoadLevelIntList(c, notes, &noteCount);
        
		for (int j = 0; j < 12; j++) {
			gameState->notes[hb][j] = false;
		}
		for (int j = 0; j < noteCount; j++) {
			gameState->notes[hb][notes[j]] = true;
		}
	}
    
	gameState->bpm = bpm;
	gameState->respawn = respawn;
	gameState->levelLength = levelLength;
    
	gameState->halfBeatCount = 0;
	gameState->lastHalfBeat = 0.0f;
	gameState->circlePos = respawn;
}

void GameUpdateAndRender(const PlatformFunctions& platformFuncs, const GameInput& input,
                         const ScreenInfo& screenInfo, float32 deltaTime, GameMemory* memory, GameAudio* audio)
{
	// NOTE: for clarity
	// A call to this function means the following has happened:
	//  - A frame has been displayed to the user
	//  - The latest user input has been processed by the platform layer
	//
	// This function is expected to update the state of the game
	// and draw the frame that will be displayed, ideally, some constant
	// amount of time in the future.
	DEBUG_ASSERT(sizeof(GameState) <= memory->permanent.size);
	GameState *gameState = (GameState*)memory->permanent.memory;
    
	if (memory->shouldInitGlobalVariables) {
		// Initialize global function names
#define FUNC(returntype, name, ...) name = platformFuncs.glFunctions.name;
        GL_FUNCTIONS_BASE
			GL_FUNCTIONS_ALL
#undef FUNC
        
		memory->shouldInitGlobalVariables = false;
		LOG_INFO("Initialized global variables\n");
	}
    
	if (!memory->isInitialized) {
		LinearAllocator allocator(memory->transient.size, memory->transient.memory);
        
		// Very explicit depth testing setup (DEFAULT VALUES)
		// NDC is left-handed with this setup
		// (very subtle left-handedness definition:
		//  front objects have z = -1, far objects have z = 1)
		glEnable(GL_DEPTH_TEST);
		// Nearer objects have less z than farther objects
		glDepthFunc(GL_LEQUAL);
		// Depth buffer clears to farthest z-value (1)
		glClearDepth(1.0);
		// Depth buffer transforms -1 to 1 range to 0 to 1 range
		glDepthRange(0.0, 1.0);
        
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
		gameState->audioState.Init(&allocator, audio);
        
		// Game data
		gameState->bpm = 120;
		gameState->lastHalfBeat = 0.0f;
		gameState->lastBeat = 0.0f;
		gameState->halfBeatCount = 0;
        
		gameState->levelLength = 8;
		for (int i = 0; i < gameState->levelLength; i++) {
			for (int j = 0; j < 12; j++) {
				gameState->snareHits[i][j] = false;
			}
		}
		gameState->respawn = 0;
        
		gameState->circlePos = 0;
        
		LoadLevel(&allocator, "data/levels/level9", gameState);
        
		gameState->dead = false;
        
		// Rendering stuff
		gameState->rectGL.Init(&allocator);
		gameState->rectPixelGL = InitRectPixelGL(&allocator);
		gameState->texturedRectPixelGL = InitTexturedRectPixelGL(&allocator);
		gameState->lineGL = InitLineGL(&allocator);
		gameState->textGL = InitTextGL(&allocator);
		gameState->psGL = InitParticleSystemGL(&allocator);
        
		gameState->markerGL.Init(&allocator);
		gameState->circleGL.Init(&allocator);
        
		FT_Error error = FT_Init_FreeType(&gameState->ftLibrary);
		if (error) {
			LOG_ERROR("FreeType init error: %d\n", error);
		}
		gameState->fontFaceSmall = LoadFontFace(&allocator, gameState->ftLibrary,
                                                "data/fonts/ibm-plex-mono/regular.ttf", 18);
		gameState->fontFaceMedium = LoadFontFace(&allocator, gameState->ftLibrary,
                                                 "data/fonts/ibm-plex-mono/regular.ttf", 24);
        
		glGenFramebuffers(NUM_FRAMEBUFFERS, gameState->framebuffers);
		for (int i = 0; i < NUM_FRAMEBUFFERS; i++) {
			gameState->colorBuffers[i] = 0;
		}
        
		const GLfloat vertices[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f,
			-1.0f, -1.0f
		};
		const GLfloat uvs[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f
		};
        
		glGenVertexArrays(1, &gameState->screenQuadVertexArray);
		glBindVertexArray(gameState->screenQuadVertexArray);
        
		glGenBuffers(1, &gameState->screenQuadVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, gameState->screenQuadVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
                              0, // match shader layout location
                              2, // size (vec2)
                              GL_FLOAT, // type
                              GL_FALSE, // normalized?
                              0, // stride
                              (void*)0 // array buffer offset
                              );
        
		glGenBuffers(1, &gameState->screenQuadUVBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, gameState->screenQuadUVBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
                              1, // match shader layout location
                              2, // size (vec2)
                              GL_FLOAT, // type
                              GL_FALSE, // normalized?
                              0, // stride
                              (void*)0 // array buffer offset
                              );
        
		glBindVertexArray(0);
        
		gameState->screenShader = LoadShaders(&allocator,
                                              "shaders/screen.vert", "shaders/screen.frag");
		gameState->bloomExtractShader = LoadShaders(&allocator,
                                                    "shaders/screen.vert", "shaders/bloomExtract.frag");
		gameState->bloomBlendShader = LoadShaders(&allocator,
                                                  "shaders/screen.vert", "shaders/bloomBlend.frag");
		gameState->blurShader = LoadShaders(&allocator,
                                            "shaders/screen.vert", "shaders/blur.frag");
		gameState->grainShader = LoadShaders(&allocator,
                                             "shaders/screen.vert", "shaders/grain.frag");
		gameState->fxaaShader = LoadShaders(&allocator,
                                            "shaders/screen.vert", "shaders/fxaa.frag");
        
		if (!LoadPNGOpenGL(&allocator, "data/textures/base.png",
                           GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, gameState->pTexBase)) {
			DEBUG_PANIC("Failed to load base texture\n");
		}
        
		CreateParticleSystem(&gameState->ps, 10000, 0, 1.5f,
                             Vec3::zero,
                             0.05f, 0.02f,
                             nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0,
                             InitParticleDeath, gameState->pTexBase.textureID
                             );
        
		// Testing FFTW3
		// const int N = 10000;
		// LOG_INFO("testing FFTW3 with N = %d\n", N);
        
		// fftwf_complex* in = (fftwf_complex*)fftwf_malloc(
		//     sizeof(fftwf_complex) * N);
		// fftwf_complex* out = (fftwf_complex*)fftwf_malloc(
		//     sizeof(fftwf_complex) * N);
		// fftwf_plan p = fftwf_plan_dft_1d(N, in, out,
		//     FFTW_FORWARD, FFTW_ESTIMATE);
        
		// // initialize in and out arrays
        
		// fftwf_execute(p);
        
		// fftwf_destroy_plan(p);
		// fftwf_free(in);
		// fftwf_free(out);
        
		// LOG_INFO("...done!\n");
		// LOG_INFO("damn, that was fast. good thing we're in the West\n");
        
		memory->isInitialized = true;
	}
	if (screenInfo.changed) {
		if (gameState->colorBuffers[0] != 0) {
			glDeleteTextures(NUM_FRAMEBUFFERS, gameState->colorBuffers);
		}
		glGenTextures(NUM_FRAMEBUFFERS, gameState->colorBuffers);
        
		for (int i = 0; i < NUM_FRAMEBUFFERS; i++) {
			glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         screenInfo.size.x, screenInfo.size.y,
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         NULL
                         );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
			glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, gameState->colorBuffers[i], 0);
            
#if GAME_SLOW
			GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
				LOG_ERROR("Incomplete framebuffer (%d), status %x\n",
                          i, fbStatus);
			}
#endif
		}
        
		LOG_INFO("Updated screen-size dependent info\n");
	}
    
	// Level loading
	for (int i = 0; i < 10; i++) {
		if (WasKeyPressed(input, (KeyInputCode)(KM_KEY_0 + i))) {
			LinearAllocator allocator(memory->transient.size, memory->transient.memory);
			char buf[128];
			sprintf(buf, "data/levels/level%d", i);
			LoadLevel(&allocator, buf, gameState);
		}
	}
	// Toggle global mute
	if (WasKeyReleased(input, KM_KEY_M)) {
		gameState->audioState.globalMute = !gameState->audioState.globalMute;
	}
    
	if (WasKeyPressed(input, KM_KEY_A)
        || WasKeyPressed(input, KM_KEY_ARROW_LEFT)) {
		gameState->circlePos--;
	}
	if (WasKeyPressed(input, KM_KEY_D)
        || WasKeyPressed(input, KM_KEY_ARROW_RIGHT)) {
		gameState->circlePos++;
	}
	if (gameState->circlePos < 0) {
		gameState->circlePos = 0;
	}
	if (gameState->circlePos > 11) {
		gameState->circlePos = 11;
	}
    
	if (deltaTime < 1.0f) {
		// TODO very janky, but hey
		gameState->lastBeat += deltaTime;
		gameState->lastHalfBeat += deltaTime;
		if (gameState->dead) {
			gameState->deadTime += deltaTime;
		}
	}
	float32 beatDuration = 60.0f / (float32)gameState->bpm;
	float32 halfBeatDuration = 60.0f / (float32)gameState->bpm / 2.0f;
	if (gameState->lastHalfBeat >= halfBeatDuration) {
		//LOG_INFO("half beat\n");
		gameState->lastHalfBeat -= halfBeatDuration;
		HalfBeat(gameState, screenInfo);
	}
	float32 beatProgress = gameState->lastBeat / beatDuration;
	float32 halfBeatProgress = gameState->lastHalfBeat / halfBeatDuration;
    
	int lineWidth = screenInfo.size.y / 140;
	float32 slotWidthPix = screenInfo.size.x / 12.0f;
	int circleDiameter = (int)(slotWidthPix * 0.6f);
	ParticleDeathData pdd;
	pdd.gameState = gameState;
	pdd.circleDiameter = circleDiameter;
	pdd.screenInfo = screenInfo;
	UpdateParticleSystem(&gameState->ps, deltaTime, &pdd);
    
	// ------------------------- Begin Rendering -------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[0]);
	glEnable(GL_DEPTH_TEST);
	Vec4 clearColor = Lerp(backgroundColorBeat_, backgroundColor_,
                           beatProgress);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);
    
	if (gameState->dead) {
		circleDiameter = 0;
	}
    
	for (int i = 0; i < 12; i++) {
		Vec2Int slotLinePos = {
			(int)(slotWidthPix * i),
			screenInfo.size.y / 2
		};
		Vec4 color = lineColor_;
		if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) {
			// Black keys on piano
			color = lineDarkColor_;
		}
		DrawRectPixel(gameState->rectPixelGL, screenInfo,
                      slotLinePos,
                      Vec2 { 0.0f, 0.5f },
                      Vec2Int { (int)slotWidthPix, lineWidth },
                      color
                      );
	}
	for (int i = 0; i < 13; i++) {
		Vec2Int markerPos = {
			(int)(slotWidthPix * i),
			screenInfo.size.y / 2
		};
		gameState->markerGL.Draw(screenInfo,
                                 markerPos,
                                 Vec2Int { lineWidth, lineWidth * 6 },
                                 lineColor_
                                 );
	}
    
	Vec2Int circlePos = {
		(int)(slotWidthPix * gameState->circlePos + slotWidthPix / 2),
		screenInfo.size.y / 2
	};
	gameState->circleGL.Draw(screenInfo,
                             circlePos,
                             Vec2Int { circleDiameter, circleDiameter },
                             circleSelectedColor_
                             );
    
	// Draw snare hits
	if (!gameState->dead) {
		Vec4 snareHitColor = snareHitColor_;
		snareHitColor.a *= 1.0f - halfBeatProgress;
		for (int i = 0; i < 12; i++) {
			Vec2Int snareHitPos = {
				(int)(slotWidthPix * i + slotWidthPix / 2.0f),
				screenInfo.size.y / 2
			};
			if (gameState->snareHits[gameState->halfBeatCount][i]) {
				DrawRectPixel(gameState->rectPixelGL, screenInfo,
                              snareHitPos,
                              Vec2 { 0.5f, 0.5f },
                              Vec2Int { (int)(slotWidthPix * 1.01f), screenInfo.size.y },
                              snareHitColor
                              );
			}
		}
	}
	else {
		Vec4 snareHitColor = snareHitColor_;
		float32 deathProgress = gameState->deadTime
			/ (DEATH_DURATION_HALFBEATS * halfBeatDuration);
		snareHitColor.a *= MaxFloat32(1.0f - deathProgress, 0.0f);
		for (int i = 0; i < 12; i++) {
			Vec2Int snareHitPos = {
				(int)(slotWidthPix * i + slotWidthPix / 2.0f),
				screenInfo.size.y / 2
			};
			if (gameState->snareHits[gameState->killerHalfBeat][i]) {
				DrawRectPixel(gameState->rectPixelGL, screenInfo,
                              snareHitPos,
                              Vec2 { 0.5f, 0.5f },
                              Vec2Int { (int)(slotWidthPix * 1.01f), screenInfo.size.y },
                              snareHitColor
                              );
			}
		}
	}
    
	// Draw notes
	if (!gameState->dead) {
		Vec4 noteColor = noteColor_;
		noteColor.a *= 1.0f - halfBeatProgress;
		for (int i = 0; i < 12; i++) {
			Vec2Int notePos = {
				(int)(slotWidthPix * i + slotWidthPix / 2.0f),
				screenInfo.size.y / 2
			};
			if (gameState->notes[gameState->halfBeatCount][i]) {
				DrawRectPixel(gameState->rectPixelGL, screenInfo,
                              notePos,
                              Vec2 { 0.5f, 0.5f },
                              Vec2Int { (int)(slotWidthPix * 1.01f), screenInfo.size.y },
                              noteColor
                              );
			}
		}
	}
    
	Vec3 scale = {
		2.0f / screenInfo.size.x,
		2.0f / screenInfo.size.y,
		1.0f
	};
	Mat4 psView = Translate(-Vec3::one) * Scale(scale);
	DrawParticleSystem(gameState->psGL, &gameState->ps,
                       Vec3::unitX, Vec3::unitY, Vec3::unitZ, Mat4::one, psView,
                       memory->transient);
    
	// Post processing passes
	glDisable(GL_DEPTH_TEST);
	GLint loc;
	// -------------------- BLOOM --------------------
	float32 bloomThreshold = 0.25f;
	int bloomKernelHalfSize = 4;
	int bloomKernelSize = bloomKernelHalfSize * 2 + 1;
	int bloomBlurPasses = 1;
	int bloomBlurPassesDeath = 1;
	float32 bloomBlurSigma = 4.0f;
	float32 bloomBlurSigmaDeathMult = 4.0f;
	float32 bloomMag = 0.5f;
	float32 bloomMagDeathMultMax = 1.0f;
	if (gameState->dead) {
		float32 deathProgress = gameState->deadTime
			/ (DEATH_DURATION_HALFBEATS * halfBeatDuration);
		bloomBlurPasses += (int)((1.0f - deathProgress)
                                 * bloomBlurPassesDeath);
		bloomBlurSigma += (1.0f - deathProgress) * bloomBlurSigmaDeathMult;
		bloomMag += (1.0f - deathProgress) * bloomMagDeathMultMax;
	}
	// Extract high-luminance pixels
	glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[1]);
	//glClear(GL_COLOR_BUFFER_BIT);
    
	glBindVertexArray(gameState->screenQuadVertexArray);
	glUseProgram(gameState->bloomExtractShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[0]);
	loc = glGetUniformLocation(gameState->bloomExtractShader,
                               "framebufferTexture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(gameState->bloomExtractShader,
                               "threshold");
	glUniform1f(loc, bloomThreshold);
    
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	// Blur high-luminance pixels
	GLfloat gaussianKernel[KERNEL_SIZE_MAX];
	GLfloat kernSum = 0.0f;
	float32 sigma = bloomBlurSigma;
	for (int i = -bloomKernelHalfSize; i <= bloomKernelHalfSize; i++) {
		float32 x = (float32)i;
		float32 g = expf(-(x * x) / (2.0f * sigma * sigma));
		gaussianKernel[i + bloomKernelHalfSize] = (GLfloat)g;
		kernSum += (GLfloat)g;
	}
	for (int i = 0; i < bloomKernelSize; i++) {
		gaussianKernel[i] /= kernSum;
	}
	for (int i = 0; i < bloomBlurPasses; i++) {
		// Horizontal pass
		glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[2]);
		//glClear(GL_COLOR_BUFFER_BIT);
        
		glBindVertexArray(gameState->screenQuadVertexArray);
		glUseProgram(gameState->blurShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[1]);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "framebufferTexture");
		glUniform1i(loc, 0);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "isHorizontal");
		glUniform1i(loc, 1);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "gaussianKernel");
		glUniform1fv(loc, bloomKernelSize, gaussianKernel);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "kernelHalfSize");
		glUniform1i(loc, bloomKernelHalfSize);
        
		glDrawArrays(GL_TRIANGLES, 0, 6);
        
		// Vertical pass
		glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[1]);
		//glClear(GL_COLOR_BUFFER_BIT);
        
		glBindVertexArray(gameState->screenQuadVertexArray);
		glUseProgram(gameState->blurShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[2]);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "framebufferTexture");
		glUniform1i(loc, 0);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "isHorizontal");
		glUniform1i(loc, 0);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "gaussianKernel");
		glUniform1fv(loc, bloomKernelSize, gaussianKernel);
		loc = glGetUniformLocation(gameState->blurShader,
                                   "kernelHalfSize");
		glUniform1i(loc, bloomKernelHalfSize);
        
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
    
	// Blend scene with blurred bright pixels
	glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[2]);
	//glClear(GL_COLOR_BUFFER_BIT);
    
	glBindVertexArray(gameState->screenQuadVertexArray);
	glUseProgram(gameState->bloomBlendShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[0]);
	loc = glGetUniformLocation(gameState->bloomBlendShader,
                               "scene");
	glUniform1i(loc, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[1]);
	loc = glGetUniformLocation(gameState->bloomBlendShader,
                               "bloomBlur");
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(gameState->bloomBlendShader,
                               "bloomMag");
	glUniform1f(loc, bloomMag);
    
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	// -------------------- GRAIN --------------------
	float32 grainMag = 0.25f;
	if (gameState->dead) {
		float32 deathProgress = gameState->deadTime
			/ (DEATH_DURATION_HALFBEATS * halfBeatDuration);
		grainMag += (1.0f - deathProgress) * (1.0f - grainMag);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, gameState->framebuffers[0]);
	//glClear(GL_COLOR_BUFFER_BIT);
    
	glBindVertexArray(gameState->screenQuadVertexArray);
	glUseProgram(gameState->grainShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[2]);
	loc = glGetUniformLocation(gameState->grainShader,
                               "scene");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(gameState->grainShader,
                               "grainMag");
	glUniform1f(loc, 0.25f);
	loc = glGetUniformLocation(gameState->grainShader,
                               "time");
	glUniform1f(loc, gameState->lastBeat * 139754.0f); // TODO janky!
    
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	// --------------------RENDER TO SCREEN --------------------
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glBindVertexArray(gameState->screenQuadVertexArray);
	glUseProgram(gameState->screenShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gameState->colorBuffers[0]);
	loc = glGetUniformLocation(gameState->screenShader,
                               "framebufferTexture");
	glUniform1i(loc, 0);
    
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
	// -------------------------- End Rendering --------------------------
    
#if GAME_INTERNAL
	{ // DEBUG recording logic
		AudioState& audioState = gameState->audioState;
		if (WasKeyPressed(input, KM_KEY_R)) {
			audioState.debugRecording = !audioState.debugRecording;
			if (audioState.debugRecording) {
				audioState.debugBufferSamples = 0;
				audioState.debugBufferView.marks;
				audioState.debugBufferView.marks.size = 0;
				audioState.debugBufferView.ResetControls();
			}
		}
		if (WasKeyPressed(input, KM_KEY_V)) {
			audioState.debugViewRecording = !audioState.debugViewRecording;
			if (audioState.debugViewRecording) {
				audioState.debugBufferView.channels = audio->channels;
				audioState.debugBufferView.buffer = audioState.debugBuffer;
				audioState.debugBufferView.SetPosition(
                                                       Vec2Int { 50, 50 },
                                                       Vec2Int { screenInfo.size.x - 100, 400 },
                                                       Vec2 { 0.0f, 0.0f }
                                                       );
				audioState.debugBufferView.ResetControls();
			}
		}
		if (audioState.debugRecording) {
			if (audio->sampleDelta > 0 && (audioState.debugBufferSamples + audio->sampleDelta)
                * audio->channels <= DEBUG_BUFFER_SAMPLES) {
				if (audioState.debugBufferView.marks.size < BUFFERVIEW_MAX_MARKS) {
					audioState.debugBufferView.marks.Append(audioState.debugBufferSamples);
				}
				MemCopy(audioState.debugBuffer + audioState.debugBufferSamples * audio->channels,
                        audio->buffer, audio->sampleDelta * audio->channels * sizeof(float32));
				audioState.debugBufferSamples += audio->sampleDelta;
			}
		}
		if (audioState.debugViewRecording) {
			audioState.debugBufferView.numSamples = audioState.debugBufferSamples;
			audioState.debugBufferView.UpdateAndDraw(input, screenInfo,
                                                     gameState->rectGL, gameState->rectPixelGL, gameState->lineGL,
                                                     gameState->textGL, gameState->fontFaceSmall,
                                                     audio->sampleRate, memory->transient);
		}
	}
#endif
    
	OutputAudio(audio, gameState, input, memory->transient);
    
	{
		char fpsStr[128];
		sprintf(fpsStr, "FPS: %f", 1.0f / deltaTime);
		Vec2Int fpsPos = {
			screenInfo.size.x - 10,
			screenInfo.size.y - 10,
		};
		DrawText(gameState->textGL, gameState->fontFaceMedium, screenInfo,
                 fpsStr, fpsPos, Vec2 { 1.0f, 1.0f }, Vec4::one, memory->transient);
	}
    
#if GAME_INTERNAL
	if (gameState->audioState.debugView) {
		const uint64 STRING_BUFFER_MAX_LENGTH = 128;
		char strBuf[STRING_BUFFER_MAX_LENGTH];
		Vec2Int audioInfoStride = {
			0,
			-((int)gameState->fontFaceSmall.height + 6)
		};
		Vec2Int audioInfoPos = {
			10,
			screenInfo.size.y - 10,
		};
		DrawText(gameState->textGL, gameState->fontFaceSmall, screenInfo,
                 "Audio Engine", audioInfoPos, Vec2 { 0.0f, 1.0f },
                 circleSelectedColor_,
                 memory->transient
                 );
		snprintf(strBuf, STRING_BUFFER_MAX_LENGTH,
                 "Sample Rate: %d", audio->sampleRate);
		audioInfoPos += audioInfoStride;
		DrawText(gameState->textGL, gameState->fontFaceSmall, screenInfo,
                 strBuf, audioInfoPos, Vec2 { 0.0f, 1.0f },
                 Vec4::one,
                 memory->transient
                 );
		snprintf(strBuf, STRING_BUFFER_MAX_LENGTH,
                 "Channels: %d", audio->channels);
		audioInfoPos += audioInfoStride;
		DrawText(gameState->textGL, gameState->fontFaceSmall, screenInfo,
                 strBuf, audioInfoPos, Vec2 { 0.0f, 1.0f },
                 Vec4::one,
                 memory->transient
                 );
        
		snprintf(strBuf, STRING_BUFFER_MAX_LENGTH,
                 "Active Voices: %d", gameState->audioState.waveTable.activeVoices);
		audioInfoPos += audioInfoStride * 2;
		DrawText(gameState->textGL, gameState->fontFaceSmall, screenInfo,
                 strBuf, audioInfoPos, Vec2 { 0.0f, 1.0f },
                 Vec4::one,
                 memory->transient
                 );
        
		if (input.arduinoIn.connected) {
			snprintf(strBuf, STRING_BUFFER_MAX_LENGTH,
                     "Arduino Channel: %u", input.arduinoIn.activeChannel);
			audioInfoPos += audioInfoStride;
			DrawText(gameState->textGL, gameState->fontFaceSmall, screenInfo,
                     strBuf, audioInfoPos, Vec2 { 0.0f, 1.0f },
                     Vec4::one,
                     memory->transient
                     );
		}
	}
#endif
    
#if GAME_SLOW
	// Catch-all site for OpenGL errors
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		LOG_ERROR("OpenGL error: %x\n", err);
	}
#endif
}

#include "audio.cpp"
#include "gui.cpp"
#include "load_png.cpp"
#include "load_wav.cpp"
#include "opengl_base.cpp"
#include "particles.cpp"
#include "text.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION

#include <km_common/km_input.cpp>
#include <km_common/km_lib.cpp>
#include <km_common/km_log.cpp>
#include <km_common/km_memory.cpp>
#include <km_common/km_os.cpp>
#include <km_common/km_string.cpp>

#if GAME_WIN32
#include <km_platform/win32_main.cpp>
#include <km_platform/win32_audio.cpp>
#include "win32_arduino.cpp"
#else
// TODO other platforms...
#error "Unsupported platform"
#endif
