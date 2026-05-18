#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <map>
#include <vector>
#include <string>
using namespace std;

struct InfoInst {
    int opcode;
    int tamanho;
    int numOperandos;
};

struct Simbolo {
    bool definido = false;
    int endereco = -1;
    vector<int> pendencias;
};

struct LinhaParseada {
    string rotulo;
    string opcode;
    vector<string> operands;
};

LinhaParseada parsearLinha(const string &linha);

bool validarLinhaParseada(const LinhaParseada &linhaParseada,
                        const map<string, InfoInst> &tabelaInstrucoes);

bool validarRotuloFormato(const string &linha);

bool validarRotuloData(const string &linha);

int checaOperando(
    const string &operando,
    map<string, Simbolo> &tabelaSimbolos,
    int enderecoAtual
);

int montar(const string &arquivo);

#endif