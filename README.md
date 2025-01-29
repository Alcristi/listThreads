# ListThreads
Este repositório contém a implementação das questões da lista de Threads da disciplina **Infraestrutura de Software (IF677)** do curso de **Engenharia da Computação** do **CIn-UFPE**, que aborda conceitos fundamentais de programação concorrente, explorando sincronização, exclusão mútua, escalonamento de threads, comunicação entre threads, etc. Utilizamos pthreads em C para resolver desafios práticos que simulam cenários reais de concorrência, como controle de tráfego, processamento paralelo de arquivos, filas bloqueantes e gerenciamento de acesso a dados. As soluções foram desenvolvidas de forma modular, priorizando clareza, organização e eficiência, além do uso de mutexes e variáveis de condição para garantir a sincronização correta entre threads.

---
## 👥 Equipe
- Allan Cristian Santos do Nascimento (acsn3)
- Bianca Duarte Santos (bds)

---

## 📂 Estrutura do Repositório

Cada questão está organizada em uma pasta `exN`, onde `N` representa o número da questão. Os arquivos de cada pasta incluem:
- **Código-fonte** (`.c`): Implementação da solução.
- **Arquivos de entrada** (`.txt`): Caso necessário para testes.
- **Script de execução** (`test.sh`): Para compilar e executar o código, se aplicável.

```
listThreads/
│── ex1/
│── ex2/
│── ex3/
│── ex4/
│── ex5/
│── ex6/
│── equipe.txt
│── README.md
```

## 🚀 Execução
Para compilar e rodar cada questão, utilize:
```sh
gcc -o exN exN.c
./exN
```
Ou, caso um script `test.sh` esteja disponível:
```sh
chmod +x test.sh
./test.sh
```

---