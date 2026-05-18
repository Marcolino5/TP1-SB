#include "simulador.hpp"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int executar(const string &arquivo) {

    ifstream file(arquivo);

    if (!file.is_open()) {
        cout << "Erro ao abrir arquivo" << endl;
        return 1;
    }

    vector<int> memoria;
    int valor;

    while (file >> valor) {
        memoria.push_back(valor);
    }

    file.close();

    int pc = 0;
    int acc = 0;

    while (true) {

        if (pc < 0 || pc >= (int)memoria.size()) {
            cout << "Erro: PC fora da memoria" << endl;
            return 1;
        }

        int opcode = memoria[pc];

        switch (opcode) {

            case 1: // ADD
                acc += memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 2: // SUB
                acc -= memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 3: // MULT
                acc *= memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 4: // DIV
                if (memoria[memoria[pc + 1]] == 0) {
                    cout << "Erro: divisao por zero" << endl;
                    return 1;
                }
                acc /= memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 5: // JMP
                pc = memoria[pc + 1];
                break;

            case 6: // JMPN
                if (acc < 0) pc = memoria[pc + 1];
                else pc += 2;
                break;

            case 7: // JMPP
                if (acc > 0) pc = memoria[pc + 1];
                else pc += 2;
                break;

            case 8: // JMPZ
                if (acc == 0) pc = memoria[pc + 1];
                else pc += 2;
                break;

            case 9: // COPY
                memoria[memoria[pc + 2]] =
                    memoria[memoria[pc + 1]];
                pc += 3;
                break;

            case 10: // LOAD
                acc = memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 11: // STORE
                memoria[memoria[pc + 1]] = acc;
                pc += 2;
                break;

            case 12: // INPUT
                cin >> memoria[memoria[pc + 1]];
                pc += 2;
                break;

            case 13: // OUTPUT
                cout << memoria[memoria[pc + 1]] << endl;
                pc += 2;
                break;

            case 14: // STOP
                return 0;

            default:
                cout << "Opcode invalido: " << opcode << endl;
                return 1;
        }
    }
}