#ifndef PREPROCESSAMENTO_HPP
#define PREPROCESSAMENTO_HPP

#include <string>
#include <map>

using namespace std;

string removerComentario(const string &linha);

string normalizarTexto(const string &linha);

bool processarEqu(
    const string &linha,
    map<string, string> &tabelaEqu
);

string substituirEqu(
    string linha,
    const map<string, string> &tabelaEqu
);

bool processarIf(
    const string &linha,
    const map<string, string> &tabelaEqu
);

string processarConstante(const string &linha);

string processarCopy(const string &linha);

string processarLinha(
    const string linha,
    string &rotuloPendente
);

bool validarLinha(const string &linha);

void preprocessar(const string &entrada);

#endif