#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQ_PRODUTOS "produtos.bin"
#define ARQ_VENDAS "vendas.bin"
#define TEMP_PRODUTOS "temp_produtos.bin"
#define TEMP_VENDAS "temp_vendas.bin"

#define MAX_ITENS 10

typedef struct {
    int id;
    char descricao[50];
    float preco;
    int estoque;
} Produto;

typedef struct {
    int id;
    char cliente[50];
    int totalItens;
    int idsProdutos[MAX_ITENS];
    int quantidades[MAX_ITENS];
} Venda;

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void lerTexto(char texto[], int tamanho) {
    fgets(texto, tamanho, stdin);
    texto[strcspn(texto, "\n")] = '\0';
}

int proximoIdProduto() {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb");
    Produto p;
    int maiorId = 0;

    if (arquivo == NULL) {
        return 1;
    }

    while (fread(&p, sizeof(Produto), 1, arquivo) == 1) {
        if (p.id > maiorId) {
            maiorId = p.id;
        }
    }

    fclose(arquivo);
    return maiorId + 1;
}

int proximoIdVenda() {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb");
    Venda v;
    int maiorId = 0;

    if (arquivo == NULL) {
        return 1;
    }

    while (fread(&v, sizeof(Venda), 1, arquivo) == 1) {
        if (v.id > maiorId) {
            maiorId = v.id;
        }
    }

    fclose(arquivo);
    return maiorId + 1;
}

int buscarProdutoPorId(int id, Produto *produto, long *posicao) {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb");

    if (arquivo == NULL) {
        return 0;
    }

    while (fread(produto, sizeof(Produto), 1, arquivo) == 1) {
        if (produto->id == id) {
            if (posicao != NULL) {
                *posicao = ftell(arquivo) - sizeof(Produto);
            }

            fclose(arquivo);
            return 1;
        }
    }

    fclose(arquivo);
    return 0;
}

int buscarVendaPorId(int id, Venda *venda, long *posicao) {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb");

    if (arquivo == NULL) {
        return 0;
    }

    while (fread(venda, sizeof(Venda), 1, arquivo) == 1) {
        if (venda->id == id) {
            if (posicao != NULL) {
                *posicao = ftell(arquivo) - sizeof(Venda);
            }

            fclose(arquivo);
            return 1;
        }
    }

    fclose(arquivo);
    return 0;
}

void cadastrarProduto() {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "ab");
    Produto p;

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de produtos.\n");
        return;
    }

    p.id = proximoIdProduto();

    printf("\n--- Cadastro de Produto ---\n");

    printf("Descricao: ");
    limparBuffer();
    lerTexto(p.descricao, 50);

    printf("Preco: ");
    scanf("%f", &p.preco);

    printf("Quantidade em estoque: ");
    scanf("%d", &p.estoque);

    fwrite(&p, sizeof(Produto), 1, arquivo);
    fclose(arquivo);

    printf("\nProduto cadastrado com sucesso! ID gerado: %d\n", p.id);
}

void listarProdutos() {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb");
    Produto p;

    if (arquivo == NULL) {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    printf("\n--- Lista de Produtos ---\n");

    while (fread(&p, sizeof(Produto), 1, arquivo) == 1) {
        printf("\nID: %d\n", p.id);
        printf("Descricao: %s\n", p.descricao);
        printf("Preco: R$ %.2f\n", p.preco);
        printf("Estoque: %d\n", p.estoque);
    }

    fclose(arquivo);
}

void buscarProdutoEspecifico() {
    Produto p;
    int id;

    printf("\nDigite o ID do produto: ");
    scanf("%d", &id);

    if (buscarProdutoPorId(id, &p, NULL)) {
        printf("\n--- Produto Encontrado ---\n");
        printf("ID: %d\n", p.id);
        printf("Descricao: %s\n", p.descricao);
        printf("Preco: R$ %.2f\n", p.preco);
        printf("Estoque: %d\n", p.estoque);
    } else {
        printf("\nProduto nao encontrado.\n");
    }
}

void atualizarProduto() {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb+");
    Produto p;
    long posicao;
    int id;

    if (arquivo == NULL) {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    printf("\nDigite o ID do produto que deseja atualizar: ");
    scanf("%d", &id);

    if (!buscarProdutoPorId(id, &p, &posicao)) {
        printf("\nProduto nao encontrado.\n");
        fclose(arquivo);
        return;
    }

    printf("\nProduto atual: %s | R$ %.2f | Estoque: %d\n", p.descricao, p.preco, p.estoque);

    printf("\nNova descricao: ");
    limparBuffer();
    lerTexto(p.descricao, 50);

    printf("Novo preco: ");
    scanf("%f", &p.preco);

    printf("Novo estoque: ");
    scanf("%d", &p.estoque);

    fseek(arquivo, posicao, SEEK_SET);
    fwrite(&p, sizeof(Produto), 1, arquivo);

    fclose(arquivo);

    printf("\nProduto atualizado com sucesso.\n");
}

int produtoJaAdicionado(Venda v, int idProduto) {
    int i;

    for (i = 0; i < v.totalItens; i++) {
        if (v.idsProdutos[i] == idProduto) {
            return 1;
        }
    }

    return 0;
}

int quantidadeAntigaDoProduto(Venda v, int idProduto) {
    int i;

    for (i = 0; i < v.totalItens; i++) {
        if (v.idsProdutos[i] == idProduto) {
            return v.quantidades[i];
        }
    }

    return 0;
}

void alterarEstoqueProduto(int idProduto, int diferenca) {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb+");
    Produto p;

    if (arquivo == NULL) {
        return;
    }

    while (fread(&p, sizeof(Produto), 1, arquivo) == 1) {
        if (p.id == idProduto) {
            p.estoque += diferenca;

            fseek(arquivo, -sizeof(Produto), SEEK_CUR);
            fwrite(&p, sizeof(Produto), 1, arquivo);

            fclose(arquivo);
            return;
        }
    }

    fclose(arquivo);
}

void cadastrarVenda() {
    FILE *arquivo = fopen(ARQ_VENDAS, "ab");
    Venda v;
    Produto p;
    int i;

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de vendas.\n");
        return;
    }

    v.id = proximoIdVenda();

    printf("\n--- Cadastro de Venda ---\n");

    printf("Nome do cliente: ");
    limparBuffer();
    lerTexto(v.cliente, 50);

    printf("Quantidade de produtos na venda, maximo %d: ", MAX_ITENS);
    scanf("%d", &v.totalItens);

    if (v.totalItens <= 0 || v.totalItens > MAX_ITENS) {
        printf("\nQuantidade invalida.\n");
        fclose(arquivo);
        return;
    }

    for (i = 0; i < v.totalItens; i++) {
        int idProduto;
        int quantidade;

        printf("\nID do produto %d: ", i + 1);
        scanf("%d", &idProduto);

        if (produtoJaAdicionado(v, idProduto)) {
            printf("Esse produto ja foi adicionado nesta venda.\n");
            fclose(arquivo);
            return;
        }

        if (!buscarProdutoPorId(idProduto, &p, NULL)) {
            printf("Produto nao encontrado.\n");
            fclose(arquivo);
            return;
        }

        printf("Produto encontrado: %s | Estoque atual: %d\n", p.descricao, p.estoque);

        printf("Quantidade vendida: ");
        scanf("%d", &quantidade);

        if (quantidade <= 0 || quantidade > p.estoque) {
            printf("Quantidade invalida ou estoque insuficiente.\n");
            fclose(arquivo);
            return;
        }

        v.idsProdutos[i] = idProduto;
        v.quantidades[i] = quantidade;
    }

    fwrite(&v, sizeof(Venda), 1, arquivo);
    fclose(arquivo);

    for (i = 0; i < v.totalItens; i++) {
        alterarEstoqueProduto(v.idsProdutos[i], -v.quantidades[i]);
    }

    printf("\nVenda cadastrada com sucesso! ID gerado: %d\n", v.id);
}

void listarVendas() {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb");
    Venda v;
    int i;

    if (arquivo == NULL) {
        printf("\nNenhuma venda cadastrada.\n");
        return;
    }

    printf("\n--- Lista de Vendas ---\n");

    while (fread(&v, sizeof(Venda), 1, arquivo) == 1) {
        printf("\nID da venda: %d\n", v.id);
        printf("Cliente: %s\n", v.cliente);
        printf("Quantidade de itens: %d\n", v.totalItens);

        for (i = 0; i < v.totalItens; i++) {
            printf("Produto ID: %d | Quantidade: %d\n", v.idsProdutos[i], v.quantidades[i]);
        }
    }

    fclose(arquivo);
}

void buscarVendaEspecifica() {
    Venda v;
    Produto p;
    int id;
    int i;
    float total = 0;

    printf("\nDigite o ID da venda: ");
    scanf("%d", &id);

    if (!buscarVendaPorId(id, &v, NULL)) {
        printf("\nVenda nao encontrada.\n");
        return;
    }

    printf("\n--- Venda Encontrada ---\n");
    printf("ID da venda: %d\n", v.id);
    printf("Cliente: %s\n", v.cliente);

    for (i = 0; i < v.totalItens; i++) {
        if (buscarProdutoPorId(v.idsProdutos[i], &p, NULL)) {
            float subtotal = p.preco * v.quantidades[i];

            printf("\nProduto: %s\n", p.descricao);
            printf("Preco unitario: R$ %.2f\n", p.preco);
            printf("Quantidade: %d\n", v.quantidades[i]);
            printf("Subtotal: R$ %.2f\n", subtotal);

            total += subtotal;
        }
    }

    printf("\nTotal da venda: R$ %.2f\n", total);
}

void atualizarVenda() {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb+");
    Venda antiga, nova;
    Produto p;
    long posicao;
    int id;
    int i;

    if (arquivo == NULL) {
        printf("\nNenhuma venda cadastrada.\n");
        return;
    }

    printf("\nDigite o ID da venda que deseja atualizar: ");
    scanf("%d", &id);

    if (!buscarVendaPorId(id, &antiga, &posicao)) {
        printf("\nVenda nao encontrada.\n");
        fclose(arquivo);
        return;
    }

    nova.id = antiga.id;

    printf("\nNovo nome do cliente: ");
    limparBuffer();
    lerTexto(nova.cliente, 50);

    printf("Nova quantidade de produtos na venda, maximo %d: ", MAX_ITENS);
    scanf("%d", &nova.totalItens);

    if (nova.totalItens <= 0 || nova.totalItens > MAX_ITENS) {
        printf("\nQuantidade invalida.\n");
        fclose(arquivo);
        return;
    }

    for (i = 0; i < nova.totalItens; i++) {
        int idProduto;
        int quantidade;
        int estoqueDisponivel;

        printf("\nID do produto %d: ", i + 1);
        scanf("%d", &idProduto);

        if (produtoJaAdicionado(nova, idProduto)) {
            printf("Esse produto ja foi adicionado nesta venda.\n");
            fclose(arquivo);
            return;
        }

        if (!buscarProdutoPorId(idProduto, &p, NULL)) {
            printf("Produto nao encontrado.\n");
            fclose(arquivo);
            return;
        }

        estoqueDisponivel = p.estoque + quantidadeAntigaDoProduto(antiga, idProduto);

        printf("Produto encontrado: %s | Estoque disponivel: %d\n", p.descricao, estoqueDisponivel);

        printf("Nova quantidade vendida: ");
        scanf("%d", &quantidade);

        if (quantidade <= 0 || quantidade > estoqueDisponivel) {
            printf("Quantidade invalida ou estoque insuficiente.\n");
            fclose(arquivo);
            return;
        }

        nova.idsProdutos[i] = idProduto;
        nova.quantidades[i] = quantidade;
    }

    for (i = 0; i < antiga.totalItens; i++) {
        alterarEstoqueProduto(antiga.idsProdutos[i], antiga.quantidades[i]);
    }

    for (i = 0; i < nova.totalItens; i++) {
        alterarEstoqueProduto(nova.idsProdutos[i], -nova.quantidades[i]);
    }

    fseek(arquivo, posicao, SEEK_SET);
    fwrite(&nova, sizeof(Venda), 1, arquivo);

    fclose(arquivo);

    printf("\nVenda atualizada com sucesso.\n");
}

int produtoEstaEmVenda(int idProduto) {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb");
    Venda v;
    int i;

    if (arquivo == NULL) {
        return 0;
    }

    while (fread(&v, sizeof(Venda), 1, arquivo) == 1) {
        for (i = 0; i < v.totalItens; i++) {
            if (v.idsProdutos[i] == idProduto) {
                fclose(arquivo);
                return 1;
            }
        }
    }

    fclose(arquivo);
    return 0;
}

void removerProduto() {
    FILE *antigo = fopen(ARQ_PRODUTOS, "rb");
    FILE *novo = fopen(TEMP_PRODUTOS, "wb");
    Produto p;
    int id;
    int encontrado = 0;

    if (antigo == NULL) {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    if (novo == NULL) {
        printf("\nErro ao criar arquivo temporario.\n");
        fclose(antigo);
        return;
    }

    printf("\nDigite o ID do produto que deseja remover: ");
    scanf("%d", &id);

    if (produtoEstaEmVenda(id)) {
        printf("\nNao e possivel remover este produto, pois ele esta associado a uma venda.\n");
        fclose(antigo);
        fclose(novo);
        remove(TEMP_PRODUTOS);
        return;
    }

    while (fread(&p, sizeof(Produto), 1, antigo) == 1) {
        if (p.id == id) {
            encontrado = 1;
        } else {
            fwrite(&p, sizeof(Produto), 1, novo);
        }
    }

    fclose(antigo);
    fclose(novo);

    remove(ARQ_PRODUTOS);
    rename(TEMP_PRODUTOS, ARQ_PRODUTOS);

    if (encontrado) {
        printf("\nProduto removido com sucesso.\n");
    } else {
        printf("\nProduto nao encontrado.\n");
    }
}

void removerVenda() {
    FILE *antigo = fopen(ARQ_VENDAS, "rb");
    FILE *novo = fopen(TEMP_VENDAS, "wb");
    Venda v;
    Venda removida;
    int id;
    int encontrado = 0;
    int i;

    if (antigo == NULL) {
        printf("\nNenhuma venda cadastrada.\n");
        return;
    }

    if (novo == NULL) {
        printf("\nErro ao criar arquivo temporario.\n");
        fclose(antigo);
        return;
    }

    printf("\nDigite o ID da venda que deseja remover: ");
    scanf("%d", &id);

    while (fread(&v, sizeof(Venda), 1, antigo) == 1) {
        if (v.id == id) {
            encontrado = 1;
            removida = v;
        } else {
            fwrite(&v, sizeof(Venda), 1, novo);
        }
    }

    fclose(antigo);
    fclose(novo);

    remove(ARQ_VENDAS);
    rename(TEMP_VENDAS, ARQ_VENDAS);

    if (encontrado) {
        for (i = 0; i < removida.totalItens; i++) {
            alterarEstoqueProduto(removida.idsProdutos[i], removida.quantidades[i]);
        }

        printf("\nVenda removida com sucesso. O estoque dos produtos foi devolvido.\n");
    } else {
        printf("\nVenda nao encontrada.\n");
    }
}

void relatorioVendasComProdutos() {
    FILE *arquivo = fopen(ARQ_VENDAS, "rb");
    Venda v;
    Produto p;
    int i;
    float totalGeral = 0;

    if (arquivo == NULL) {
        printf("\nNenhuma venda cadastrada.\n");
        return;
    }

    printf("\n--- Relatorio de Vendas com Produtos ---\n");

    while (fread(&v, sizeof(Venda), 1, arquivo) == 1) {
        float totalVenda = 0;

        printf("\nVenda ID: %d\n", v.id);
        printf("Cliente: %s\n", v.cliente);

        for (i = 0; i < v.totalItens; i++) {
            if (buscarProdutoPorId(v.idsProdutos[i], &p, NULL)) {
                float subtotal = p.preco * v.quantidades[i];

                printf("Produto: %s | Quantidade: %d | Preco: R$ %.2f | Subtotal: R$ %.2f\n",
                       p.descricao,
                       v.quantidades[i],
                       p.preco,
                       subtotal);

                totalVenda += subtotal;
            } else {
                printf("Produto ID %d nao encontrado.\n", v.idsProdutos[i]);
            }
        }

        printf("Total da venda: R$ %.2f\n", totalVenda);
        totalGeral += totalVenda;
    }

    printf("\nTotal geral vendido: R$ %.2f\n", totalGeral);

    fclose(arquivo);
}

void menu() {
    int opcao;

    do {
        printf("\n========== MENU ==========\n");
        printf("1 - Cadastrar produto\n");
        printf("2 - Cadastrar venda\n");
        printf("3 - Listar todos os produtos\n");
        printf("4 - Listar todas as vendas\n");
        printf("5 - Buscar produto por ID\n");
        printf("6 - Buscar venda por ID\n");
        printf("7 - Atualizar produto\n");
        printf("8 - Atualizar venda\n");
        printf("9 - Remover produto\n");
        printf("10 - Remover venda\n");
        printf("11 - Relatorio de vendas com nomes dos produtos\n");
        printf("0 - Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                cadastrarProduto();
                break;

            case 2:
                cadastrarVenda();
                break;

            case 3:
                listarProdutos();
                break;

            case 4:
                listarVendas();
                break;

            case 5:
                buscarProdutoEspecifico();
                break;

            case 6:
                buscarVendaEspecifica();
                break;

            case 7:
                atualizarProduto();
                break;

            case 8:
                atualizarVenda();
                break;

            case 9:
                removerProduto();
                break;

            case 10:
                removerVenda();
                break;

            case 11:
                relatorioVendasComProdutos();
                break;

            case 0:
                printf("\nEncerrando o programa...\n");
                break;

            default:
                printf("\nOpcao invalida.\n");
        }

    } while (opcao != 0);
}

int main() {
    menu();
    return 0;
}
