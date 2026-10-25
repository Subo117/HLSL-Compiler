// =======================================================================
// File: compiler/lexer.cpp
// Description: Phase 1 - Lexical Analysis (Tokenizer)
// Converts raw source code into a list of Tokens.
// No user-defined header files are used.
// =======================================================================

#ifndef LEXER_CPP
#define LEXER_CPP

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// ---------------------------------------------------------
// 1. Token Types
// ---------------------------------------------------------
enum TokenType {
    TOKEN_SHADER,       // "shader"
    TOKEN_COLOR,        // "color"
    TOKEN_NOISE,        // "noise"
    TOKEN_SCALE,        // "scale"
    TOKEN_SPEED,        // "speed"
    TOKEN_DISTORTION,   // "distortion"
    TOKEN_EMISSION,     // "emission"
    TOKEN_COLON,        // ":"
    TOKEN_IDENTIFIER,   // Variable names or color names (e.g., "Fire", "orange")
    TOKEN_NUMBER,       // Numeric values (e.g., "5", "0.2")
    TOKEN_EOF,          // End of file
    TOKEN_UNKNOWN       // Unknown character / error
};

// ---------------------------------------------------------
// 2. Token Structure
// ---------------------------------------------------------
struct Token {
    TokenType type;
    string text;
    int line;
};

// Helper function to get token type as a readable string
string tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_SHADER:     return "TOKEN_SHADER";
        case TOKEN_COLOR:      return "TOKEN_COLOR";
        case TOKEN_NOISE:      return "TOKEN_NOISE";
        case TOKEN_SCALE:      return "TOKEN_SCALE";
        case TOKEN_SPEED:      return "TOKEN_SPEED";
        case TOKEN_DISTORTION: return "TOKEN_DISTORTION";
        case TOKEN_EMISSION:   return "TOKEN_EMISSION";
        case TOKEN_COLON:      return "TOKEN_COLON";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:     return "TOKEN_NUMBER";
        case TOKEN_EOF:        return "TOKEN_EOF";
        default:               return "TOKEN_UNKNOWN";
    }
}

// ---------------------------------------------------------
// 3. Lexer Class
// ---------------------------------------------------------
class Lexer {
private:
    string source;
    int index;
    int line;

public:
    Lexer(string src) {
        source = src;
        index = 0;
        line = 1;
    }

    // Peek current character without consuming
    char peek() {
        if (index >= (int)source.length()) {
            return '\0';
        }
        return source[index];
    }

    // Advance to next character
    char advance() {
        if (index >= (int)source.length()) {
            return '\0';
        }
        char c = source[index];
        index++;
        return c;
    }

    // Skip spaces, tabs, newlines, and comments (# or //)
    void skipWhitespaceAndComments() {
        while (index < (int)source.length()) {
            char c = peek();

            // Track line numbers
            if (c == '\n') {
                line++;
                advance();
            }
            // Skip whitespace
            else if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            }
            // Skip comments starting with '#'
            else if (c == '#') {
                while (peek() != '\n' && peek() != '\0') {
                    advance();
                }
            }
            // Skip comments starting with '//'
            else if (c == '/' && index + 1 < (int)source.length() && source[index + 1] == '/') {
                while (peek() != '\n' && peek() != '\0') {
                    advance();
                }
            }
            else {
                break;
            }
        }
    }

    // Read full numeric token (integers & floating points)
    Token readNumber() {
        int tokenLine = line;
        string numStr = "";
        bool hasDecimal = false;

        while (isdigit(peek()) || (peek() == '.' && !hasDecimal)) {
            if (peek() == '.') {
                hasDecimal = true;
            }
            numStr += advance();
        }

        Token t;
        t.type = TOKEN_NUMBER;
        t.text = numStr;
        t.line = tokenLine;
        return t;
    }

    // Read word (keywords or identifiers)
    Token readWord() {
        int tokenLine = line;
        string word = "";

        while (isalnum(peek()) || peek() == '_') {
            word += advance();
        }

        Token t;
        t.line = tokenLine;
        t.text = word;

        // Check if word is a keyword
        if (word == "shader") {
            t.type = TOKEN_SHADER;
        } else if (word == "color") {
            t.type = TOKEN_COLOR;
        } else if (word == "noise") {
            t.type = TOKEN_NOISE;
        } else if (word == "scale") {
            t.type = TOKEN_SCALE;
        } else if (word == "speed") {
            t.type = TOKEN_SPEED;
        } else if (word == "distortion") {
            t.type = TOKEN_DISTORTION;
        } else if (word == "emission") {
            t.type = TOKEN_EMISSION;
        } else {
            t.type = TOKEN_IDENTIFIER;
        }

        return t;
    }

    // Main tokenization method
    vector<Token> tokenize() {
        vector<Token> tokens;

        while (index < (int)source.length()) {
            skipWhitespaceAndComments();

            if (index >= (int)source.length()) {
                break;
            }

            char c = peek();

            // Colon symbol
            if (c == ':') {
                advance();
                Token t;
                t.type = TOKEN_COLON;
                t.text = ":";
                t.line = line;
                tokens.push_back(t);
            }
            // Numbers
            else if (isdigit(c) || (c == '.' && index + 1 < (int)source.length() && isdigit(source[index + 1]))) {
                tokens.push_back(readNumber());
            }
            // Identifiers / Keywords
            else if (isalpha(c) || c == '_') {
                tokens.push_back(readWord());
            }
            // Unknown symbol
            else {
                Token t;
                t.type = TOKEN_UNKNOWN;
                t.text = string(1, advance());
                t.line = line;
                tokens.push_back(t);
            }
        }

        // EOF Token
        Token eofToken;
        eofToken.type = TOKEN_EOF;
        eofToken.text = "EOF";
        eofToken.line = line;
        tokens.push_back(eofToken);

        return tokens;
    }
};

#endif // LEXER_CPP
