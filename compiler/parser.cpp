// =======================================================================
// File: compiler/parser.cpp
// Description: Phase 2 - Syntax Analysis (Parser)
// Parses token stream according to grammar rules and outputs a RawShaderAST.
// No user-defined header files are used.
// =======================================================================

#ifndef PARSER_CPP
#define PARSER_CPP

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

// ---------------------------------------------------------
// 1. Raw Abstract Syntax Tree / Parsed Syntax Data
// ---------------------------------------------------------
struct RawShaderAST {
    string shaderName;
    bool hasShaderName;
    int shaderNameLine;

    string colorName;
    bool hasColor;
    int colorLine;

    bool hasNoise;
    int noiseLine;

    bool hasScale;
    float noiseScale;
    int scaleLine;

    bool hasSpeed;
    float noiseSpeed;
    int speedLine;

    bool hasDistortion;
    float distortion;
    int distortionLine;

    bool hasEmission;
    float emission;
    int emissionLine;

    RawShaderAST() {
        shaderName = "";
        hasShaderName = false;
        shaderNameLine = 0;

        colorName = "white";
        hasColor = false;
        colorLine = 0;

        hasNoise = false;
        noiseLine = 0;

        hasScale = false;
        noiseScale = 1.0f;
        scaleLine = 0;

        hasSpeed = false;
        noiseSpeed = 1.0f;
        speedLine = 0;

        hasDistortion = false;
        distortion = 0.0f;
        distortionLine = 0;

        hasEmission = false;
        emission = 0.0f;
        emissionLine = 0;
    }
};

// ---------------------------------------------------------
// 2. Parser Class
// ---------------------------------------------------------
class Parser {
private:
    vector<Token> tokens;
    int current;

public:
    Parser(const vector<Token>& inputTokens) {
        tokens = inputTokens;
        current = 0;
    }

    bool isAtEnd() {
        return peek().type == TOKEN_EOF;
    }

    Token peek() {
        if (current < (int)tokens.size()) {
            return tokens[current];
        }
        Token eofToken;
        eofToken.type = TOKEN_EOF;
        eofToken.text = "EOF";
        eofToken.line = -1;
        return eofToken;
    }

    Token previous() {
        return tokens[current - 1];
    }

    Token advance() {
        if (!isAtEnd()) {
            current++;
        }
        return previous();
    }

    bool check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    Token consume(TokenType type, string errorMessage) {
        if (check(type)) {
            return advance();
        }

        cerr << "[Syntax Error] Line " << peek().line 
             << ": " << errorMessage << " (Found '" << peek().text << "')" << endl;
        exit(1);
    }

    // Main grammar parsing routine
    RawShaderAST parse() {
        RawShaderAST ast;

        // Check for 'shader <Name>'
        if (match(TOKEN_SHADER)) {
            Token nameToken = consume(TOKEN_IDENTIFIER, "Expected shader name identifier after 'shader' keyword.");
            ast.shaderName = nameToken.text;
            ast.hasShaderName = true;
            ast.shaderNameLine = nameToken.line;
        }

        // Parse remaining properties until EOF
        while (!isAtEnd()) {
            // color <name>
            if (match(TOKEN_COLOR)) {
                Token colToken = consume(TOKEN_IDENTIFIER, "Expected color name identifier after 'color'.");
                ast.colorName = colToken.text;
                ast.hasColor = true;
                ast.colorLine = colToken.line;
            }
            // noise:
            else if (match(TOKEN_NOISE)) {
                ast.hasNoise = true;
                ast.noiseLine = previous().line;

                if (check(TOKEN_COLON)) {
                    advance();
                }

                // Check nested scale and speed within noise block
                while (check(TOKEN_SCALE) || check(TOKEN_SPEED)) {
                    if (match(TOKEN_SCALE)) {
                        Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'scale'.");
                        ast.noiseScale = (float)atof(num.text.c_str());
                        ast.hasScale = true;
                        ast.scaleLine = num.line;
                    } else if (match(TOKEN_SPEED)) {
                        Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'speed'.");
                        ast.noiseSpeed = (float)atof(num.text.c_str());
                        ast.hasSpeed = true;
                        ast.speedLine = num.line;
                    }
                }
            }
            // Standalone scale <number>
            else if (match(TOKEN_SCALE)) {
                Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'scale'.");
                ast.noiseScale = (float)atof(num.text.c_str());
                ast.hasScale = true;
                ast.hasNoise = true;
                ast.scaleLine = num.line;
            }
            // Standalone speed <number>
            else if (match(TOKEN_SPEED)) {
                Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'speed'.");
                ast.noiseSpeed = (float)atof(num.text.c_str());
                ast.hasSpeed = true;
                ast.hasNoise = true;
                ast.speedLine = num.line;
            }
            // distortion <number>
            else if (match(TOKEN_DISTORTION)) {
                Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'distortion'.");
                ast.distortion = (float)atof(num.text.c_str());
                ast.hasDistortion = true;
                ast.distortionLine = num.line;
            }
            // emission <number>
            else if (match(TOKEN_EMISSION)) {
                Token num = consume(TOKEN_NUMBER, "Expected numeric value after 'emission'.");
                ast.emission = (float)atof(num.text.c_str());
                ast.hasEmission = true;
                ast.emissionLine = num.line;
            }
            // Gracefully ignore lonely colons
            else if (match(TOKEN_COLON)) {
                // Ignore
            }
            else {
                Token unk = advance();
                cerr << "[Syntax Warning] Line " << unk.line 
                     << ": Unrecognized token '" << unk.text << "' ignored." << endl;
            }
        }

        return ast;
    }
};

#endif // PARSER_CPP
