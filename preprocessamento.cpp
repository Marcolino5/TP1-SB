#include "preprocessamento.hpp"
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdio>
#include <map>

using namespace std;

// remove tudo após ';' (comentário de assembly)
string removerComentario(const string &linha) {

    size_t pos = linha.find(';');

    if (pos == string::npos)
        return linha;

    return linha.substr(0, pos);
}

// normaliza texto: maiúsculas + remove múltiplos espaços/tabs
string normalizarTexto(const string &linha) {

    string saida;

    bool ultimoEspaco = false;

    for (unsigned char c : linha) {

        char ch = static_cast<char>(toupper(c));

        if (ch == '\t')
            ch = ' ';

        if (isspace(ch)) {

            if (!saida.empty() && !ultimoEspaco) {
                saida += ' ';
                ultimoEspaco = true;
            }

        } else {
            saida += ch;
            ultimoEspaco = false;
        }
    }

    while (!saida.empty() && saida.back() == ' ')
        saida.pop_back();

    return saida;
}

bool processarEqu(const string &linha, map<string, string> &tabelaEqu) {

    size_t doisPontos = linha.find(':');

    if (doisPontos == string::npos)
        return false;

    string rotulo = linha.substr(0, doisPontos);
    string resto = linha.substr(doisPontos + 1);

    while (!resto.empty() && resto[0] == ' ')
        resto.erase(0, 1);

    size_t espaco = resto.find(' ');

    if (espaco == string::npos)
        return false;

    string diretiva = resto.substr(0, espaco);

    if (diretiva != "EQU")
        return false;

    string valor = resto.substr(espaco + 1);

    while (!valor.empty() && valor[0] == ' ')
        valor.erase(0, 1);

    tabelaEqu[rotulo] = valor;

    return true;
}

string substituirEqu(string linha, const map<string, string> &tabelaEqu) {

    for (const auto &par : tabelaEqu) {

        const string &rotulo = par.first;
        const string &valor = par.second;

        size_t pos = 0;

        while ((pos = linha.find(rotulo, pos)) != string::npos) {

            bool primeiro =
                (pos == 0) ||
                (!isalnum(linha[pos - 1]) && linha[pos - 1] != '_');

            bool segundo =
                (pos + rotulo.size() >= linha.size()) ||
                (!isalnum(linha[pos + rotulo.size()]) &&
                 linha[pos + rotulo.size()] != '_');

            if (primeiro && segundo) {
                linha.replace(pos, rotulo.size(), valor);
                pos += valor.size();
            } else {
                pos += rotulo.size();
            }
        }
    }

    return linha;
}

bool processarIf(const string &linha, const map<string, string> &tabelaEqu) {

    if (linha.find("IF ") != 0)
        return true;

    string simbolo = linha.substr(3);

    while (!simbolo.empty() && simbolo[0] == ' ')
        simbolo.erase(0, 1);

    auto it = tabelaEqu.find(simbolo);

    if (it == tabelaEqu.end())
        return false;

    return (it->second != "0");
}

string processarConstante(const string &linha) {

    size_t pos = linha.find("CONST");

    if (pos == string::npos)
        return linha;

    string antes = linha.substr(0, pos);
    string depois = linha.substr(pos + 5);

    while (!depois.empty() && depois[0] == ' ')
        depois.erase(0, 1);

    size_t espaco = depois.find(' ');

    string valor = (espaco == string::npos)
        ? depois
        : depois.substr(0, espaco);

    long numero = stol(valor, nullptr, 0);

    return antes + "CONST " + to_string(numero);
}

string processarCopy(const string &linha) {

    size_t pos = linha.find("COPY");

    if (pos == string::npos)
        return linha;

    string antes = linha.substr(0, pos);

    while (!antes.empty() && antes.back() == ' ')
        antes.pop_back();

    string depois = linha.substr(pos + 4);

    while (!depois.empty() && depois[0] == ' ')
        depois.erase(0, 1);

    size_t virgula = depois.find(',');

    if (virgula == string::npos)
        return linha;

    string op1 = depois.substr(0, virgula);
    string op2 = depois.substr(virgula + 1);

    while (!op1.empty() && op1[0] == ' ')
        op1.erase(0, 1);

    while (!op1.empty() && op1.back() == ' ')
        op1.pop_back();

    while (!op2.empty() && op2[0] == ' ')
        op2.erase(0, 1);

    return antes + "COPY " + op1 + "," + op2;
}

string processarLinha(string linha, string &rotuloPendente) {

    string rotulo, opcode, operando;

    size_t doisPontos = linha.find(':');
    string resto = linha;

    if (doisPontos != string::npos) {

        rotulo = linha.substr(0, doisPontos);
        resto = linha.substr(doisPontos + 1);

        while (!resto.empty() && resto[0] == ' ')
            resto.erase(0, 1);
    }

    size_t espaco = resto.find(' ');

    if (espaco == string::npos)
        opcode = resto;
    else {
        opcode = resto.substr(0, espaco);
        operando = resto.substr(espaco + 1);
    }

    if (!rotulo.empty() && opcode.empty()) {
        rotuloPendente = rotulo + ":";
        return "";
    }

    string resultado;

    if (!rotuloPendente.empty()) {
        resultado += rotuloPendente + " ";
        rotuloPendente.clear();
    }

    if (!rotulo.empty())
        resultado += rotulo + ": ";

    resultado += opcode;

    if (!operando.empty())
        resultado += " " + operando;

    return resultado;
}

bool validarLinha(const string &linha) {
    return !linha.empty();
}

void preprocessar(const string &entrada) {

    ifstream arquivoEntrada(entrada);
    string saida = entrada.substr(0, entrada.find_last_of('.')) + ".pre";
    ofstream arquivoSaida(saida);

    if (!arquivoEntrada.is_open()) {
        cerr << "Erro ao abrir " << entrada << endl;
        return;
    }

    if (!arquivoSaida.is_open()) {
        cerr << "Erro ao criar o arquivo preprocessado\n";
        return;
    }

    map<string, string> tabelaEqu;

    string linha, rotuloPendente;

    bool primeiro = true;
    bool erro = false;
    bool pularProximaLinha = false;

    int numeroLinha = 0;

    string secoesData;
    bool dados = false;

    arquivoSaida << "SECTION TEXT";
    primeiro = false;

    while (getline(arquivoEntrada, linha)) {

        numeroLinha++;

        linha = removerComentario(linha);
        linha = normalizarTexto(linha);

        if (linha.empty())
            continue;

        if (pularProximaLinha) {
            pularProximaLinha = false;
            continue;
        }

        if (processarEqu(linha, tabelaEqu))
            continue;

        if (linha.find("IF ") == 0) {

            bool manter = processarIf(linha, tabelaEqu);

            if (!manter)
                pularProximaLinha = true;

            continue;
        }

        linha = substituirEqu(linha, tabelaEqu);

        if (linha == "SECTION DATA") {
            dados = true;
            continue;
        }

        if (linha == "SECTION TEXT") {
            dados = false;
            continue;
        }

        if (dados) {

            string processado = processarLinha(linha, rotuloPendente);
            processado = processarConstante(processado);
            processado = processarCopy(processado);

            if (!processado.empty()) {
                if (!secoesData.empty())
                    secoesData += "\n";
                secoesData += processado;
            }

            continue;
        }

        linha = processarLinha(linha, rotuloPendente);
        linha = processarConstante(linha);
        linha = processarCopy(linha);

        if (linha.empty())
            continue;

        if (!validarLinha(linha)) {
            cerr << "Erro sintático na linha "
                 << numeroLinha << ": " << linha << endl;
            erro = true;
            break;
        }

        if (!primeiro)
            arquivoSaida << '\n';

        arquivoSaida << linha;
        primeiro = false;
    }

    arquivoEntrada.close();

    if (!erro && !secoesData.empty()) {
        arquivoSaida << "\nSECTION DATA\n";
        arquivoSaida << secoesData;
    }

    arquivoSaida.close();

    if (erro) {
        remove(saida.c_str());
        cerr << "Erro ao gerar o arquivo preprocessado\n";
    } else {
        cout << "Preprocessamento concluido.\n";
    }
}