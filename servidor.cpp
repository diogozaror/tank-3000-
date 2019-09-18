#include "PIG.h"

PIG_Evento evento;
PIG_Teclado meuTeclado;

typedef struct c{
    int x, y, xMapa, yMapa;
    float xMouse, yMouse;
    int tipo;
    int vivo;
    int vida;
    int movimento;
    float angle;
    char ip[20], nome[20];
    int porta, id, idSecundario;
    struct c *prox, *ant;
}Cliente;


Cliente *CriaLista(){
    Cliente *inicio;
    inicio = (Cliente *)malloc(sizeof(Cliente));
    inicio->id=-1;
    inicio->porta=-1;
    strcpy(inicio->ip, "null");
    inicio->prox = inicio;
    inicio->ant = inicio;

    return inicio;
}

void InsereFim(Cliente *inicio, char ip[20], int porta, int id, int secundario){
    Cliente *novo = NULL;
    novo = (Cliente *) malloc(sizeof(Cliente));

    strcpy(novo->ip, ip);
    novo->porta = porta;
    novo->id = id;
    novo->idSecundario = secundario;

    novo->prox = inicio;
    novo->ant = inicio->ant;
    inicio->ant->prox = novo;
    inicio->ant = novo;
}

Cliente *Busca(Cliente *inicio, char ip[20], int porta){
    Cliente *aux = inicio->prox;
    while(strcmp(aux->ip, ip)!=0 && aux->porta != porta && aux->prox!=inicio){
        aux = aux->prox;
    }
    if(strcmp(aux->ip, ip)==0 && aux->porta==porta){
        return aux;
    }else{
        return NULL;
    }
}

Cliente *BuscaId(Cliente *inicio, int id){
    Cliente *aux = inicio->prox;
    while(aux->id != id && aux->prox!=inicio){
        aux = aux->prox;
    }
    if(aux->id==id){
        return aux;
    }else{
        return NULL;
    }
}

void Remocao(Cliente *inicio, int id){
    Cliente *aux = BuscaId(inicio, id);
    if(aux == NULL) return;

    aux->ant->prox = aux->prox;
    aux->prox->ant = aux->ant;

    free(aux);
}

void Remocao(Cliente *inicio, char ip[20], int porta){
    Cliente *aux = Busca(inicio, ip, porta);
    if(aux == NULL) return;

    aux->ant->prox = aux->prox;
    aux->prox->ant = aux->ant;

    free(aux);
}

void DestroiLista(Cliente *inicio){
    Cliente *aux, *aux2;
    aux = inicio->prox;
    aux2 = aux->prox;
    while(aux!=inicio){
        free(aux);
        aux = aux2;
        aux2 = aux->prox;
    }
    free(inicio);
}

void DestroiElementos(Cliente *inicio){
    Cliente *aux, *aux2;
    aux = inicio->prox;
    aux2 = aux->prox;
    while(aux!=inicio){
        free(aux);
        aux = aux2;
        aux2 = aux->prox;
    }
}

int colidiuMouseTexto(int xFonte, int yFonte, int tamanhoFonte, int letras, int linhas,  int xMouse, int yMouse){
    int xMax, yMax;
    xMax = xFonte+(letras*(tamanhoFonte/2));
    yMax = yFonte+(linhas*tamanhoFonte);

    if(xMouse >= xFonte && xMouse <= xMax && yMouse >= yFonte && yMouse <= yMax)return 1;

    return 0;
}

void mandarMensagemParaTodos(Cliente *inicio, int socket, char msg[40]){
    Cliente *aux = inicio->prox;
    while(aux!=inicio){
        EnviaDadosSocketServidor(socket, aux->idSecundario, msg, strlen(msg)+1);
        aux = aux->prox;
    }
}

void checkWin(Cliente *inicio, int socket){
    int quantidade = 0;
    char msg2[100];
    Cliente *aux = inicio->prox;
    Cliente *vencedor = NULL;
    while(aux!=inicio){
        if(aux->vivo!=0){
            quantidade++;
            vencedor = aux;
        }
    aux = aux->prox;
    }

    if(quantidade==1){
        sprintf(msg2, "vencedor:%d\0", vencedor->id);
        mandarMensagemParaTodos(inicio, socket, msg2);
    }else{
        sprintf(msg2, "quantidade:%d:%d\0", vencedor->id, quantidade);
        mandarMensagemParaTodos(inicio, socket, msg2);
    }
}

int main( int argc, char* args[] )
{
    Cliente *inicio;
    int fonte1, fonte2, fonte3;
    char str[20];
    int iniciado = 0, mouse1=0, mouse2=0, mouse3=0;
    int portaPadrao = 36664;
    int comecarPartida = 0;
    int socket = 0;
    int bytesEnviados = 0;
    int idDisponivel = 0;

    CriaJogo("Tank Multiplayer 3000 Server");

    meuTeclado = GetTeclado();

    fonte1 = CriaFonteNormal("../fontes/arial.ttf", 30, PRETO, 0, BRANCO, 0, 0);
    fonte2 = CriaFonteNormal("../fontes/arial.ttf", 30, PRETO, 0, BRANCO, ESTILO_SUBLINHADO, 0);
    fonte3 = CriaFonteNormal("../fontes/arial.ttf", 10, PRETO, 0, BRANCO, 0, 0);

    printf("Iniciando configuracoes...\n");

    while(JogoRodando()){
        evento = GetEvento();

        if(evento.tipoEvento==EVENTO_MOUSE){
            if(colidiuMouseTexto(100, 420, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                mouse1=1;
                if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                    //INICIA
                    if(!iniciado){
                        iniciado = 1;
                        inicio = CriaLista();
                        socket = CriaSocketServidor(20, portaPadrao);
                        printf("Ligando servidor...\n");
                    }
                }
            }else if(colidiuMouseTexto(400, 420, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                mouse2=1;
                if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                    //FECHA
                    if(iniciado){
                        iniciado = 0;
                        comecarPartida = 0;
                        idDisponivel = 0;
                        DestroiSocketServidor(socket);
                        DestroiLista(inicio);
                        printf("Desligando servidor...\n");
                    }
                }
            }else if(colidiuMouseTexto(450, 390, 30, 7, 1, evento.mouse.posX, evento.mouse.posY)){
                mouse3=1;
                if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                    //FECHA
                    if(iniciado && !comecarPartida){
                        comecarPartida=1;
                        char msg[40];
                        sprintf(msg, "comecou:\0");
                        mandarMensagemParaTodos(inicio, socket, msg);
                    }
                }
            }else{
                mouse1=0;
                mouse2=0;
                mouse3=0;
            }
        }

        if(evento.tipoEvento == EVENTO_REDE){
            if(evento.rede.tipoMensagem == REDE_CONEXAO){
                InsereFim(inicio, evento.rede.host, evento.rede.porta, idDisponivel, evento.rede.idSecundario);
                printf("Conexao iniciada por: %d <%d>\n", evento.rede.porta, idDisponivel);
                //envia mensagem
                char msg[100];
                if(comecarPartida==0){
                    //envia o id do jogador
                    sprintf(msg, "id:%d\0", idDisponivel);
                    EnviaDadosSocketServidor(socket, evento.rede.idSecundario, msg, strlen(msg)+1);
                }else{
                    //avisa que o servidor ta fechado
                    sprintf(msg, "fechado:\0");
                    EnviaDadosSocketServidor(socket, evento.rede.idSecundario, msg, strlen(msg)+1);
                }
                //aumenta o id

                idDisponivel++;
            }
            if(evento.rede.tipoMensagem == REDE_DESCONEXAO){
                Cliente *aqui = Busca(inicio, evento.rede.host, evento.rede.porta);
                if(aqui!=NULL){
                    char msg[100];
                    sprintf(msg, "desconectado:%d", aqui->id);
                    mandarMensagemParaTodos(inicio, socket, msg);
                    Remocao(inicio, aqui->id);
                }else{
                    Remocao(inicio, evento.rede.host, evento.rede.porta);
                }
                //envia mensagem
                printf("Desconexao por: %d\n", evento.rede.porta);

                checkWin(inicio, socket);
            }
            if(evento.rede.tipoMensagem == REDE_MENSAGEM_TCP){
                char msg[100], copia[100];
                sprintf(msg, "%s", evento.rede.mensagem);
                strcpy(copia, msg);
                int id = -1;
                char *cortada = strtok(msg, ":");
                cortada = strtok(NULL, ":");
                id = atoi(cortada);
                Cliente *aqui = BuscaId(inicio, id);

                if(aqui!=NULL){
                    if(strncmp(msg, "criar", 5)==0){
                        cortada = strtok(NULL, ":");
                        aqui->tipo = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->x = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->y = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->angle = atof(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->movimento = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->vivo = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        strcpy(aqui->nome, cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);

                        //envia pra pessoa que comecou agora todos os players que ja estao na partida
                        char msg2[100];
                        Cliente *aux = inicio->prox;
                        while(aux!=inicio){
                            if(aux!=aqui){
                                sprintf(msg2, "criar:%d:%d:%d:%d:%f:%d:%d:%s\0", aux->id, aux->tipo, aux->x, aux->y, aux->angle, aux->movimento, aux->vivo, aux->nome);
                                EnviaDadosSocketServidor(socket, evento.rede.idSecundario, msg2, strlen(msg2)+1);
                            }
                            aux = aux->prox;
                        }
                    }
                    if(strncmp(msg, "andar", 5)==0){
                        cortada = strtok(NULL, ":");
                        aqui->x = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->y = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->xMapa = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->yMapa = atoi(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->movimento = atoi(cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);
                    }
                    if(strncmp(msg, "topmove", 7)==0){
                        cortada = strtok(NULL, ":");
                        aqui->angle = atof(cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);
                    }
                    if(strncmp(msg, "tiro", 4)==0){
                        cortada = strtok(NULL, ":");
                        aqui->xMouse = atof(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->yMouse = atof(cortada);
                        cortada = strtok(NULL, ":");
                        aqui->tipo = atoi(cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);

                    }
                    if(strncmp(msg, "vida", 4)==0){
                        cortada = strtok(NULL, ":");
                        aqui->vida = atoi(cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);

                    }
                    if(strncmp(msg, "vivo", 4)==0){
                        cortada = strtok(NULL, ":");
                        aqui->vivo = atoi(cortada);

                        mandarMensagemParaTodos(inicio, socket, copia);

                        checkWin(inicio, socket);
                    }
                }
                //handle msg
            }
        }

        IniciaDesenho();

        DesenhaRetangulo(0, 0, 480, 640, CINZA, 0);

        if(iniciado==1){
            if(GetAtivoSocketServidor(socket)){
                char ip[20], fin[30];
                GetHostLocalSocketServidor(socket, ip);
                sprintf(fin, "%s:%d", ip, GetPortaLocalSocketServidor(socket));
                EscreverEsquerda(fin, 150, 380, fonte1);

                DesenhaRetangulo(300, 430, 20, 20, VERDE, 0);
            }else{
                DesenhaRetangulo(300, 430, 20, 20, VERMELHO, 0);
            }
        }else{
            DesenhaRetangulo(300, 430, 20, 20, VERMELHO, 0);
        }

        if(mouse1){
            EscreverEsquerda("Abrir", 100, 420, fonte2);
        }else{
            EscreverEsquerda("Abrir", 100, 420, fonte1);
        }

        if(mouse2){
            EscreverEsquerda("Fechar", 400, 420, fonte2);
        }else{
            EscreverEsquerda("Fechar", 400, 420, fonte1);
        }

        if(comecarPartida==1){
            DesenhaRetangulo(580, 390, 20, 20, VERDE, 0);
        }else{
            DesenhaRetangulo(580, 390, 20, 20, VERMELHO, 0);
        }

        if(mouse3){
            EscreverEsquerda("Começar", 450, 390, fonte2);
        }else{
            EscreverEsquerda("Começar", 450, 390, fonte1);
        }

        if(iniciado){
            Cliente *aux = inicio->prox;
            int linha=0, coluna=0;
            while(aux!=inicio){
                if(linha<=5){
                    char textoAux[30];
                    sprintf(textoAux, "IP: %s", aux->ip);
                    DesenhaRetangulo(10+ 200*linha, 5 + 50*coluna, 50, 90, BRANCO, 0);
                    EscreverEsquerda(textoAux, 20 + 200*linha, 30 + 55*coluna, fonte3);
                    sprintf(textoAux, "PORTA: %d", aux->porta);
                    EscreverEsquerda(textoAux, 20 + 200*linha, 20 + 55*coluna, fonte3);
                    sprintf(textoAux, "ID: %d", aux->id);
                    EscreverEsquerda(textoAux, 20 + 200*linha, 10 + 55*coluna, fonte3);
                    linha++;
                }else{
                    coluna++;
                    linha=0;
                }
                aux = aux->prox;
            }
        }

        EncerraDesenho();
    }

    DestroiSocketServidor(socket);
    DestroiLista(inicio);

    FinalizaJogo();
    return 0;
}
