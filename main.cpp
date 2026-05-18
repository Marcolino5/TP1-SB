#include "preprocessamento.hpp"
#include "assembler.hpp"
#include "simulador.hpp"
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

string getExtensao(const string &arquivo) {

    size_t pos = arquivo.find_last_of('.');

    if (pos == string::npos)
        return "";

    return arquivo.substr(pos + 1);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cout << "Uso: ./montador arquivo.asm|pre|obj" << endl;
        return 1;
    }

    string arquivo = argv[1];
    string ext = getExtensao(arquivo);

    if (ext == "asm") {

        cout << "== PREPROCESSANDO ==" << endl;
        preprocessar(arquivo);

        cout << "== MONTANDO ==" << endl;
        montar(arquivo.substr(0, arquivo.find_last_of('.')) + ".pre");

    } 
    else if (ext == "pre") {

        cout << "== MONTANDO ==" << endl;
        montar(arquivo);

    } 
    else if (ext == "obj") {

        cout << "== EXECUTANDO ==" << endl;
        executar(arquivo);

    } 
    else {
        cout << "Extensao invalida. Use .asm .pre ou .obj" << endl;
        return 1;
    }

    return 0;
}