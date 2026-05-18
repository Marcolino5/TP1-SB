#include "assembler.hpp"
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdio>
#include <sstream>

//analisador lexico sintatico e semantico
//gerar codigo objeto e tabela de pendencias

LinhaParseada parsearLinha(const string & linha) {

    LinhaParseada resultado;

    // separa linha nas diferentes partes: label (se tiver), instrução/diretiva e operandos (se tiver)
    stringstream ss(linha);
    string parte;
    vector<string> partes;
    while (ss >> parte) {
        partes.push_back(parte);
    }

    if (partes.empty())
         return resultado;

    int index = 0;
    if (partes[0].back() == ':') {
        resultado.rotulo = partes[0].substr(0, partes[0].size() - 1);
        index++;
    }

    // cobre caso em que rótulo e operação tem quebra de linha
    if (index < partes.size()) {
        resultado.opcode = partes[index];
        index++;
    }

    if (index < partes.size()) {
        string operando = partes[index];
        if (operando.find(',') != string::npos) {
            size_t posVirgula = operando.find(',');
            resultado.operands.push_back(operando.substr(0, posVirgula));
            resultado.operands.push_back(operando.substr(posVirgula + 1));
        } else resultado.operands.push_back(operando);

    }


    return resultado;
}

// verifica se linha parseada é válida sintaticamente (instrução existe, número de operandos correto, diretiva válida)
bool validarLinhaParseada(const LinhaParseada &linhaParseada,
                        const map<string, InfoInst> &tabelaInstrucoes) {
    if (linhaParseada.opcode.empty())
        return true;
        
    auto it = tabelaInstrucoes.find(linhaParseada.opcode);

    if (it == tabelaInstrucoes.end())
        return false;

    const InfoInst &info = it->second;

    if (linhaParseada.operands.size() != info.numOperandos)
        return false;

    return true;
}

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

bool converteAInt(const string &s) {
    try {
        size_t pos;
        std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

// verifica se operando é um número ou símbolo indefinido/pendente
// retorna número que deve ser adicionado ao código
int checaOperando (
    const string &operando,
    map<string, Simbolo> &tabelaSimbolos,
    int enderecoAtual
) {
    if (converteAInt(operando))
        return std::stoi(operando);
    else {
        if (tabelaSimbolos[operando].definido) return tabelaSimbolos[operando].endereco;
        else {
            tabelaSimbolos[operando].pendencias.push_back(enderecoAtual);
            return -1; // endereço provisório para símbolo pendente
        }
    }
}

int montar(const string &arquivo) {
    ifstream arquivoEntrada(arquivo);

    ofstream arquivoSaida1(arquivo.substr(0, arquivo.find_last_of('.')) + ".obj");
    ofstream arquivoSaida2(arquivo.substr(0, arquivo.find_last_of('.')) + ".pen");

    if (!arquivoEntrada.is_open()) {
        cerr << "Erro ao abrir " << arquivo << "\n";
        return 1;
    }

    if (!arquivoSaida1.is_open()) {
        cerr << "Erro ao criar " << arquivo.substr(0, arquivo.find_last_of('.')) + ".obj" << "\n";
        return 1;
    }

    if (!arquivoSaida2.is_open()) {
        cerr << "Erro ao criar " << arquivo.substr(0, arquivo.find_last_of('.')) + ".pen" << "\n";
        return 1;
    }

    map<string, InfoInst> tabelaInstrucoes = {
        {"ADD", {1, 2, 1}},
        {"SUB", {2, 2, 1}},
        {"MULT", {3, 2, 1}},
        {"DIV", {4, 2, 1}},
        {"JMP", {5, 2, 1}},
        {"JMPN", {6, 2, 1}},
        {"JMPP", {7, 2, 1}},
        {"JMPZ", {8, 2, 1}},
        {"COPY", {9, 3, 2}},
        {"LOAD", {10, 2, 1}},
        {"STORE", {11, 2, 1}},
        {"INPUT", {12, 2, 1}},
        {"OUTPUT", {13, 2, 1}},
        {"STOP", {14, 1, 0}},
        {"SPACE", {0, 1, 0}},
        {"CONST", {-1, 1, 1}}
    };
    map<string, Simbolo> tabelaSimbolos;

    string linha;

    int numeroLinha = 0;

    bool dados = false;
    bool erro = false;

    vector<int> codigoObj;
    vector<int> codigoPen;
    int posicaoAtual = 0;

    while (getline(arquivoEntrada, linha)) {
        numeroLinha++;

        if (linha == "SECTION DATA") {
            dados = true;
            continue;}
        if (linha == "SECTION TEXT") {
            dados = false;
            continue;
        }
        if (linha.empty()) continue;

        LinhaParseada linhaParseada;
        linhaParseada = parsearLinha(linha);

        if (!validarLinhaParseada(linhaParseada, tabelaInstrucoes)) {

            cerr << "Erro sintatico na linha "
                 << numeroLinha
                 << ": "
                 << linha
                 << endl;

            erro = true;

            break;
        }
        
        if (!linhaParseada.rotulo.empty()) {
            if (dados) {
                if (!validarRotuloData(linha)) {
                    cerr << "Erro sintatico: rotulo com formato invalido na linha " << numeroLinha << "\n";
                    erro = true;
                    break;
                }
            } else {
                if (!validarRotuloFormato(linha)) {
                    cerr << "Erro sintatico: rotulo com formato invalido na linha " << numeroLinha << "\n";
                    erro = true;
                    break;
                }
            }

            if (tabelaSimbolos.count(linhaParseada.rotulo) && tabelaSimbolos[linhaParseada.rotulo].definido) {
                cerr << "Erro semantico: rotulo " << linhaParseada.rotulo << " ja definido (linha " << numeroLinha << ")\n";
                erro = true;
                break;
            }

            tabelaSimbolos[linhaParseada.rotulo].definido = true;
            tabelaSimbolos[linhaParseada.rotulo].endereco = posicaoAtual;

            for (int pendencia : tabelaSimbolos[linhaParseada.rotulo].pendencias) {
                codigoObj[pendencia] = posicaoAtual;
            }
        }

        if (linhaParseada.opcode.empty()) continue; // linha só tem rótulo
        int opcode = (tabelaInstrucoes[linhaParseada.opcode].opcode);
        switch (opcode) {
            case -1:
                codigoObj.push_back(std::stoi(linhaParseada.operands[0]));
                codigoPen.push_back(std::stoi(linhaParseada.operands[0]));
                break;
            case 0: case 14:
                codigoObj.push_back(opcode);
                codigoPen.push_back(opcode);
                break;
            case 9:
                codigoObj.push_back(9);
                codigoPen.push_back(9);

                codigoObj.push_back(checaOperando(linhaParseada.operands[0], tabelaSimbolos, codigoObj.size()));
                codigoPen.push_back(codigoObj.back());
                
                codigoObj.push_back(checaOperando(linhaParseada.operands[1], tabelaSimbolos, codigoObj.size()));
                codigoPen.push_back(codigoObj.back());
                break;
            default:
                codigoObj.push_back(opcode);
                codigoPen.push_back(opcode);

                
                codigoObj.push_back(checaOperando(linhaParseada.operands[0], tabelaSimbolos, codigoObj.size()));
                codigoPen.push_back(codigoObj.back());
                break;
        }
        posicaoAtual += tabelaInstrucoes[linhaParseada.opcode].tamanho;
    }
    for (const auto &par : tabelaSimbolos) {
        if (!par.second.definido) {
            cerr << "Erro semantico: simbolo " << par.first << " indefinido\n";
            erro = true;
        }
    }

    for (int valor : codigoObj) {
        arquivoSaida1 << valor << " ";
    }
    for (int valor : codigoPen) {
        arquivoSaida2 << valor << " ";
    }

    arquivoEntrada.close();
    arquivoSaida1.close();
    arquivoSaida2.close();

    if (erro) {
        cerr << "Erro ao gerar o arquivo objeto\n";
    } else {
        cout << "Montagem concluida.\n";
    }

    return erro ? 1 : 0;
}