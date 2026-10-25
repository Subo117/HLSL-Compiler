// =======================================================================
// File: compiler/semantic.cpp
// Description: Phase 3 - Semantic Analysis (Semantic Analyzer)
// Validates types, value ranges, identifier bindings, and emits warnings/errors.
// No user-defined header files are used.
// =======================================================================

#ifndef SEMANTIC_CPP
#define SEMANTIC_CPP

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

// ---------------------------------------------------------
// Validated Data Structure passed to Code Generator
// ---------------------------------------------------------
struct ValidatedShaderData {
    string shaderName;

    // Color attributes
    string colorName;
    float colorR;
    float colorG;
    float colorB;
    float colorA;

    // Noise attributes
    bool hasNoise;
    float noiseScale;
    float noiseSpeed;

    // Effects
    float distortion;
    float emission;

    ValidatedShaderData() {
        shaderName = "DefaultShader";
        colorName = "white";
        colorR = 1.0f;
        colorG = 1.0f;
        colorB = 1.0f;
        colorA = 1.0f;

        hasNoise = false;
        noiseScale = 1.0f;
        noiseSpeed = 1.0f;

        distortion = 0.0f;
        emission = 0.0f;
    }
};

// ---------------------------------------------------------
// Semantic Analyzer Class
// ---------------------------------------------------------
class SemanticAnalyzer {
private:
    RawShaderAST ast;
    int errorCount;
    int warningCount;

    // Helper: Convert string to lowercase for case-insensitive matching
    string toLower(const string& str) {
        string res = str;
        for (size_t i = 0; i < res.length(); i++) {
            res[i] = tolower(res[i]);
        }
        return res;
    }

    // Resolves color symbol name to RGBA floating point values
    bool resolveColor(const string& rawName, float& r, float& g, float& b, float& a) {
        string name = toLower(rawName);

        if (name == "orange") {
            r = 1.0f; g = 0.5f; b = 0.0f; a = 1.0f; return true;
        } else if (name == "red") {
            r = 1.0f; g = 0.1f; b = 0.1f; a = 1.0f; return true;
        } else if (name == "green") {
            r = 0.1f; g = 0.9f; b = 0.2f; a = 1.0f; return true;
        } else if (name == "blue") {
            r = 0.1f; g = 0.4f; b = 1.0f; a = 1.0f; return true;
        } else if (name == "cyan") {
            r = 0.0f; g = 0.9f; b = 0.9f; a = 1.0f; return true;
        } else if (name == "yellow") {
            r = 1.0f; g = 0.9f; b = 0.1f; a = 1.0f; return true;
        } else if (name == "purple" || name == "magenta") {
            r = 0.7f; g = 0.1f; b = 0.9f; a = 1.0f; return true;
        } else if (name == "white") {
            r = 1.0f; g = 1.0f; b = 1.0f; a = 1.0f; return true;
        } else if (name == "black") {
            r = 0.05f; g = 0.05f; b = 0.05f; a = 1.0f; return true;
        } else if (name == "fire" || name == "gold") {
            r = 1.0f; g = 0.6f; b = 0.1f; a = 1.0f; return true;
        } else if (name == "plasma") {
            r = 0.9f; g = 0.1f; b = 0.8f; a = 1.0f; return true;
        } else if (name == "acid") {
            r = 0.3f; g = 1.0f; b = 0.1f; a = 1.0f; return true;
        }

        // Unknown color
        r = 1.0f; g = 1.0f; b = 1.0f; a = 1.0f;
        return false;
    }

public:
    SemanticAnalyzer(const RawShaderAST& inputAst) {
        ast = inputAst;
        errorCount = 0;
        warningCount = 0;
    }

    ValidatedShaderData analyze() {
        ValidatedShaderData validData;

        // 1. Check Shader Name
        if (!ast.hasShaderName || ast.shaderName.empty()) {
            cerr << "[Semantic Warning] Missing 'shader <name>' declaration. Defaulting to 'GeneratedShader'." << endl;
            validData.shaderName = "GeneratedShader";
            warningCount++;
        } else {
            validData.shaderName = ast.shaderName;
        }

        // 2. Color Symbol Resolution & Range Check
        if (ast.hasColor) {
            validData.colorName = ast.colorName;
            bool knownColor = resolveColor(ast.colorName, validData.colorR, validData.colorG, validData.colorB, validData.colorA);
            if (!knownColor) {
                cerr << "[Semantic Warning] Line " << ast.colorLine 
                     << ": Unknown color identifier '" << ast.colorName 
                     << "'. Defaulting to 'white' (1.0, 1.0, 1.0)." << endl;
                warningCount++;
            }
        }

        // 3. Noise & Scale Validation
        validData.hasNoise = ast.hasNoise;
        if (ast.hasNoise) {
            if (ast.hasScale) {
                if (ast.noiseScale <= 0.0f) {
                    cerr << "[Semantic Warning] Line " << ast.scaleLine 
                         << ": Noise scale cannot be <= 0. Clamping to default 1.0." << endl;
                    validData.noiseScale = 1.0f;
                    warningCount++;
                } else {
                    validData.noiseScale = ast.noiseScale;
                }
            } else {
                validData.noiseScale = 1.0f; // default scale
            }

            if (ast.hasSpeed) {
                if (ast.noiseSpeed < 0.0f) {
                    cerr << "[Semantic Warning] Line " << ast.speedLine 
                         << ": Negative speed detected. Taking absolute value." << endl;
                    validData.noiseSpeed = -ast.noiseSpeed;
                    warningCount++;
                } else {
                    validData.noiseSpeed = ast.noiseSpeed;
                }
            } else {
                validData.noiseSpeed = 1.0f; // default speed
            }
        }

        // 4. Distortion Range Check
        if (ast.hasDistortion) {
            if (ast.distortion < 0.0f) {
                cerr << "[Semantic Warning] Line " << ast.distortionLine 
                     << ": Distortion cannot be negative. Resetting to 0.0." << endl;
                validData.distortion = 0.0f;
                warningCount++;
            } else if (ast.distortion > 1.0f) {
                cerr << "[Semantic Notice] Line " << ast.distortionLine 
                     << ": Distortion (" << ast.distortion 
                     << ") is > 1.0; shader may appear heavily warped." << endl;
                validData.distortion = ast.distortion;
            } else {
                validData.distortion = ast.distortion;
            }
        }

        // 5. Emission Check
        if (ast.hasEmission) {
            if (ast.emission < 0.0f) {
                cerr << "[Semantic Warning] Line " << ast.emissionLine 
                     << ": Emission intensity cannot be negative. Setting to 0.0." << endl;
                validData.emission = 0.0f;
                warningCount++;
            } else {
                validData.emission = ast.emission;
            }
        }

        return validData;
    }
};

#endif // SEMANTIC_CPP
