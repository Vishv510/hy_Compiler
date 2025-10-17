#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <variant>
#include "lexer.hpp"
#include "parser.hpp"
#include "generation.hpp"

int main(int argc, char* argv[]){
    // std::cout << argv[0] << " " <<  argv[1] << "\n";
    if (argc != 2) {
        std::cerr << "Incorrect file path. Correct usage is ..." << std::endl;
        std::cerr << "my Example.hy ....." << std::endl;
        return EXIT_SUCCESS;
    }

    // copy high level language
    std::fstream inputFile(argv[1], std::ios::in);
    std::stringstream contents_stream;
    contents_stream << inputFile.rdbuf();
    inputFile.close();

    std::string contents = contents_stream.str();

    // ------------------
    // lexer build
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    // print tokens for validation
    // for (const Token& t:tokens) {
    //     std::cout << "types: " << tokenToString(t.type) << " | line: " << t.line << " | column: " << t.column << " value: " << t.value.value() << std::endl;
    // }

    // ----------------
    // parser tree build
    Parser p(tokens);
    auto prs = p.parse_program();

    if (!prs.has_value()) {
        std::cerr <<  "Parsing error." << std::endl;
        return EXIT_FAILURE;
    }

    //-------------------
    // assembly genration
    try {
        Generator generator(prs.value());
        std::string assembly = generator.gen_prog();

        // check if generaion have error
        if (assembly.empty()) {
            std::cerr << "WARNING: Generated assembly is empty!\n";
        }
        // else { // print assembly language
        //     std::cout << "\n--- Assembly Output ---\n";
        //     std::cout << assembly << std::endl;
        //     std::cout << "--- End Assembly ---\n\n";
        // }

        // Write to file to out.asm fix file
        std::string output_filename = "out.asm";
        std::ofstream output_file(output_filename);
        if (!output_file.is_open()) { // check file open successfully
            std::cerr << "ERROR: Could not open output file: " << output_filename << std::endl;
            return EXIT_FAILURE;
        }

        output_file << assembly; // write all into file
        output_file.close();

        // Assemble and link
        std::cout << "\n=== ASSEMBLING ===\n";
        system("nasm -felf64 out.asm");


        std::cout << "=== LINKING ===\n";
        system("ld -o out out.o"); // generate out.o file

        std::cout << "\nCompilation complete! Executable: ./out\n";
    } catch (const std::exception& e) {
        std::cerr << "ERROR during code generation: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}