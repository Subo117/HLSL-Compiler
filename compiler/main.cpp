// =======================================================================
// File: compiler/main.cpp
// Description: Main Compiler Driver for mysh (HLSL Converter)
// Orchestrates the 4 compiler phases sequentially:
//   1. Lexical Analysis   (lexer.cpp)
//   2. Syntax Analysis    (parser.cpp)
//   3. Semantic Analysis  (semantic.cpp)
//   4. Code Generation    (codegen.cpp)
// No user-defined header files are used.
// =======================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <direct.h> // for _mkdir
#else
#include <sys/stat.h>
#endif

// Include compiler phases directly (no user-defined .h header files)
#include "lexer.cpp"
#include "parser.cpp"
#include "semantic.cpp"
#include "codegen.cpp"

using namespace std;

// Helper: Read entire source file into string
string readFile(const string& filePath) {
    ifstream inFile(filePath.c_str());
    if (!inFile.is_open()) {
        cerr << "[Error] Could not open source file: " << filePath << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

// Helper: Ensure output directory exists
void ensureOutputDirectory(const string& dirName) {
#ifdef _WIN32
    _mkdir(dirName.c_str());
#else
    mkdir(dirName.c_str(), 0777);
#endif
}

// Helper: Write generated HLSL to output path
void writeOutputFile(const string& outputPath, const string& content) {
    ofstream outFile(outputPath.c_str());
    if (!outFile.is_open()) {
        cerr << "[Error] Could not create output file: " << outputPath << endl;
        exit(1);
    }
    outFile << content;
    outFile.close();
}

int main(int argc, char* argv[]) {
    cout << "======================================================" << endl;
    cout << "             mysh HLSL Compiler Pipeline              " << endl;
    cout << "======================================================" << endl;

    // Check command line arguments
    if (argc < 2) {
        cout << "Usage: mysh <path_to_file.mysh>" << endl;
        cout << "Example: mysh inputs/Fire.mysh" << endl;
        return 1;
    }

    string inputFilePath = argv[1];
    cout << "[Source File] " << inputFilePath << endl;
    string sourceCode = readFile(inputFilePath);

    // ---------------------------------------------------------
    // Phase 1: Lexical Analysis (Lexer)
    // ---------------------------------------------------------
    cout << "\n>>> Phase 1: Lexical Analysis (Lexer)..." << endl;
    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.tokenize();
    cout << "  [SUCCESS] Token stream created (" << tokens.size() << " tokens)." << endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type != TOKEN_EOF) {
            cout << "    Token: " << tokenTypeToString(tokens[i].type) 
                 << " -> '" << tokens[i].text << "' (Line " << tokens[i].line << ")" << endl;
        }
    }

    // ---------------------------------------------------------
    // Phase 2: Syntax Analysis (Parser)
    // ---------------------------------------------------------
    cout << "\n>>> Phase 2: Syntax Analysis (Parser)..." << endl;
    Parser parser(tokens);
    RawShaderAST rawAst = parser.parse();
    cout << "  [SUCCESS] Syntax verified and Abstract Syntax Tree (AST) generated." << endl;
    cout << "    Parsed Shader Name : " << (rawAst.hasShaderName ? rawAst.shaderName : "(None)") << endl;
    cout << "    Parsed Color       : " << (rawAst.hasColor ? rawAst.colorName : "(Default)") << endl;

    // ---------------------------------------------------------
    // Phase 3: Semantic Analysis (Semantic Analyzer)
    // ---------------------------------------------------------
    cout << "\n>>> Phase 3: Semantic Analysis (Semantic Analyzer)..." << endl;
    SemanticAnalyzer semantic(rawAst);
    ValidatedShaderData validatedData = semantic.analyze();
    cout << "  [SUCCESS] Semantic checks passed. Symbol table & bounds verified." << endl;
    cout << "    Validated Shader  : " << validatedData.shaderName << endl;
    cout << "    Resolved Color    : " << validatedData.colorName 
         << " -> RGBA(" << validatedData.colorR << ", " 
         << validatedData.colorG << ", " 
         << validatedData.colorB << ", " 
         << validatedData.colorA << ")" << endl;
    cout << "    Validated Noise   : " << (validatedData.hasNoise ? "Active" : "None") << endl;
    if (validatedData.hasNoise) {
        cout << "      Scale           : " << validatedData.noiseScale << endl;
        cout << "      Speed           : " << validatedData.noiseSpeed << endl;
    }
    cout << "    Validated Distortion: " << validatedData.distortion << endl;
    cout << "    Validated Emission  : " << validatedData.emission << endl;

    // ---------------------------------------------------------
    // Phase 4: Code Generation (Codegen)
    // ---------------------------------------------------------
    cout << "\n>>> Phase 4: Code Generation (Codegen)..." << endl;
    string generatedHLSL = CodeGenerator::generateHLSL(validatedData);
    cout << "  [SUCCESS] Valid HLSL shader code generated." << endl;

    // ---------------------------------------------------------
    // Output File Writing into 'outputs/' folder
    // ---------------------------------------------------------
    ensureOutputDirectory("outputs");
    string outputFileName = "outputs/" + validatedData.shaderName + ".hlsl";
    writeOutputFile(outputFileName, generatedHLSL);

    cout << "\n======================================================" << endl;
    cout << "COMPILATION SUCCESSFUL!" << endl;
    cout << "Target HLSL File: " << outputFileName << endl;
    cout << "======================================================" << endl;

    return 0;
}
