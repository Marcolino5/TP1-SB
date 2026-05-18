#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>

using namespace std;

bool validarRotuloFormato(const string &linha);

bool validarRotuloData(const string &linha);

int montar(const string &arquivo);

#endif