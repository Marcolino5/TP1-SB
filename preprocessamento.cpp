#include "preprocessamento.hpp"   
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdio>

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

    // remove espaços finais
    while (!saida.empty() && saida.back() == ' ')
        saida.pop_back();

    return saida;
}

// processa diretiva EQU e salva na tabela de símbolos
bool processarEqu(
    const string &linha,
    map<string, string> &tabelaEqu
) {

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

// substitui ocorrências de EQU respeitando limites de identificadores
string substituirEqu(
    string linha,
    const map<string, string> &tabelaEqu
) {

    for (const auto &par : tabelaEqu) {

        const string &rotulo = par.first;
        const string &valor = par.second;

        size_t pos = 0;

        while ((pos = linha.find(rotulo, pos)) != string::npos) {

            bool primeiro =
                (pos == 0) ||
                (!isalnum(linha[pos - 1]) &&
                 linha[pos - 1] != '_');

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

// avalia diretiva condicional IF (se símbolo == 0, próxima linha é ignorada)
bool processarIf(
    const string &linha,
    const map<string, string> &tabelaEqu
) {

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

// normaliza constante numérica após CONST
string processarConstante(const string &linha) {

    size_t pos = linha.find("CONST");

    if (pos == string::npos)
        return linha;

    string antes = linha.substr(0, pos);

    string depois = linha.substr(pos + 5);

    while (!depois.empty() && depois[0] == ' ')
        depois.erase(0, 1);

    size_t espaco = depois.find(' ');

    string valor;

    if (espaco == string::npos)
        valor = depois;
    else
        valor = depois.substr(0, espaco);

    long numero = stol(valor, nullptr, 0);

    return antes + "CONST " + to_string(numero);
}

// garante formato correto da instrução COPY (operandos separados por vírgula)
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

// reorganiza linha separando label, opcode e operandos
string processarLinha(
    const string &linha,
    string &rotuloPendente
) {

    string rotulo;
    string opcode;
    string operando;

    size_t doisPontos = linha.find(':');

    string resto = linha;

    if (doisPontos != string::npos) {

        rotulo = linha.substr(0, doisPontos);

        resto = linha.substr(doisPontos + 1);

        while (!resto.empty() && resto[0] == ' ')
            resto.erase(0, 1);
    }

    size_t espaco = resto.find(' ');

    if (espaco == string::npos) {

        opcode = resto;

    } else {

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

// valida linha não vazia
bool validarLinha(const string &linha) {

    if (linha.empty())
        return false;

    return true;
}

int main() {

    ifstream arquivoEntrada("myfile.asm");

    ofstream arquivoSaida("myfile.pre");

    if (!arquivoEntrada.is_open()) {

        cerr << "Erro ao abrir entrada.asm\n";

        return 1;
    }

    if (!arquivoSaida.is_open()) {

        cerr << "Erro ao criar myfile.pre\n";

        return 1;
    }

    map<string, string> tabelaEqu;

    string linha;

    string rotuloPendente;

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

            string processado =
                processarLinha(linha, rotuloPendente);

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
                 << numeroLinha
                 << ": "
                 << linha
                 << endl;

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

        remove("myfile.pre");

        cerr << "Erro: Nao foi possivel criar corretamente o arquivo myfile.pre.\n";

        return 1;
    }

    cout << "Preprocessamento concluído.\n";

    return 0;
}