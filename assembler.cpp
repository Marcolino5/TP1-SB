#include "assembler.hpp"
#include <string>
#include <cctype>

using namespace std;

bool validarRotuloFormato(const string &linha) {

    size_t doisPontos = linha.find(':');

    if (doisPontos == string::npos)
        return true;

    string rotulo = linha.substr(0, doisPontos);

    while (!rotulo.empty() && rotulo[0] == ' ')
        rotulo.erase(0, 1);

    if (rotulo.empty())
        return false;

    if (!isalpha(rotulo[0]))
        return false;

    for (char c : rotulo) {

        if (!(isalnum(c) || c == '_'))
            return false;
    }

    return true;
}

bool validarRotuloData(const string &linha) {

    size_t doisPontos = linha.find(':');

    if (doisPontos == string::npos)
        return false;

    string rotulo = linha.substr(0, doisPontos);

    while (!rotulo.empty() && rotulo[0] == ' ')
        rotulo.erase(0, 1);

    if (rotulo.empty())
        return false;

    if (!isalpha(rotulo[0]))
        return false;

    for (char c : rotulo) {

        if (!(isalnum(c) || c == '_'))
            return false;
    }

    return true;
}

int montar(const string &arquivo) {
    
    
    
    
    
    
    return 0;
}