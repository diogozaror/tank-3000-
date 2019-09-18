#include "PIG.h"

PIG_Evento evento;
PIG_Teclado meuTeclado;

time_t t = time(NULL);
struct tm tm = *localtime(&t);

float grausParaRadianos(float angulo){
    return ((angulo*M_PI)/180);
}

float radianosParaGraus(float angulo){
    return ((angulo*180)/M_PI);
}

typedef struct objetoCenario{
    int xInicial, yInicial, tamanhoX, tamanhoY, objId;
}ObjetoCenario;

typedef struct objetoPrincipal{
    int vida, vidaTotal, municao, velocidade, dano, lastX, lastY, vivo;
    float tempoRecarga, tempoReload, auxRecarga;
}ObjetoPrincipal;

typedef struct objetoInimigo{
    int objId, objTopoId;
    int x, y;
    ObjetoPrincipal principal;
    int tipo;
    int vivo;
    int movimento;
    int seMovimentando;
    int vendoInimigo;
    int fugindo;
    int mudarDirecao;
    int andandoEmX;
    int andandoPositivo;
    float angleAndando;
    float recarregando;
    float auxAndar;
    float auxMudarDirecao;
    float auxMudarDirecao2;
    int ultimoVistoX;
    int ultimoVistoY;

}ObjetoInimigo;

typedef struct objetoAdversario{
    int objId, objTopoId, idAdversario;
    int x, y, xMapa, yMapa;
    ObjetoPrincipal principal;
    float angle;
    int tipo;
    int vivo;
    int movimento;
    char nome[20];

    struct objetoAdversario *prox, *ant;

}ObjetoAdversario;

typedef struct coordenadas{
    int x, y;
}Coordenadas;

typedef struct caixa{
    int x;
    int y;
    int usada;
}Caixa;

ObjetoAdversario *CriaLista(){
    ObjetoAdversario *inicio;
    inicio = (ObjetoAdversario *)malloc(sizeof(ObjetoAdversario));
    inicio->idAdversario=-1;
    inicio->prox = inicio;
    inicio->ant = inicio;

    return inicio;
}

void InsereFim(ObjetoAdversario *inicio, int objId, int objTopoId, int idAdversario, int x, int y, float angle, ObjetoPrincipal principal, int tipo, int vivo, int movimento, char nome[20]){
    ObjetoAdversario *novo = NULL;
    novo = (ObjetoAdversario *) malloc(sizeof(ObjetoAdversario));

    novo->objId = objId;
    novo->objTopoId = objTopoId;
    novo->idAdversario = idAdversario;
    novo->x = x;
    novo->y = y;
    novo->angle = angle;
    novo->principal = principal;
    novo->vivo = vivo;
    novo->movimento = movimento;
    strcpy(novo->nome, nome);

    novo->prox = inicio;
    novo->ant = inicio->ant;
    inicio->ant->prox = novo;
    inicio->ant = novo;
}

ObjetoAdversario *Busca(ObjetoAdversario *inicio, int id){
    ObjetoAdversario *aux = inicio->prox;
    while(aux->idAdversario !=id && aux!=inicio){
        aux = aux->prox;
    }
    if(aux->idAdversario==id){
        return aux;
    }else{
        return NULL;
    }
}

void Remocao(ObjetoAdversario *inicio, int id){
    ObjetoAdversario *aux = Busca(inicio, id);
    if(aux == NULL) return;

    aux->ant->prox = aux->prox;
    aux->prox->ant = aux->ant;

    free(aux);
}

void DestroiLista(ObjetoAdversario *inicio){
    ObjetoAdversario *aux, *aux2;
    aux = inicio->prox;
    aux2 = aux->prox;
    while(aux!=inicio){
        free(aux);
        aux = aux2;
        aux2 = aux->prox;
    }
    free(inicio);
}

void DestroiElementos(ObjetoAdversario *inicio){
    ObjetoAdversario *aux, *aux2;
    aux = inicio->prox;
    aux2 = aux->prox;
    while(aux!=inicio){
        free(aux);
        aux = aux2;
        aux2 = aux->prox;
    }
}

int calculaQuantidadeInimigos(int fase){
    return (int)floor((log(fase)*(fase)/2)+1);
}

PIG_Cor corVida(int vida, int vidaMaxima, int transparencia){
    PIG_Cor cor = BRANCO;
    float porcentagem = (float)vida/vidaMaxima;
    if(porcentagem>0.5){
        cor.r = 255-(510*porcentagem);
        cor.g = 255;
        cor.b = 0;
        cor.a = transparencia;
    }else{
        cor.r = 255;
        cor.g = 510*(porcentagem+0.5);
        cor.b = 0;
        cor.a = transparencia;
    }

    return cor;
}

void desenhaVida(int x, int y, int altura, int largura, int vida, int vidaTotal, int transparencia, int janela=0){
    DesenhaRetangulo(x, y, altura, ((float)vida/vidaTotal)*largura, corVida(vida, vidaTotal, transparencia), janela);
}

void desenhaMiniMapa(int posicaoX, int posicaoY, int tamanho, ObjetoCenario objCenario[], ObjetoInimigo *objInimigo, int quantidadeInimigo, int xPlayer, int yPlayer, int xMapa, int yMapa, int xCaixa, int yCaixa, int dificuldade, int transparencia){
    int i;
    PIG_Cor corFundo;
    corFundo.a = transparencia;
    corFundo.r = 150;
    corFundo.g = 150;
    corFundo.b = 150;

    PIG_Cor corPreto;
    corPreto.a = transparencia;
    corPreto.r = 0;
    corPreto.g = 0;
    corPreto.b = 0;

    PIG_Cor corLaranja;
    corLaranja.a = transparencia;
    corLaranja.r = 255;
    corLaranja.g = 128;
    corLaranja.b = 0;

    PIG_Cor corVermelho;
    corVermelho.a = transparencia;
    corVermelho.r = 255;
    corVermelho.g = 0;
    corVermelho.b = 0;

    PIG_Cor corVerde;
    corVerde.a = transparencia;
    corVerde.r = 0;
    corVerde.g = 255;
    corVerde.b = 0;

    PIG_Cor corAmarela;
    corAmarela.a = transparencia;
    corAmarela.r = 255;
    corAmarela.g = 255;
    corAmarela.b = 0;

    DesenhaRetangulo(posicaoX-5, posicaoY-5, tamanho+10, tamanho+10, corPreto);
    DesenhaRetangulo(posicaoX, posicaoY, tamanho, tamanho, corFundo);

    for(i=6;i<24;i++){
        DesenhaRetangulo(posicaoX+((objCenario[i].xInicial)/(tamanho/5)), posicaoY+((objCenario[i].yInicial)/(tamanho/5)), (objCenario[i].tamanhoY/(tamanho/5)), (objCenario[i].tamanhoX/(tamanho/5)), corLaranja);
    }

    if(dificuldade!=3){
        for(i=0;i<quantidadeInimigo;i++){
            if(objInimigo[i].vivo)
            DesenhaRetangulo(posicaoX+((objInimigo[i].x)/(tamanho/5)), posicaoY+((objInimigo[i].y)/(tamanho/5)), 5, 5, corVermelho);
        }
    }

    DesenhaRetangulo(posicaoX+((xCaixa)/(tamanho/5)), posicaoY+((yCaixa)/(tamanho/5)), 2, 2, corAmarela);
    DesenhaRetangulo(posicaoX+((xPlayer-xMapa)/(tamanho/5)), posicaoY+((yPlayer-yMapa)/(tamanho/5)), 5, 5, corVerde);
}

void desenhaMiniMapa(int posicaoX, int posicaoY, int tamanho, ObjetoCenario objCenario[], int xPlayer, int yPlayer, int xMapa, int yMapa, int xCaixa, int yCaixa, int transparencia, ObjetoAdversario *inicio){
    int i;
    PIG_Cor corFundo;
    corFundo.a = transparencia;
    corFundo.r = 150;
    corFundo.g = 150;
    corFundo.b = 150;

    PIG_Cor corPreto;
    corPreto.a = transparencia;
    corPreto.r = 0;
    corPreto.g = 0;
    corPreto.b = 0;

    PIG_Cor corLaranja;
    corLaranja.a = transparencia;
    corLaranja.r = 255;
    corLaranja.g = 128;
    corLaranja.b = 0;

    PIG_Cor corVermelho;
    corVermelho.a = transparencia;
    corVermelho.r = 255;
    corVermelho.g = 0;
    corVermelho.b = 0;

    PIG_Cor corVerde;
    corVerde.a = transparencia;
    corVerde.r = 0;
    corVerde.g = 255;
    corVerde.b = 0;

    PIG_Cor corAmarela;
    corAmarela.a = transparencia;
    corAmarela.r = 255;
    corAmarela.g = 255;
    corAmarela.b = 0;

    DesenhaRetangulo(posicaoX-5, posicaoY-5, tamanho+10, tamanho+10, corPreto);
    DesenhaRetangulo(posicaoX, posicaoY, tamanho, tamanho, corFundo);

    for(i=6;i<24;i++){
        DesenhaRetangulo(posicaoX+((objCenario[i].xInicial)/(tamanho/5)), posicaoY+((objCenario[i].yInicial)/(tamanho/5)), (objCenario[i].tamanhoY/(tamanho/5)), (objCenario[i].tamanhoX/(tamanho/5)), corLaranja);
    }

    ObjetoAdversario *aux = inicio->prox;
    while(aux!=inicio){
        if(aux->vivo)
        DesenhaRetangulo(posicaoX+((aux->x)/(tamanho/5)), posicaoY+((aux->y)/(tamanho/5)), 5, 5, corVermelho);

        aux=aux->prox;
    }


    DesenhaRetangulo(posicaoX+((xCaixa)/(tamanho/5)), posicaoY+((yCaixa)/(tamanho/5)), 2, 2, corAmarela);
    DesenhaRetangulo(posicaoX+((xPlayer-xMapa)/(tamanho/5)), posicaoY+((yPlayer-yMapa)/(tamanho/5)), 5, 5, corVerde);
}

int colidiuMouseTexto(int xFonte, int yFonte, int tamanhoFonte, int letras, int linhas,  int xMouse, int yMouse){
    int xMax, yMax;
    xMax = xFonte+(letras*(tamanhoFonte/2));
    yMax = yFonte+(linhas*tamanhoFonte);

    if(xMouse >= xFonte && xMouse <= xMax && yMouse >= yFonte && yMouse <= yMax)return 1;

    return 0;
}

int colisaoObjetos(ObjetoCenario objCenario[], int obj){
    int colidiu = 0;
    for(int i=0;i<24;i++){
        if(TestaColisaoObjetos(objCenario[i].objId,obj))
            colidiu=1;
    }

    return colidiu;
}

float normalizarVetorX(float x, float y, float escalar){
    float norma = sqrt(pow(x,2)+pow(y,2));

    return (x/norma)*escalar;
}

float normalizarVetorY(float x, float y, float escalar){
    float norma = sqrt(pow(x,2)+pow(y,2));

    return (y/norma)*escalar;
}

void setClasseReload(ObjetoPrincipal *objPrincipal){
    objPrincipal->vivo = 1;
    objPrincipal->vida=2500; //CLASSE RELOAD
    objPrincipal->vidaTotal = 2500;
    objPrincipal->municao = 20;
    objPrincipal->velocidade=5;
    objPrincipal->dano=125;
    objPrincipal->tempoRecarga=0.6;
    objPrincipal->tempoReload = 3;
}

void setClasseKnife(ObjetoPrincipal *objPrincipal){
    objPrincipal->vivo = 1;
    objPrincipal->vida=2000; //CLASSE KNIFE
    objPrincipal->vidaTotal = 2000;
    objPrincipal->municao = 20;
    objPrincipal->velocidade=6;
    objPrincipal->dano=250;
    objPrincipal->tempoRecarga=2.0;
    objPrincipal->tempoReload=10;
}

void setClasseTank(ObjetoPrincipal *objPrincipal){
    objPrincipal->vivo = 1;
    objPrincipal->vida=5000; //CLASSE TANK
    objPrincipal->vidaTotal = 5000;
    objPrincipal->municao = 20;
    objPrincipal->velocidade=4;
    objPrincipal->dano=100;
    objPrincipal->tempoRecarga=0.9;
    objPrincipal->tempoReload=4;
}

void setClasseSpeed(ObjetoPrincipal *objPrincipal){
    objPrincipal->vivo = 1;
    objPrincipal->vida=2500; //CLASSE SPEED
    objPrincipal->vidaTotal = 2500;
    objPrincipal->municao = 20;
    objPrincipal->velocidade=10;
    objPrincipal->dano=150;
    objPrincipal->tempoRecarga=1.5;
    objPrincipal->tempoReload=7;
}

int estaDentroDoCenario(ObjetoCenario objcenario, int x, int y, int xMapa, int yMapa){
    int xMin = objcenario.xInicial+xMapa;
    int yMin = objcenario.yInicial+yMapa;
    int xMax = xMin+objcenario.tamanhoX;
    int yMax = yMin+objcenario.tamanhoY;
    if(x >= xMin && x <= xMax && y >= yMin && y <= yMax)return 1;

    return 0;
}

int obstaculoEntrePlayerInimigo(int xPlayer, int yPlayer, int xMapa, int yMapa, int xw, int yw, ObjetoCenario objcenario){
    int resp=0;
    float i;
    for(i=0;i<=1;i+=0.01){
        int xAtual = xPlayer+(xw*i);
        int yAtual = yPlayer+(yw*i);
        if(estaDentroDoCenario(objcenario, xAtual, yAtual, xMapa, yMapa)){
            resp=1;
            break;
        }
    }
    return resp;
}

int posicaoLivre(ObjetoCenario objCenario[], int x, int y, int xMapa, int yMapa){
    int resp=1;
    x+=32;
    y+=32;
    for(int i=0;i<24;i++){
        if(estaDentroDoCenario(objCenario[i], x, y, 0, 0)){
            resp=0;
            break;
        }

    }
    if(!(x>=0 && x<=2048 && y>=0 && y<=2048)){
        resp = 0;
    }

    return resp;
}

Coordenadas getCoordenadasAleatorias(ObjetoInimigo *objinimigo, int quantidade, ObjetoCenario objcenario[]){
    Coordenadas a;
    int x, y, i, usado=0;
    do{
        usado=0;
        x = rand()%2048;
        y = rand()%2048;
        for(i=0;i<24;i++){
            if(x>= objcenario[i].xInicial && x<= objcenario[i].xInicial+objcenario[i].tamanhoX && y>=objcenario[i].yInicial && y<=(objcenario[i].yInicial+objcenario[i].tamanhoY))usado=1;
            if(x+64>= objcenario[i].xInicial && x+64<= objcenario[i].xInicial+objcenario[i].tamanhoX+6 && y+64>=objcenario[i].yInicial && y+64<=(objcenario[i].yInicial+objcenario[i].tamanhoY))usado=1;
        }
        for(i=0;i<quantidade;i++){
            if((x>= objinimigo[i].x && x<=objinimigo[i].x+64 && y>=objinimigo[i].y && y<=objinimigo[i].y+64))usado=1;
        }
    }while(usado==1);

    a.x = x;
    a.y = y;

    return a;
}

Coordenadas getCoordenadasAleatorias(ObjetoCenario objcenario[]){
    Coordenadas a;
    int x, y, i, usado=0;
    do{
        usado=0;
        x = rand()%2020;
        y = rand()%2020;
        for(i=0;i<24;i++){
            if(x>= objcenario[i].xInicial && x<= objcenario[i].xInicial+objcenario[i].tamanhoX && y>=objcenario[i].yInicial && y<=(objcenario[i].yInicial+objcenario[i].tamanhoY))usado=1;
            if(x+64>= objcenario[i].xInicial && x+64<= objcenario[i].xInicial+objcenario[i].tamanhoX+6 && y+64>=objcenario[i].yInicial && y+64<=(objcenario[i].yInicial+objcenario[i].tamanhoY))usado=1;
        }
    }while(usado==1);

    a.x = x;
    a.y = y;

    return a;
}

void iniciarCenario(ObjetoCenario objCenario[]){
    objCenario[0].objId = CriaObjeto("../imagens/barreira1.png", 1, 255, 0);
    objCenario[0].xInicial = 110;
    objCenario[0].yInicial = 25;
    objCenario[0].tamanhoX = 186;
    objCenario[0].tamanhoY = 15;
    objCenario[1].objId = CriaObjeto("../imagens/barreira2.png", 1, 255, 0);
    objCenario[1].xInicial = 1120;
    objCenario[1].yInicial = 25;
    objCenario[1].tamanhoX = 159;
    objCenario[1].tamanhoY = 15;
    objCenario[2].objId = CriaObjeto("../imagens/barreira3.png", 1, 255, 0);
    objCenario[2].xInicial = 2017;
    objCenario[2].yInicial = 560;
    objCenario[2].tamanhoX = 15;
    objCenario[2].tamanhoY = 95;
    objCenario[3].objId = CriaObjeto("../imagens/barreira4.png", 1, 255, 0);
    objCenario[3].xInicial = 2017;
    objCenario[3].yInicial = 1265;
    objCenario[3].tamanhoX = 15;
    objCenario[3].tamanhoY = 118;
    objCenario[4].objId = CriaObjeto("../imagens/barreira5.png", 1, 255, 0);
    objCenario[4].xInicial = 1765;
    objCenario[4].yInicial = 2010;
    objCenario[4].tamanhoX = 186;
    objCenario[4].tamanhoY = 15;
    objCenario[5].objId = CriaObjeto("../imagens/barreira6.png", 1, 255, 0);
    objCenario[5].xInicial = 765;
    objCenario[5].yInicial = 2010;
    objCenario[5].tamanhoX = 175;
    objCenario[5].tamanhoY = 15;
    objCenario[6].objId = CriaObjeto("../imagens/barreira7.png", 1, 255, 0);
    objCenario[6].xInicial = 5;
    objCenario[6].yInicial = 1315;
    objCenario[6].tamanhoX = 15;
    objCenario[6].tamanhoY = 174;
    objCenario[7].objId = CriaObjeto("../imagens/predio1.png", 0, 255, 0);
    objCenario[7].xInicial = 305;
    objCenario[7].yInicial = 248;
    objCenario[7].tamanhoX = 300;
    objCenario[7].tamanhoY = 300;
    objCenario[8].objId = CriaObjeto("../imagens/predio2.png", 0, 255, 0);
    objCenario[8].xInicial = 0;
    objCenario[8].yInicial = 50;
    objCenario[8].tamanhoX = 100;
    objCenario[8].tamanhoY = 1253;
    objCenario[9].objId = CriaObjeto("../imagens/predio3.png", 0, 255, 0);
    objCenario[9].xInicial = 800;
    objCenario[9].yInicial = 248;
    objCenario[9].tamanhoX = 300;
    objCenario[9].tamanhoY = 300;
    objCenario[10].objId = CriaObjeto("../imagens/predio4.png", 0, 255, 0);
    objCenario[10].xInicial = 1300;
    objCenario[10].yInicial = 248;
    objCenario[10].tamanhoX = 500;
    objCenario[10].tamanhoY = 300;
    objCenario[11].objId = CriaObjeto("../imagens/predio5.png", 0, 255, 0);
    objCenario[11].xInicial = 300;
    objCenario[11].yInicial = 750;
    objCenario[11].tamanhoX = 300;
    objCenario[11].tamanhoY = 540;
    objCenario[12].objId = CriaObjeto("../imagens/predio6.png", 0, 255, 0);
    objCenario[12].xInicial = 1497;
    objCenario[12].yInicial = 750;
    objCenario[12].tamanhoX = 290;
    objCenario[12].tamanhoY = 500;
    objCenario[13].objId = CriaObjeto("../imagens/predio7.png", 0, 255, 0);
    objCenario[13].xInicial = 248;
    objCenario[13].yInicial = 1500;
    objCenario[13].tamanhoX = 500;
    objCenario[13].tamanhoY = 300;
    objCenario[14].objId = CriaObjeto("../imagens/predio8.png", 0, 255, 0);
    objCenario[14].xInicial = 950;
    objCenario[14].yInicial = 1495;
    objCenario[14].tamanhoX = 300;
    objCenario[14].tamanhoY = 300;
    objCenario[15].objId = CriaObjeto("../imagens/predio9.png", 0, 255, 0);
    objCenario[15].xInicial = 1448;
    objCenario[15].yInicial = 1493;
    objCenario[15].tamanhoX = 300;
    objCenario[15].tamanhoY = 300;
    objCenario[16].objId = CriaObjeto("../imagens/predio10.png", 0, 255, 0);
    objCenario[16].xInicial = 0;
    objCenario[16].yInicial = 1497;
    objCenario[16].tamanhoX = 48;
    objCenario[16].tamanhoY = 551;
    objCenario[17].objId = CriaObjeto("../imagens/predio11.png", 0, 255, 0);
    objCenario[17].xInicial = 7;
    objCenario[17].yInicial = 1997;
    objCenario[17].tamanhoX = 754;
    objCenario[17].tamanhoY = 50;
    objCenario[18].objId = CriaObjeto("../imagens/predio12.png", 0, 255, 0);
    objCenario[18].xInicial = 949;
    objCenario[18].yInicial = 1994;
    objCenario[18].tamanhoX = 802;
    objCenario[18].tamanhoY = 48;
    objCenario[19].objId = CriaObjeto("../imagens/predio13.png", 0, 255, 0);
    objCenario[19].xInicial = 1947;
    objCenario[19].yInicial = 1390;
    objCenario[19].tamanhoX = 102;
    objCenario[19].tamanhoY = 603;
    objCenario[20].objId = CriaObjeto("../imagens/predio14.png", 0, 255, 0);
    objCenario[20].xInicial = 1972;
    objCenario[20].yInicial = 660;
    objCenario[20].tamanhoX = 77;
    objCenario[20].tamanhoY = 594;
    objCenario[21].objId = CriaObjeto("../imagens/predio15.png", 0, 255, 0);
    objCenario[21].xInicial = 2000;
    objCenario[21].yInicial = 1;
    objCenario[21].tamanhoX = 50;
    objCenario[21].tamanhoY = 552;
    objCenario[22].objId = CriaObjeto("../imagens/predio16.png", 0, 255, 0);
    objCenario[22].xInicial = 1296;
    objCenario[22].yInicial = 3;
    objCenario[22].tamanhoX = 706;
    objCenario[22].tamanhoY = 42;
    objCenario[23].objId = CriaObjeto("../imagens/predio17.png", 0, 255, 0);
    objCenario[23].xInicial = 300;
    objCenario[23].yInicial = 1;
    objCenario[23].tamanhoX = 803;
    objCenario[23].tamanhoY = 50;

    for(int i=0;i<24;i++){
        MoveObjeto(objCenario[i].objId, objCenario[i].xInicial, objCenario[i].yInicial);
    }
}

void iniciaInimigos(ObjetoInimigo **inimigos, int quantidade, ObjetoCenario objcenario[]){
    *inimigos = (ObjetoInimigo *) calloc(quantidade, sizeof(ObjetoInimigo));
    int random = 0, i=0;
    for(i=0;i<quantidade;i++){
        random = rand()%4;
        if(random==0){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_1.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top1.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseTank(&((*inimigos)[i].principal));
            (*inimigos)[i].x = 0;
            (*inimigos)[i].y = 0;
            (*inimigos)[i].tipo = 1;
            (*inimigos)[i].vivo = 1;
            (*inimigos)[i].andandoEmX = 1;
            (*inimigos)[i].andandoPositivo = 1;
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = 0;
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else if(random==1){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_2.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top2.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseSpeed(&((*inimigos)[i].principal));
            (*inimigos)[i].x = 0;
            (*inimigos)[i].y = 0;
            (*inimigos)[i].tipo = 2;
            (*inimigos)[i].vivo = 1;
            (*inimigos)[i].andandoEmX = 1;
            (*inimigos)[i].andandoPositivo = 1;
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = 0;
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else if(random==2){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_3.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top3.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseKnife(&((*inimigos)[i].principal));
            (*inimigos)[i].x = 0;
            (*inimigos)[i].y = 0;
            (*inimigos)[i].tipo = 3;
            (*inimigos)[i].vivo = 1;
            (*inimigos)[i].andandoEmX = 1;
            (*inimigos)[i].andandoPositivo = 1;
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = 0;
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else{
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_4.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top4.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseReload(&((*inimigos)[i].principal));
            (*inimigos)[i].x = 0;
            (*inimigos)[i].y = 0;
            (*inimigos)[i].tipo = 4;
            (*inimigos)[i].vivo = 1;
            (*inimigos)[i].andandoEmX = 1;
            (*inimigos)[i].andandoPositivo = 1;
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = 0;
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }
    }

    for(i=0;i<quantidade;i++){
        Coordenadas coor = getCoordenadasAleatorias(*inimigos, quantidade, objcenario);
        (*inimigos)[i].x = coor.x;
        (*inimigos)[i].y = coor.y;
        MoveObjeto((*inimigos)[i].objId, (*inimigos)[i].x, (*inimigos)[i].y);
        MoveObjeto((*inimigos)[i].objTopoId, (*inimigos)[i].x, (*inimigos)[i].y);
    }
}

void iniciaInimigosCarregados(ObjetoInimigo **inimigos, int matriz[][7], int quantidade){
    *inimigos = (ObjetoInimigo *) calloc(quantidade, sizeof(ObjetoInimigo));
    int i=0;
    for(i=0;i<quantidade;i++){
        if(matriz[i][4]==100){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_1.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top1.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseTank(&((*inimigos)[i].principal));
            (*inimigos)[i].x = matriz[i][0];
            (*inimigos)[i].y = matriz[i][1];
            (*inimigos)[i].tipo = 1;
            (*inimigos)[i].vivo = matriz[i][5];
            (*inimigos)[i].andandoEmX = matriz[i][2];
            (*inimigos)[i].andandoPositivo = matriz[i][3];
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = matriz[i][6];
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else if(matriz[i][4]==150){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_2.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top2.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseSpeed(&((*inimigos)[i].principal));
            (*inimigos)[i].x = matriz[i][0];
            (*inimigos)[i].y = matriz[i][1];
            (*inimigos)[i].tipo = 2;
            (*inimigos)[i].vivo = matriz[i][5];
            (*inimigos)[i].andandoEmX = matriz[i][2];
            (*inimigos)[i].andandoPositivo = matriz[i][3];
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = matriz[i][6];
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else if(matriz[i][4]==250){
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_3.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top3.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseKnife(&((*inimigos)[i].principal));
            (*inimigos)[i].x = matriz[i][0];
            (*inimigos)[i].y = matriz[i][1];
            (*inimigos)[i].tipo = 3;
            (*inimigos)[i].vivo = matriz[i][5];
            (*inimigos)[i].andandoEmX = matriz[i][2];
            (*inimigos)[i].andandoPositivo = matriz[i][3];
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = matriz[i][6];
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }else{
            (*inimigos)[i].objId = CriaObjeto("../imagens/tank_base_4.png", 1, 255, 0);
            (*inimigos)[i].objTopoId = CriaObjeto("../imagens/tank_top4.png", 1, 255, 0);
            DefineFrameObjeto((*inimigos)[i].objId, 0, 0, 64, 64);
            SetDimensoesObjeto((*inimigos)[i].objId, 64, 64);
            SetPivoObjeto((*inimigos)[i].objId, 0.5F, 0.5F);
            SetPivoObjeto((*inimigos)[i].objTopoId, 0.5F, 0.5F);
            setClasseReload(&((*inimigos)[i].principal));
            (*inimigos)[i].x = matriz[i][0];
            (*inimigos)[i].y = matriz[i][1];
            (*inimigos)[i].tipo = 4;
            (*inimigos)[i].vivo = matriz[i][5];
            (*inimigos)[i].andandoEmX = matriz[i][2];
            (*inimigos)[i].andandoPositivo = matriz[i][3];
            (*inimigos)[i].fugindo = 0;
            (*inimigos)[i].angleAndando = 0;
            (*inimigos)[i].mudarDirecao = 0;
            (*inimigos)[i].recarregando = 0;
            (*inimigos)[i].seMovimentando = 0;
            (*inimigos)[i].vendoInimigo = matriz[i][6];
            (*inimigos)[i].movimento = 0;
            (*inimigos)[i].auxAndar = 0;
            (*inimigos)[i].auxMudarDirecao = 0;
            (*inimigos)[i].auxMudarDirecao2 = 0;
        }
    }

    for(i=0;i<quantidade;i++){
        MoveObjeto((*inimigos)[i].objId, (*inimigos)[i].x, (*inimigos)[i].y);
        MoveObjeto((*inimigos)[i].objTopoId, (*inimigos)[i].x, (*inimigos)[i].y);
    }
}

void iniciaDemonstracao(int objDemonstracao[]){
    objDemonstracao[1] = CriaObjeto("../imagens/tank_base1.png", 1, 255, 0);
    objDemonstracao[0] = CriaObjeto("../imagens/tank_base2.png", 1, 255, 0);
    objDemonstracao[2] = CriaObjeto("../imagens/tank_base3.png", 1, 255, 0);
    objDemonstracao[3] = CriaObjeto("../imagens/tank_base4.png", 1, 255, 0);
    objDemonstracao[5] = CriaObjeto("../imagens/tank_top1.png", 1, 255, 0);
    objDemonstracao[4] = CriaObjeto("../imagens/tank_top2.png", 1, 255, 0);
    objDemonstracao[6] = CriaObjeto("../imagens/tank_top3.png", 1, 255, 0);
    objDemonstracao[7] = CriaObjeto("../imagens/tank_top4.png", 1, 255, 0);

    MoveObjeto(objDemonstracao[0], 50, 400);
    MoveObjeto(objDemonstracao[1], 250, 400);
    MoveObjeto(objDemonstracao[2], 450, 400);
    MoveObjeto(objDemonstracao[3], 650, 400);
    MoveObjeto(objDemonstracao[4], 50, 400);
    MoveObjeto(objDemonstracao[5], 250, 400);
    MoveObjeto(objDemonstracao[6], 450, 400);
    MoveObjeto(objDemonstracao[7], 650, 400);

    SetPivoObjeto(objDemonstracao[0], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[1], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[2], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[3], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[4], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[5], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[6], 0.5f, 0.5f);
    SetPivoObjeto(objDemonstracao[7], 0.5f, 0.5f);
}

void destruirInimigos(ObjetoInimigo **inimigos){
    free(*inimigos);
}

void seguirAlgo(ObjetoInimigo *inimigo, int x, int y, int _xMapa, int _yMapa, ObjetoCenario objCenario[]){
    if((*inimigo).auxAndar<=0){
        (*inimigo).auxAndar = 0.05;
        int andarX = (*inimigo).x;
        int andarY = (*inimigo).y;
        int offsetX = x;
        int offsetY = y;
        if((*inimigo).auxMudarDirecao<=0){
            (*inimigo).auxMudarDirecao=1;
            (*inimigo).andandoEmX = rand()%2;
        }
        if((*inimigo).andandoEmX){
            if((offsetX-andarX)>(*inimigo).principal.velocidade){
                if(posicaoLivre(objCenario, ((*inimigo).x+32)+(*inimigo).principal.velocidade, (*inimigo).y, _xMapa, _yMapa)){
                    (*inimigo).andandoEmX=1;
                    (*inimigo).andandoPositivo=1;
                    (*inimigo).x+=(*inimigo).principal.velocidade;
                }else{
                    (*inimigo).andandoEmX=0;
                }
                 }else if((offsetX-andarX)<-(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, ((*inimigo).x-32)-(*inimigo).principal.velocidade, (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).x-=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=0;
                    }
                }else{
                    (*inimigo).andandoEmX=0;
                }
            }else{
                if((offsetY-andarY)>(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+(*inimigo).principal.velocidade, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).y+=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=1;
                    }
                }else if((offsetY-andarY)<-(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-(*inimigo).principal.velocidade, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).y-=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=1;
                    }
                }else{
                    (*inimigo).andandoEmX=1;
            }
        }
    }
}

void seguirPlayer(ObjetoInimigo *inimigo, int _x, int _y, int _xMapa, int _yMapa, ObjetoCenario objCenario[]){
    seguirAlgo(inimigo, _x-_xMapa, _y-_yMapa, _xMapa, _yMapa, objCenario);
}

void seguirUltimoVisto(ObjetoInimigo *inimigo, int _x, int _y, int _xMapa, int _yMapa, ObjetoCenario objCenario[]){
    seguirAlgo(inimigo, _x, _y, _xMapa, _yMapa, objCenario);
}

void fugirPlayer(ObjetoInimigo *inimigo, int _x, int _y, int _xMapa, int _yMapa, ObjetoCenario objCenario[]){
    if((*inimigo).auxAndar<=0){
        (*inimigo).auxAndar = 0.05;
        int andarX = (*inimigo).x;
        int andarY = (*inimigo).y;
        int offsetX = _x-_xMapa;
        int offsetY = _y-_yMapa;
        if((*inimigo).auxMudarDirecao<=0){
            (*inimigo).auxMudarDirecao=1;
            (*inimigo).andandoEmX = rand()%2;
        }
        if((*inimigo).andandoEmX){
            if((offsetX-andarX)>(*inimigo).principal.velocidade){
                if(posicaoLivre(objCenario, ((*inimigo).x-32)-(*inimigo).principal.velocidade, (*inimigo).y, _xMapa, _yMapa)){
                    (*inimigo).andandoEmX=1;
                    (*inimigo).andandoPositivo=1;
                    (*inimigo).x-=(*inimigo).principal.velocidade;
                }else{
                    (*inimigo).andandoEmX=0;
                }
                 }else if((offsetX-andarX)<-(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, ((*inimigo).x+32)+(*inimigo).principal.velocidade, (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).x+=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=0;
                    }
                }else{
                    (*inimigo).andandoEmX=0;
                }
            }else{
                if((offsetY-andarY)>(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-(*inimigo).principal.velocidade, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).y-=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=1;
                    }
                }else if((offsetY-andarY)<-(*inimigo).principal.velocidade){
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+(*inimigo).principal.velocidade, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).y+=(*inimigo).principal.velocidade;
                    }else{
                        (*inimigo).andandoEmX=1;
                    }
                }else{
                    (*inimigo).andandoEmX=1;
            }
        }
    }
}

void andarAleatoriamente(ObjetoInimigo *inimigo, int _xMapa, int _yMapa, ObjetoCenario objCenario[]){
    if((*inimigo).auxMudarDirecao2<=0){
        (*inimigo).auxMudarDirecao2=5;
        if((rand()%10)<4){
            (*inimigo).andandoEmX = rand()%2;
            (*inimigo).andandoPositivo = rand()%2;
        }
    }
    if((*inimigo).auxAndar<=0){
        (*inimigo).auxAndar = 0.05;
        if((*inimigo).andandoEmX){
            if((*inimigo).andandoPositivo){
                if(posicaoLivre(objCenario, ((*inimigo).x+32)+((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                    (*inimigo).x+=(*inimigo).principal.velocidade;
                }else{
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).y+=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).y-=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, ((*inimigo).x-32)-((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).x-=((*inimigo).principal.velocidade);
                    }
                }
            }else{
                if(posicaoLivre(objCenario, ((*inimigo).x-32)-((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                    (*inimigo).x-=((*inimigo).principal.velocidade);
                }else{
                    if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).y+=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).y+=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, ((*inimigo).x+32)+((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).x+=((*inimigo).principal.velocidade);
                    }
                }
            }
        }else{
            if((*inimigo).andandoPositivo){
                if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                    (*inimigo).y+=((*inimigo).principal.velocidade);
                }else{
                    if(posicaoLivre(objCenario, ((*inimigo).x+32)+((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).x+=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, ((*inimigo).x-32)-((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).x-=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).y-=((*inimigo).principal.velocidade);
                    }
                }
            }else{
                if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y-32)-((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                    (*inimigo).y-=((*inimigo).principal.velocidade);
                }else{
                    if(posicaoLivre(objCenario, ((*inimigo).x+32)+((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).x+=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, ((*inimigo).x-32)-((*inimigo).principal.velocidade), (*inimigo).y, _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=1;
                        (*inimigo).andandoPositivo=0;
                        (*inimigo).x-=((*inimigo).principal.velocidade);
                    }else if(posicaoLivre(objCenario, (*inimigo).x, ((*inimigo).y+32)+((*inimigo).principal.velocidade), _xMapa, _yMapa)){
                        (*inimigo).andandoEmX=0;
                        (*inimigo).andandoPositivo=1;
                        (*inimigo).y+=((*inimigo).principal.velocidade);
                    }
                }
            }
        }
    }
}

void salvarJogo(FILE **arqSave, FILE **arqNome, int xPlayer, int yPlayer, int xMapa, int yMapa, int xCaixa, int yCaixa, ObjetoPrincipal objPrincipalPlayer, int dificuldade, int fase, ObjetoInimigo **inimigos, int quantidadeInimigos){
    char data[20];
    char url[80];

    sprintf(data, "%d-%d-%d_%d-%d-%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    sprintf(url, "../saves/%s.bin", data);
    sprintf(data, "%s\n", data);

    *arqNome = fopen("../saves/save.save", "a");
    fprintf(*arqNome, data);
    fclose(*arqNome);

    *arqSave = fopen(url, "wb");
    if (!(*arqSave))
    perror("fopen");

    fwrite(&dificuldade, sizeof(int), 1, *arqSave);
    fwrite(&fase, sizeof(int), 1, *arqSave);
    fwrite(&xMapa, sizeof(int), 1, *arqSave);
    fwrite(&yMapa, sizeof(int), 1, *arqSave);
    fwrite(&xCaixa, sizeof(int), 1, *arqSave);
    fwrite(&yCaixa, sizeof(int), 1, *arqSave);
    fwrite(&xPlayer, sizeof(int), 1, *arqSave);
    fwrite(&yPlayer, sizeof(int), 1, *arqSave);
    fwrite(&objPrincipalPlayer.municao, sizeof(int), 1, *arqSave);
    fwrite(&objPrincipalPlayer.vida, sizeof(int), 1, *arqSave);
    fwrite(&objPrincipalPlayer.dano, sizeof(int), 1, *arqSave); //esse vai servir pra saber que classe
    fwrite(&quantidadeInimigos, sizeof(int), 1, *arqSave);

    int matriz[quantidadeInimigos][7];
    for(int i=0;i<quantidadeInimigos;i++){
        matriz[i][0]= (*inimigos)[i].x;
        matriz[i][1]= (*inimigos)[i].y;
        matriz[i][2]= (*inimigos)[i].andandoEmX;
        matriz[i][3]= (*inimigos)[i].andandoPositivo;
        matriz[i][4]= (*inimigos)[i].principal.dano; //esse vai servir pra saber que classe ele é
        matriz[i][5]= (*inimigos)[i].vivo;
        matriz[i][6]= (*inimigos)[i].vendoInimigo;
    }
    fwrite(matriz, sizeof(int), quantidadeInimigos*7, *arqSave);

    fclose(*arqSave);
}

int carregarJogo(FILE **arqSave, char data[50], int *x, int *y, int *xMapa, int *yMapa, int *xCaixa, int *yCaixa, ObjetoPrincipal *principal, int *dificuldade, int *fase, int *quantidadeInimigos, int matriz[100][7]){
    *arqSave = fopen(data, "rb");

    if(!(*arqSave)){
        perror("error:");
        printf("\nErro ao carregar o jogo!\n");
        return 0;
    }

    fread(dificuldade, sizeof(int), 1, *arqSave);
    fread(fase, sizeof(int), 1, *arqSave);
    fread(xMapa, sizeof(int), 1, *arqSave);
    fread(yMapa, sizeof(int), 1, *arqSave);
    fread(xCaixa, sizeof(int), 1, *arqSave);
    fread(yCaixa, sizeof(int), 1, *arqSave);
    fread(x, sizeof(int), 1, *arqSave);
    fread(y, sizeof(int), 1, *arqSave);
    fread(&principal->municao, sizeof(int), 1, *arqSave);
    fread(&principal->vida, sizeof(int), 1, *arqSave);
    fread(&principal->dano, sizeof(int), 1, *arqSave); //esse vai servir pra saber que classe
    fread(quantidadeInimigos, sizeof(int), 1, *arqSave);

    fread(matriz, sizeof(int), (*quantidadeInimigos)*7, *arqSave);

    fclose(*arqSave);

    return 1;
}

void fazerAviso(char msg[30], float time, int fonte5){
    DesenhaRetangulo(0, 0, 50, 350, BRANCO, 0);
    EscreverEsquerda(msg, 30, 20, fonte5);
    DesenhaRetangulo(0, 0, 5, 340-((time/5)*340), AZUL, 0);
}

void criarAdversario(ObjetoAdversario *inicio, int idAdversario, int x, int y, float angle, int tipo, int vivo, int mov, char nome[20]){
    int objId, objTopoId;
    ObjetoPrincipal principal;
    if(tipo==0){
        //speed
        setClasseSpeed(&principal);
        principal.vivo=vivo;
        objId = CriaObjeto("../imagens/tank_base_2.png",1, 255, 0);
        DefineFrameObjeto(objId, 0, 0, 64, 64);
        SetDimensoesObjeto(objId, 64, 64);
        objTopoId = CriaObjeto("../imagens/tank_top2.png",1, 255, 0);
        MoveObjeto(objId, x, x);
        MoveObjeto(objTopoId, x, y);
        SetPivoObjeto(objId, 0.5f, 0.5f);
        SetPivoObjeto(objTopoId, 0.5f, 0.5f);
    }else if(tipo==1){
        //tank
        setClasseTank(&principal);
        principal.vivo=vivo;
        objId = CriaObjeto("../imagens/tank_base_1.png",1, 255, 0);
        DefineFrameObjeto(objId, 0, 0, 64, 64);
        SetDimensoesObjeto(objId, 64, 64);
        objTopoId = CriaObjeto("../imagens/tank_top1.png",1, 255, 0);
        MoveObjeto(objId, x, x);
        MoveObjeto(objTopoId, x, y);
        SetPivoObjeto(objId, 0.5f, 0.5f);
        SetPivoObjeto(objTopoId, 0.5f, 0.5f);
    }else if(tipo==2){
        //knife
        setClasseKnife(&principal);
        principal.vivo=vivo;
        objId = CriaObjeto("../imagens/tank_base_3.png",1, 255, 0);
        DefineFrameObjeto(objId, 0, 0, 64, 64);
        SetDimensoesObjeto(objId, 64, 64);
        objTopoId = CriaObjeto("../imagens/tank_top3.png",1, 255, 0);
        MoveObjeto(objId, x, x);
        MoveObjeto(objTopoId, x, y);
        SetPivoObjeto(objId, 0.5f, 0.5f);
        SetPivoObjeto(objTopoId, 0.5f, 0.5f);
    }else{
        //reload
        setClasseReload(&principal);
        principal.vivo=vivo;
        objId = CriaObjeto("../imagens/tank_base_4.png",1, 255, 0);
        DefineFrameObjeto(objId, 0, 0, 64, 64);
        SetDimensoesObjeto(objId, 64, 64);
        objTopoId = CriaObjeto("../imagens/tank_top4.png",1, 255, 0);
        MoveObjeto(objId, x, x);
        MoveObjeto(objTopoId, x, y);
        SetPivoObjeto(objId, 0.5f, 0.5f);
        SetPivoObjeto(objTopoId, 0.5f, 0.5f);
    }

    InsereFim(inicio, objId, objTopoId, idAdversario, x, y, angle, principal, tipo, vivo, mov, nome);
}

char pegarTeclado(PIG_Evento evento, int capsLock){
    if(evento.teclado.tecla == TECLA_q){
        if(capsLock){
            return 'Q';
        }else{
            return 'q';
        }
    }else if(evento.teclado.tecla == TECLA_w){
        if(capsLock){
            return 'W';
        }else{
            return 'w';
        }
    }else if(evento.teclado.tecla == TECLA_e){
        if(capsLock){
            return 'E';
        }else{
            return 'e';
        }
    }else if(evento.teclado.tecla == TECLA_r){
        if(capsLock){
            return 'R';
        }else{
            return 'r';
        }
    }else if(evento.teclado.tecla == TECLA_t){
        if(capsLock){
            return 'T';
        }else{
            return 't';
        }
    }else if(evento.teclado.tecla == TECLA_y){
        if(capsLock){
            return 'Y';
        }else{
            return 'y';
        }
    }else if(evento.teclado.tecla == TECLA_u){
        if(capsLock){
            return 'U';
        }else{
            return 'u';
        }
    }else if(evento.teclado.tecla == TECLA_i){
        if(capsLock){
            return 'I';
        }else{
            return 'i';
        }
    }else if(evento.teclado.tecla == TECLA_o){
        if(capsLock){
            return 'O';
        }else{
            return 'o';
        }
    }else if(evento.teclado.tecla == TECLA_p){
        if(capsLock){
            return 'P';
        }else{
            return 'p';
        }
    }else if(evento.teclado.tecla == TECLA_a){
        if(capsLock){
            return 'A';
        }else{
            return 'a';
        }
    }else if(evento.teclado.tecla == TECLA_s){
        if(capsLock){
            return 'S';
        }else{
            return 's';
        }
    }else if(evento.teclado.tecla == TECLA_d){
        if(capsLock){
            return 'D';
        }else{
            return 'd';
        }
    }else if(evento.teclado.tecla == TECLA_f){
        if(capsLock){
            return 'F';
        }else{
            return 'f';
        }
    }else if(evento.teclado.tecla == TECLA_g){
        if(capsLock){
            return 'G';
        }else{
            return 'g';
        }
    }else if(evento.teclado.tecla == TECLA_h){
        if(capsLock){
            return 'H';
        }else{
            return 'h';
        }
    }else if(evento.teclado.tecla == TECLA_j){
        if(capsLock){
            return 'J';
        }else{
            return 'j';
        }
    }else if(evento.teclado.tecla == TECLA_k){
        if(capsLock){
            return 'K';
        }else{
            return 'k';
        }
    }else if(evento.teclado.tecla == TECLA_l){
        if(capsLock){
            return 'L';
        }else{
            return 'l';
        }
    }else if(evento.teclado.tecla == TECLA_z){
        if(capsLock){
            return 'Z';
        }else{
            return 'z';
        }
    }else if(evento.teclado.tecla == TECLA_x){
        if(capsLock){
            return 'X';
        }else{
            return 'x';
        }
    }else if(evento.teclado.tecla == TECLA_c){
        if(capsLock){
            return 'C';
        }else{
            return 'c';
        }
    }else if(evento.teclado.tecla == TECLA_v){
        if(capsLock){
            return 'V';
        }else{
            return 'v';
        }
    }else if(evento.teclado.tecla == TECLA_b){
        if(capsLock){
            return 'B';
        }else{
            return 'b';
        }
    }else if(evento.teclado.tecla == TECLA_n){
        if(capsLock){
            return 'N';
        }else{
            return 'n';
        }
    }else if(evento.teclado.tecla == TECLA_m){
        if(capsLock){
            return 'M';
        }else{
            return 'm';
        }
    }else if(evento.teclado.tecla == TECLA_0 || evento.teclado.tecla == TECLA_KP_0){
        return '0';
    }else if(evento.teclado.tecla == TECLA_1 || evento.teclado.tecla == TECLA_KP_1){
        return '1';
    }else if(evento.teclado.tecla == TECLA_2 || evento.teclado.tecla == TECLA_KP_2){
        return '2';
    }else if(evento.teclado.tecla == TECLA_3 || evento.teclado.tecla == TECLA_KP_3){
        return '3';
    }else if(evento.teclado.tecla == TECLA_4 || evento.teclado.tecla == TECLA_KP_4){
        return '4';
    }else if(evento.teclado.tecla == TECLA_5 || evento.teclado.tecla == TECLA_KP_5){
        return '5';
    }else if(evento.teclado.tecla == TECLA_6 || evento.teclado.tecla == TECLA_KP_6){
        return '6';
    }else if(evento.teclado.tecla == TECLA_7 || evento.teclado.tecla == TECLA_KP_7){
        return '7';
    }else if(evento.teclado.tecla == TECLA_8 || evento.teclado.tecla == TECLA_KP_8){
        return '8';
    }else if(evento.teclado.tecla == TECLA_9 || evento.teclado.tecla == TECLA_KP_9){
        return '9';
    }else if(evento.teclado.tecla == TECLA_PONTO || evento.teclado.tecla == TECLA_KP_PONTO){
        return '.';
    }else if(evento.teclado.tecla == TECLA_BARRAESPACO){
        return ' ';
    }else{
        return NULL;
    }
}

int main(int argc, char* args[])
{
    int fonte1, fonte2, fonte3, fonte4, fonte5, fonte6, fonte7, fonte8, fonte9, fonte10, fonte11; //FONTES PARA DESENHAR
    int tela=0; //MOSTRA QUAL TELA DO JOGO ESTÁ SENDO MOSTRADA, 0 = TELA INICIAL...
    int escolhaInicial=0; // SERVE PRA SABER ONDE ESTA A ESCOLHA DAS OPCOES
    int objfundo; //É A IMAGEM DO FUNDO DA TELA INICIAL
    int objMapa; // É A IMAGEM DO MAPA
    int volumeMusica= 50; // O VOLUME DA MUSICA
    int volumeJogo = 50; //O VOLUME DO JOGO
    int fullscreen=0; // PRA SABER SE ESTA EM FULLSCREEN OU NAO
    int mostrarFps=0; // PRA SABER SE ESTA MOSTRANDO O FPS OU NAO
    int pause=0; // PAUSE QUANDO APERTAR ESC
    int opcoes=0; // MOSTRA OPCOES NO PAUSE
    int transparencia=255; // COLOCA TRANSPARENCIA NO HUD SE TIVER ALGUEM LA
    int dificuldade=0; // DIFICULDADE DO JOGO 0 PARA FACIL 1 PARA MEDIO E 2 PARA DIFICIL
    int tank_base_obj, tank_top_obj; // OBJETOS DOS TANKS
    int movimento=0; // TIPOS DE MOVIMENTOS 0=CIMA, 1=BAIXO, 2=DIREITA, 3=ESQUERDA
    int _x = 100, _y = 200, absX= 100, absY= 200; // COORDENADAS DO TANK
    int _xMapa = 0, _yMapa = 0; //COORDENADAS DO MAPA
    int _xOffSet, _yOffSet; //OFFSET DO CANHAO
    ObjetoCenario objCenario[24]; //OBJETOS QUE TERAO COLISAO
    ObjetoPrincipal objPrincipal; // GUARDA INFORMAÇÕES DAS CLASSES
    int objDemonstracao[8]; // SERVE PRA MOSTRAR OS TANKS NA TELA DE ESCOLHA DE CLASSE
    float timer=0, auxTimer=0, auxCaixa=0, auxDelay=0, auxCarregando=0, auxAviso=0, auxPiscada=0; // TEMPORIZADORES
    int geradordeParticulas; //GERADOR DE PARTICULAS DO PLAYER
    int geradordeParticulasInimigoTank; //GERADOR DE PARTICULAS DOS INIMIGOS TANK
    int geradordeParticulasInimigoSpeed; //GERADOR DE PARTICULAS DOS INIMIGOS
    int geradordeParticulasInimigoKnife; //GERADOR DE PARTICULAS DOS INIMIGOS
    int geradordeParticulasInimigoReload; //GERADOR DE PARTICULAS DOS INIMIGOS
    int fase = 1; // DIZ EM QUE FASEO  JOGO ESTÁ
    ObjetoInimigo *inimigos=NULL; //GUARDA OS INIMIGOS
    int inimigosIniciados=0; //BOOLEAN PARA SABER SE OS INIMIGOS JA FORAM CARREGADOS
    int quantidadeInimigos = calculaQuantidadeInimigos(fase); //PRA SABER QUANTOS INIMIGOS TEM
    int quantidadeInimigosVivos = quantidadeInimigos;// PRA SABER QUANTOS INIMIGOS VIVOS TEM
    int objCaixa; //OBJ QUE SERVE PRA RESTAURAR A VIDA
    Caixa caixa; //INFORMAÇOES DA CAIXA
    FILE *arquivoSaveNome; //ARQUIVO PARA SALVAR OS SAVES
    FILE *arquivoSave; // ARQUIVO PARA SALVAR O JOGO
    int carregando=0; //BARRA DA TELA 7 PARA COMPLETAR O CARREGAMENTO
    int isMultiplayer=0; // INDICA SE ESTÁ JOGANDO NO MODO MULTIPLAYER
    char input[20] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; //SERVE PRA GUARDAR O INPUT DO IP DO MULTIPLAYER
    char input2[20] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; //SERVE PRA GUARDAR O INPUT DO NOME NO MULTIPLAYER
    int letrasInput = 0; //AUXILIAR PARA GUARDAR A POSICAO DO ESPAÇO LIBERADO NO TECLADO
    int letrasInput2 = 0;//AUXILIAR PARA GUARDAR A POSICAO DO ESPAÇO LIBERADO NO TECLADO
    int socket = -1; //GUARDA O NUMERO DO SOCKET
    int idMultiplayer = -1; //GUARDA O SEU ID DENTRO DO SERVIDOR
    int idAdversario = 0; //GUARDA O ID DO ADVERSARIO
    int comecouMultiplayer = 0; //CHECA SE O JOGO JA COMEÇOU OU NAO, NO MULTIPLAYER
    ObjetoAdversario *inicio; //GUARDA OS ADVERSARIOS
    char aviso[100]; //GUARDA A MENSAGEM QUE SERA DADA COMO AVISO
    int mouseClicado = 1; // AUXILIAR PARA A TELA 3
    int onMouseClicado = 0; // AUXILIAR DA TELA 3
    int capsLock = 0; // AUXILIAR DA TELA 3
    int acabou = 0;
    char vencedor[20];
    int audio_Andar_Idle=0, audio_Explosao=0, audio_Recarregar=0, audio_Soundtrack=0, audio_Tiro1=0, audio_Tiro2=0, audio_Tomou_Tiro=0;

    srand(time(NULL));

    t = time(NULL);
    tm = *localtime(&t);

    printf("Criando variaveis...\n");

    CriaJogo("Tank Multiplayer 3000", 0, 600, 800);

    printf("Criando base do jogo...\n");

    meuTeclado = GetTeclado();

    printf("Mapeando teclado...\n");

    PIG_Cor preto;
    preto.r = 0;
    preto.g = 0;
    preto.b = 0;
    preto.a = 50;
    PIG_Cor vermelho;
    vermelho.r = 255;
    vermelho.g = 0;
    vermelho.b = 0;
    vermelho.a = 50;

    fonte1 = CriaFonteFundo("../fontes/arial.ttf",50,"../imagens/aco-inox.png", 1, PRETO);
    fonte2 = CriaFonteNormal("../fontes/arial.ttf", 30,VERMELHO, 0, PRETO);
    fonte3 = CriaFonteNormal("../fontes/arial.ttf", 30,VERMELHO, 1, PRETO);
    fonte4 = CriaFonteNormal("../fontes/TopSecret.ttf", 50, PRETO, 0, BRANCO);
    fonte5 = CriaFonteNormal("../fontes/gunplay.ttf", 20, PRETO, 0, BRANCO);
    fonte6 = CriaFonteNormal("../fontes/gunplay.ttf", 20, VERMELHO, 1, PRETO);
    fonte7 = CriaFonteNormal("../fontes/TopSecret.ttf", 70, CINZA, 1, PRETO);
    fonte8 = CriaFonteNormal("../fontes/gunplay.ttf", 100, PRETO, 1, PRETO);
    fonte9 = CriaFonteNormal("../fontes/arial.ttf", 30, vermelho, 1, preto);
    fonte10 = CriaFonteNormal("../fontes/arial.ttf", 10, PRETO, 0, PRETO);
    fonte11 = CriaFonteNormal("../fontes/arial.ttf", 20, PRETO, 0, PRETO);

    printf("Criando fontes...\n");

    audio_Andar_Idle = CriaAudio("../audios/andaridle.ogg", -1, -1);
    audio_Explosao = CriaAudio("../audios/explosao.ogg", 0, -1);
    audio_Recarregar = CriaAudio("../audios/recarregar.ogg", 0, -1);
    audio_Tiro1 = CriaAudio("../audios/tiro1.ogg", 0, -1);
    audio_Tiro2 = CriaAudio("../audios/tiro2.ogg", 0, -1);
    audio_Tomou_Tiro = CriaAudio("../audios/tomoutiro.ogg", 0, -1);
    CarregaBackground("../audios/soundtrack.ogg");

    printf("Criando sons...\n");

    objfundo = CriaObjeto("../imagens/tela-inicial.png", 0, 255, 0);
    objMapa = CriaObjeto("../imagens/mapa.png",0, 255, 0);

    objCaixa = CriaObjeto("../imagens/caixa.png", 0, 255, 0);
    caixa.x=3000;
    caixa.y=3000;

    printf("Criando imagens...\n");

    geradordeParticulas = CriaGeradorParticulas(300, "../imagens/tiro.bmp", -1, -1, 0);
    geradordeParticulasInimigoTank = CriaGeradorParticulas(300, "../imagens/tiro1.bmp", -1, -1 , 0);
    geradordeParticulasInimigoSpeed = CriaGeradorParticulas(300, "../imagens/tiro2.bmp", -1, -1 , 0);
    geradordeParticulasInimigoKnife = CriaGeradorParticulas(300, "../imagens/tiro3.bmp", -1, -1 , 0);
    geradordeParticulasInimigoReload = CriaGeradorParticulas(300, "../imagens/tiro4.bmp", -1, -1 , 0);

    printf("Iniciando geradores de particulas...\n");

    MoveGeradorParticulas(geradordeParticulas, 100, 200);
    MudaRotacaoParticulas(geradordeParticulas, 4);

    printf("Configurando geradores de particulas...\n");

    iniciaDemonstracao(objDemonstracao);

    printf("Iniciando demonstraveis...\n");

    iniciarCenario(objCenario);

    printf("Carregando cenario...\n\n");
    printf("Iniciando jogo...\n");

    PlayBackground();


    while(JogoRodando()){
        evento = GetEvento();

        timer++;
        if(timer>=GetFPS()/100){
            if(!pause){
                if(auxTimer>0){
                    auxTimer-=0.01;
                }else{
                    auxTimer=0;
                }
                if(auxCarregando>0){
                    auxCarregando-=0.01;
                }else{
                    auxCarregando=0;
                }
                if(auxCaixa>0){
                    auxCaixa-=0.01;
                }else{
                    auxCaixa=0;
                }
                if(auxDelay>0){
                    auxDelay-=0.01;
                }else{
                    auxDelay=0;
                }
                if(auxAviso>0){
                    auxAviso-=0.01;
                }else{
                    auxAviso=0;
                }
                if(objPrincipal.auxRecarga>0){
                    objPrincipal.auxRecarga-=0.01;
                }else{
                    objPrincipal.auxRecarga=0;
                }
                if(auxPiscada>0){
                    auxPiscada-=0.01;
                }else{
                    auxPiscada=0;
                }
                if(inimigosIniciados){
                    for(int i=0;i<quantidadeInimigos;i++){
                        if(inimigos[i].recarregando>0){
                            inimigos[i].recarregando-=0.01;
                        }else{
                            inimigos[i].recarregando=0;
                        }
                        if(inimigos[i].auxAndar>0){
                            inimigos[i].auxAndar-=0.01;
                        }else{
                            inimigos[i].auxAndar=0;
                        }
                        if(inimigos[i].auxMudarDirecao>0){
                            inimigos[i].auxMudarDirecao-=0.01;
                        }else{
                            inimigos[i].auxMudarDirecao=0;
                        }
                        if(inimigos[i].auxMudarDirecao2>0){
                            inimigos[i].auxMudarDirecao2-=0.01;
                        }else{
                            inimigos[i].auxMudarDirecao2=0;
                        }
                    }
                }
                timer=0;
            }
        }

        objPrincipal.lastX = _x;
        objPrincipal.lastY = _y;

        if(evento.tipoEvento == EVENTO_TECLADO){
            if(evento.teclado.acao == TECLA_PRESSIONADA){
                if(tela==0){
                    if(evento.teclado.tecla == TECLA_CIMA){
                        if(escolhaInicial!=0){
                            escolhaInicial--;
                        }else{
                            escolhaInicial=4;
                        }
                    }else if(evento.teclado.tecla == TECLA_BAIXO){
                        if(escolhaInicial!=4){
                            escolhaInicial++;
                        }else{
                            escolhaInicial=0;
                        }
                    }else if(evento.teclado.tecla == TECLA_ENTER){
                        if(escolhaInicial==6){
                            FinalizaJogo(); // SAI DO JOGO
                        }else if(escolhaInicial==5){
                            tela = 1; // CREDITO
                            escolhaInicial=0;
                        }else if(escolhaInicial==4){
                            tela = 8; // AJUDA
                            escolhaInicial=0;
                        }else if(escolhaInicial==3){
                            tela = 2; // OPCOES
                            escolhaInicial=0;
                        }else if(escolhaInicial==2){
                            tela = 3; // MULTIPLAYER
                            escolhaInicial=0;
                        }else if(escolhaInicial==1){
                            tela = 7; // CARREGAR JOGO
                            escolhaInicial=0;
                        }else if(escolhaInicial==0){
                            tela = 5; // VAI PRA TELA DE ESCOLHER A CLASSE
                            escolhaInicial=0;
                        }
                    }
                }else if(tela==1){
                    if(evento.teclado.tecla == TECLA_ENTER){
                        tela=0;
                        escolhaInicial=0;
                    }
                }else if(tela==2){
                    if(evento.teclado.tecla == TECLA_CIMA){
                        if(escolhaInicial!=0){
                            escolhaInicial--;
                        }else{
                            escolhaInicial=4;
                        }
                    }else if(evento.teclado.tecla == TECLA_BAIXO){
                        if(escolhaInicial!=4){
                            escolhaInicial++;
                        }else{
                            escolhaInicial=0;
                        }
                    }else if(evento.teclado.tecla == TECLA_DIREITA){
                        if(escolhaInicial==0){
                            if(volumeMusica!=100){
                                volumeMusica++;
                            }
                        }else if(escolhaInicial==1){
                            if(volumeJogo!=100){
                                volumeJogo++;
                            }
                        }
                    }else if(evento.teclado.tecla == TECLA_ESQUERDA){
                        if(escolhaInicial==0){
                            if(volumeMusica!=0){
                                volumeMusica--;
                            }
                        }else if(escolhaInicial==1){
                            if(volumeJogo!=0){
                                volumeJogo--;
                            }
                        }
                    }else if(evento.teclado.tecla == TECLA_ENTER){
                        if(escolhaInicial==4){
                            tela = 0;
                            escolhaInicial=0;
                        }else if(escolhaInicial==2){
                            if(fullscreen==0){
                                fullscreen=1;
                                SetModoJanela(JANELA_TELACHEIA);
                            }else{
                                fullscreen=0;
                                SetModoJanela(JANELA_NORMAL);
                            }
                        }else if(escolhaInicial==3){
                            if(mostrarFps==0){
                                mostrarFps=1;
                            }else{
                                mostrarFps=0;
                            }
                        }
                    }
                }else if(tela==5){
                    if(evento.teclado.tecla == TECLA_ESQUERDA){
                        if(escolhaInicial!=0){
                            escolhaInicial--;
                        }else{
                            escolhaInicial=4;
                        }
                    }else if(evento.teclado.tecla == TECLA_DIREITA){
                        if(escolhaInicial!=4){
                            escolhaInicial++;
                        }else{
                            escolhaInicial=0;
                        }
                    }else if(evento.teclado.tecla == TECLA_ENTER){
                        if(escolhaInicial==4){
                            setClasseReload(&objPrincipal);
                            tank_base_obj = CriaObjeto("../imagens/tank_base_4.png",1, 255, 0);
                            DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                            SetDimensoesObjeto(tank_base_obj, 64, 64);
                            tank_top_obj = CriaObjeto("../imagens/tank_top4.png",1, 255, 0);
                            MoveObjeto(tank_base_obj, _x, _y);
                            MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                            SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                            SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            tela=6;
                            escolhaInicial=0;
                        }else if(escolhaInicial==3){
                            setClasseKnife(&objPrincipal);
                            tank_base_obj = CriaObjeto("../imagens/tank_base_3.png",1, 255, 0);
                            DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                            SetDimensoesObjeto(tank_base_obj, 64, 64);
                            tank_top_obj = CriaObjeto("../imagens/tank_top3.png",1, 255, 0);
                            MoveObjeto(tank_base_obj, _x, _y);
                            MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                            SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                            SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            tela=6;
                            escolhaInicial=0;
                        }else if(escolhaInicial==2){
                            setClasseTank(&objPrincipal);
                            tank_base_obj = CriaObjeto("../imagens/tank_base_1.png",1, 255, 0);
                            DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                            SetDimensoesObjeto(tank_base_obj, 64, 64);
                            tank_top_obj = CriaObjeto("../imagens/tank_top1.png",1, 255, 0);
                            MoveObjeto(tank_base_obj, _x, _y);
                            MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                            SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                            SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            tela=6;
                            escolhaInicial=0;
                        }else if(escolhaInicial==1){
                            setClasseSpeed(&objPrincipal);
                            tank_base_obj = CriaObjeto("../imagens/tank_base_2.png",1, 255, 0);
                            DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                            SetDimensoesObjeto(tank_base_obj, 64, 64);
                            tank_top_obj = CriaObjeto("../imagens/tank_top2.png",1, 255, 0);
                            MoveObjeto(tank_base_obj, _x, _y);
                            MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                            SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                            SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            tela=6;
                            escolhaInicial=0;
                        }else if(escolhaInicial==0){
                            tela = 0; // VAI PRA TELA PRINCIPAL
                            escolhaInicial=0;
                        }
                    }
                }else if(tela==6){
                    if(evento.teclado.tecla == TECLA_ESQUERDA){
                        if(escolhaInicial!=0){
                            escolhaInicial--;
                        }else{
                            escolhaInicial=3;
                        }
                    }else if(evento.teclado.tecla == TECLA_DIREITA){
                        if(escolhaInicial!=3){
                            escolhaInicial++;
                        }else{
                            escolhaInicial=0;
                        }
                    }else if(evento.teclado.tecla == TECLA_ENTER){
                        if(escolhaInicial==0){
                            tela = 0; // VAI PRA TELA PRINCIPAL
                            escolhaInicial=0;
                        }else{
                            iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);
                            inimigosIniciados=1;
                            quantidadeInimigosVivos = quantidadeInimigos;
                            auxCaixa=60;
                            dificuldade= escolhaInicial;
                            tela=4;
                            escolhaInicial=0;
                        }
                    }
                }
            }
        }

        if(evento.tipoEvento== EVENTO_TECLADO){
            if(tela==4){
                if(evento.teclado.acao==TECLA_PRESSIONADA){
                   if(evento.teclado.tecla == TECLA_ESC){
                        if(pause){
                            pause=0;
                            opcoes=0;
                        }else{
                            pause=1;
                            escolhaInicial=0;
                        }
                   }
                   if(pause){
                        if(!opcoes){
                           if(evento.teclado.tecla == TECLA_CIMA){
                                if(escolhaInicial!=0){
                                    escolhaInicial--;
                                }else{
                                    escolhaInicial=2;
                                }
                           }else if(evento.teclado.tecla == TECLA_BAIXO){
                                if(escolhaInicial!=2){
                                    escolhaInicial++;
                                }else{
                                    escolhaInicial=0;
                                }
                           }else if(evento.teclado.tecla == TECLA_ENTER){
                                if(escolhaInicial==2){
                                    pause = 0;
                                    tela=0; // SAI DO JOGO
                                    escolhaInicial=0;
                                    _x = 100;
                                    _y = 200;
                                    _xMapa = 0;
                                    _yMapa = 0;
                                    free(inimigos);
                                    inimigosIniciados=0;
                                }else if(escolhaInicial==1){
                                    opcoes=1; // OPÇÕES
                                    escolhaInicial=0;
                                }else if(escolhaInicial==0){
                                    pause=0; // VOLTA O JOGO
                                }
                            }
                        }else{
                            if(evento.teclado.tecla == TECLA_CIMA){
                                if(escolhaInicial!=0){
                                    escolhaInicial--;
                                }else{
                                    escolhaInicial=4;
                                }
                            }else if(evento.teclado.tecla == TECLA_BAIXO){
                                if(escolhaInicial!=4){
                                    escolhaInicial++;
                                }else{
                                    escolhaInicial=0;
                                }
                            }else if(evento.teclado.tecla == TECLA_DIREITA){
                                if(escolhaInicial==0){
                                    if(volumeMusica!=100){
                                        volumeMusica++;
                                        SetVolumeBackground(volumeMusica);
                                    }
                                }else if(escolhaInicial==1){
                                    if(volumeJogo!=100){
                                        volumeJogo++;
                                        SetVolumeTudo(volumeJogo);
                                    }
                                }
                            }else if(evento.teclado.tecla == TECLA_ESQUERDA){
                                if(escolhaInicial==0){
                                    if(volumeMusica!=0){
                                        volumeMusica--;
                                        SetVolumeBackground(volumeMusica);
                                    }
                                }else if(escolhaInicial==1){
                                    if(volumeJogo!=0){
                                        volumeJogo--;
                                        SetVolumeTudo(volumeJogo);
                                    }
                                }
                            }else if(evento.teclado.tecla == TECLA_ENTER){
                                if(escolhaInicial==4){
                                    opcoes=0;
                                    escolhaInicial=0;
                                }else if(escolhaInicial==2){
                                    if(fullscreen==0){
                                        fullscreen=1;
                                        SetModoJanela(JANELA_TELACHEIA);
                                    }else{
                                        fullscreen=0;
                                        SetModoJanela(JANELA_NORMAL);
                                    }
                                }else if(escolhaInicial==3){
                                    if(mostrarFps==0){
                                        mostrarFps=1;
                                    }else{
                                        mostrarFps=0;
                                    }
                                }
                            }
                        }
                   }else{
                       if(objPrincipal.vivo && !acabou){
                           if(evento.teclado.tecla == TECLA_a || evento.teclado.tecla == TECLA_ESQUERDA){
                                if(movimento!=3){
                                    SetAnguloObjeto(tank_base_obj, 90);
                                    movimento=3;
                                    _xOffSet=-6;
                                    _yOffSet=1;
                                }
                                if(_x<390){
                                    if(_xMapa<0){
                                        _xMapa+=objPrincipal.velocidade;
                                        absX-=objPrincipal.velocidade;
                                    }else{
                                        _x-=objPrincipal.velocidade;
                                        absX-=objPrincipal.velocidade;
                                    }
                                }else{
                                    _x-=objPrincipal.velocidade;
                                    absX-=objPrincipal.velocidade;
                                }
                                MoveObjeto(tank_base_obj, _x,_y);
                                if(colisaoObjetos(objCenario, tank_base_obj)){
                                    _x+=objPrincipal.velocidade;
                                    absX+=objPrincipal.velocidade;
                                }

                           }else if(evento.teclado.tecla == TECLA_d || evento.teclado.tecla == TECLA_DIREITA){
                                if(movimento!=2){
                                    SetAnguloObjeto(tank_base_obj, 270);
                                    movimento=2;
                                    _xOffSet=6;
                                    _yOffSet=1;
                                }
                                if(_x>410){
                                    if(_xMapa>-1245){
                                        _xMapa-=objPrincipal.velocidade;
                                        absX+=objPrincipal.velocidade;
                                    }else{
                                        _x+=objPrincipal.velocidade;
                                        absX+=objPrincipal.velocidade;
                                    }
                                }else{
                                    _x+=objPrincipal.velocidade;
                                    absX+=objPrincipal.velocidade;
                                }
                                MoveObjeto(tank_base_obj, _x,_y);
                                if(colisaoObjetos(objCenario, tank_base_obj)){
                                    _x-=objPrincipal.velocidade;
                                    absX-=objPrincipal.velocidade;
                                }
                           }else if(evento.teclado.tecla == TECLA_s || evento.teclado.tecla == TECLA_BAIXO){
                                if(movimento!=1){
                                    SetAnguloObjeto(tank_base_obj, 180);
                                    movimento=1;
                                    _xOffSet=-1;
                                    _yOffSet=-6;
                                }
                                if(_y<290){
                                    if(_yMapa<0){
                                        _yMapa+=objPrincipal.velocidade;
                                        absY-=objPrincipal.velocidade;
                                    }else{
                                        _y-=objPrincipal.velocidade;
                                        absY-=objPrincipal.velocidade;
                                    }
                                }else{
                                    _y-=objPrincipal.velocidade;
                                    absY-=objPrincipal.velocidade;
                                }
                                MoveObjeto(tank_base_obj, _x,_y);
                                if(colisaoObjetos(objCenario, tank_base_obj)){
                                    _y+=objPrincipal.velocidade;
                                    absY+=objPrincipal.velocidade;
                                }

                           }else if(evento.teclado.tecla == TECLA_w || evento.teclado.tecla == TECLA_CIMA){
                                if(movimento!=0){
                                    SetAnguloObjeto(tank_base_obj, 0);
                                    movimento=0;
                                    _xOffSet=1;
                                    _yOffSet=6;
                                }
                                if(_y>310){
                                    if(_yMapa>-1445){
                                        _yMapa-=objPrincipal.velocidade;
                                        absY+=objPrincipal.velocidade;
                                    }else{
                                        _y+=objPrincipal.velocidade;
                                        absY+=objPrincipal.velocidade;
                                    }
                                }else{
                                    _y+=objPrincipal.velocidade;
                                    absY+=objPrincipal.velocidade;
                                }
                                MoveObjeto(tank_base_obj, _x,_y);

                                if(colisaoObjetos(objCenario, tank_base_obj)){
                                    _y-=objPrincipal.velocidade;
                                    absY-=objPrincipal.velocidade;
                                }
                           }
                        }
                   }

                    if(isMultiplayer){
                        char msg[40];
                        sprintf(msg, "andar:%d:%d:%d:%d:%d:%d\0", idMultiplayer, absX, absY, _xMapa, _yMapa, movimento);
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                    }

                    MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                    MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                    MoveObjeto(objMapa, _xMapa, _yMapa);
                    for(int i=0;i<24;i++){
                        MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                    }

                    if(inimigosIniciados){
                        for(int i=0;i<quantidadeInimigos;i++){
                            int __x = inimigos[i].x +_xMapa;
                            int __y = inimigos[i].y +_yMapa;
                            MoveObjeto(inimigos[i].objId, __x, __y);
                            MoveObjeto(inimigos[i].objTopoId, __x, __y);
                        }
                        MoveObjeto(objCaixa, caixa.x+_xMapa, caixa.y+_yMapa);
                    }
                }
            }
        }

        if(evento.tipoEvento == EVENTO_MOUSE){
            if(tela==4){
                if(!pause && objPrincipal.vivo!=0 && !acabou){
                    int pos_x = evento.mouse.posX;
                    int pos_y = evento.mouse.posY;

                    float angle = atan2(pos_x-(_x+32),pos_y-(_y+32));

                    SetAnguloObjeto(tank_top_obj, -radianosParaGraus(angle));
                    MudaDirecaoParticulas(geradordeParticulas, normalizarVetorX(pos_x-(_x+32), pos_y-(_y+32), 500), normalizarVetorY(pos_x-(_x+32), pos_y-(_y+32), 500));

                    if(isMultiplayer){
                        char msg[20];
                        sprintf(msg, "topmove:%d:%f\0", idMultiplayer, -radianosParaGraus(angle));
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                        if(comecouMultiplayer){
                            if(evento.mouse.acao == MOUSE_PRESSIONADO){
                                if(evento.mouse.botao == MOUSE_ESQUERDO){
                                    if(auxTimer<=0 && objPrincipal.municao>0){
                                        auxTimer = objPrincipal.tempoRecarga;
                                        objPrincipal.municao--;
                                        CriaParticula(geradordeParticulas, 1, 0,0, 800+50, 600+50, 5);

                                        PlayAudio(audio_Tiro1);

                                        int tipo = -1;
                                        if(objPrincipal.dano==150){
                                            tipo=0; //speed
                                        }else if(objPrincipal.dano==100){
                                            tipo=1; //tank
                                        }else if(objPrincipal.dano==250){
                                            tipo=2; //knife
                                        }else{
                                            tipo=3; //reload
                                        }

                                        sprintf(msg, "tiro:%d:%f:%f:%d\0", idMultiplayer, normalizarVetorX(pos_x-(_x+32), pos_y-(_y+32), 500), normalizarVetorY(pos_x-(_x+32), pos_y-(_y+32), 500), tipo);
                                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                                    }
                                }
                            }
                        }
                    }else{
                        if(evento.mouse.acao == MOUSE_PRESSIONADO){
                                if(evento.mouse.botao == MOUSE_ESQUERDO){
                                    if(auxTimer<=0 && objPrincipal.municao>0){
                                        auxTimer = objPrincipal.tempoRecarga;
                                        objPrincipal.municao--;
                                        CriaParticula(geradordeParticulas, 1, 0,0, 800+50, 600+50, 5);
                                        PlayAudio(audio_Tiro1);

                                    }
                                }
                            }
                    }
                }else{
                    if(objPrincipal.vivo==0){
                        if(colidiuMouseTexto(400, 250, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                            if(evento.mouse.acao==MOUSE_LIBERADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                pause = 0;
                                tela=0; // SAI DO JOGO
                                escolhaInicial=0;
                                _x = 100;
                                _y = 200;
                                _xMapa = 0;
                                _yMapa = 0;
                                fase = 0;
                                dificuldade = 0;
                                free(inimigos);
                                inimigosIniciados=0;
                            }
                        }
                    }
                    if(isMultiplayer && acabou){
                        if(colidiuMouseTexto(400, 250, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                            if(evento.mouse.acao==MOUSE_LIBERADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                pause = 0;
                                tela = 0; // SAI DO JOGO
                                escolhaInicial=0;
                                _x = 100;
                                _y = 200;
                                _xMapa = 0;
                                _yMapa = 0;
                                fase = 0;
                                dificuldade = 0;
                                free(inimigos);
                                inimigosIniciados=0;
                                if(isMultiplayer){
                                    isMultiplayer=0;
                                    comecouMultiplayer=0;
                                    DestroiSocketCliente(socket);
                                    socket = -1;
                                    DestroiLista(inicio);
                                    acabou=0;
                                }
                            }
                        }
                    }
                    if(!opcoes){
                        if(colidiuMouseTexto(320, 450, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 0;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                pause = 0;
                            }
                        }else if(colidiuMouseTexto(320, 400, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 1;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                opcoes = 1;
                            }
                        }else if(colidiuMouseTexto(320, 300, 30, 4, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 2;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                pause = 0;
                                escolhaInicial=0;
                                _x = 100;
                                _y = 200;
                                _xMapa = 0;
                                _yMapa = 0;
                                fase = 0;
                                dificuldade = 0;
                                free(inimigos);
                                inimigosIniciados=0;
                                tela=0; // SAI DO JOGO
                                if(isMultiplayer){
                                    isMultiplayer=0;
                                    DestroiSocketCliente(socket);
                                    comecouMultiplayer=0;
                                    socket = -1;
                                    acabou = 0;
                                }
                            }
                        }
                    }else{
                        if(colidiuMouseTexto(300, 450, 30, 22, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 0;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                if(volumeMusica!=128)
                                volumeMusica++;
                                SetVolumeBackground(volumeMusica);
                            }else if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_DIREITO){
                                if(volumeMusica!=0)
                                volumeMusica--;
                                SetVolumeBackground(volumeMusica);
                            }
                        }else if(colidiuMouseTexto(300, 400, 30, 18, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 1;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                if(volumeJogo!=128)
                                    volumeJogo++;
                                SetVolumeTudo(volumeJogo);
                            }else if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_DIREITO){
                                if(volumeJogo!=0)
                                volumeJogo--;
                                SetVolumeTudo(volumeJogo);
                            }
                        }else if(colidiuMouseTexto(300, 350, 30, 15, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 2;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                if(fullscreen){
                                    fullscreen=0;
                                    SetModoJanela(JANELA_NORMAL);
                                }else{
                                    fullscreen=1;
                                    SetModoJanela(JANELA_TELACHEIA);
                                }
                            }
                        }else if(colidiuMouseTexto(300, 300, 30, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 3;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                if(mostrarFps){
                                    mostrarFps=0;
                                }else{
                                    mostrarFps=1;
                                }
                            }
                        }else if(colidiuMouseTexto(300, 200, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                            escolhaInicial = 4;
                            if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                                escolhaInicial=0;
                                opcoes = 0;
                            }
                        }
                    }
                }
            }
            if(tela==0){
                if(colidiuMouseTexto(50, 350, 30, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 5; // NOVO JOGO: VAI PRA TELA DE ESCOLHER A CLASSE
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(50, 300, 30, 14, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 1;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 7; // CARREGAR JOGO
                        escolhaInicial=0;

                        arquivoSaveNome = fopen("../saves/save.save", "r");
                        if(!arquivoSaveNome){
                            tela=0;
                            escolhaInicial=0;
                            printf("Erro ao ler os arquivos!");
                            continue;
                        }
                        char linha[17];
                        char url[40];
                        int w=0;
                        while(fgets(linha, sizeof(linha)*17, arquivoSaveNome)!=NULL){
                            printf("Carregando arquivos [%d]...\n",w);
                            w++;
                        }
                        linha[strlen(linha)-1]='\0';
                        sprintf(url, "../saves/%s.bin", linha);
                        int matriz[100][7];
                        if(carregarJogo(&arquivoSave, url, &_x, &_y, &_xMapa, &_yMapa, &caixa.x, &caixa.y, &objPrincipal, &dificuldade, &fase, &quantidadeInimigos, matriz)){
                            iniciaInimigosCarregados(&inimigos, matriz, quantidadeInimigos);
                            quantidadeInimigosVivos = quantidadeInimigos;
                            inimigosIniciados=1;
                            auxCaixa=60;
                            t = time(NULL);
                            tm = *localtime(&t);
                            ObjetoPrincipal copia = objPrincipal;
                            if(copia.dano==150){
                                setClasseSpeed(&objPrincipal);
                                objPrincipal.municao = copia.municao;
                                objPrincipal.vida = copia.vida;
                                tank_base_obj = CriaObjeto("../imagens/tank_base_2.png",1, 255, 0);
                                DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                                SetDimensoesObjeto(tank_base_obj, 64, 64);
                                tank_top_obj = CriaObjeto("../imagens/tank_top2.png",1, 255, 0);
                                MoveObjeto(tank_base_obj, _x, _y);
                                MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                                SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                                SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            }else if(copia.dano==100){
                                setClasseTank(&objPrincipal);
                                objPrincipal.municao = copia.municao;
                                objPrincipal.vida = copia.vida;
                                tank_base_obj = CriaObjeto("../imagens/tank_base_1.png",1, 255, 0);
                                DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                                SetDimensoesObjeto(tank_base_obj, 64, 64);
                                tank_top_obj = CriaObjeto("../imagens/tank_top1.png",1, 255, 0);
                                MoveObjeto(tank_base_obj, _x, _y);
                                MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                                SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                                SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            }else if(copia.dano==250){
                                setClasseKnife(&objPrincipal);
                                objPrincipal.municao = copia.municao;
                                objPrincipal.vida = copia.vida;
                                tank_base_obj = CriaObjeto("../imagens/tank_base_3.png",1, 255, 0);
                                DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                                SetDimensoesObjeto(tank_base_obj, 64, 64);
                                tank_top_obj = CriaObjeto("../imagens/tank_top3.png",1, 255, 0);
                                MoveObjeto(tank_base_obj, _x, _y);
                                MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                                SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                                SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            }else{
                                setClasseReload(&objPrincipal);
                                objPrincipal.municao = copia.municao;
                                objPrincipal.vida = copia.vida;
                                tank_base_obj = CriaObjeto("../imagens/tank_base_4.png",1, 255, 0);
                                DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                                SetDimensoesObjeto(tank_base_obj, 64, 64);
                                tank_top_obj = CriaObjeto("../imagens/tank_top4.png",1, 255, 0);
                                MoveObjeto(tank_base_obj, _x, _y);
                                MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                                SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                                SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                            }
                            MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                            MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                            MoveObjeto(objMapa, _xMapa, _yMapa);
                            for(int i=0;i<24;i++){
                                MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                            }

                            if(inimigosIniciados){
                                for(int i=0;i<quantidadeInimigos;i++){
                                    int __x = inimigos[i].x +_xMapa;
                                    int __y = inimigos[i].y +_yMapa;
                                    MoveObjeto(inimigos[i].objId, __x, __y);
                                    MoveObjeto(inimigos[i].objTopoId, __x, __y);
                                }
                                MoveObjeto(objCaixa, caixa.x+_xMapa, caixa.y+_yMapa);
                            }
                        }else{
                            tela=0;
                            escolhaInicial=0;
                        }
                    }
                }else if(colidiuMouseTexto(50, 250, 30, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 2;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 3; // MULTIPLAYER
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(50, 200, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 3;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 2; // OPCOES
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(50, 150, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 4;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 8; //AJUDA
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(50, 100, 30, 7, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 5;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 1; //CREDITOS
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(50, 50, 30, 4, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 6;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(isMultiplayer)
                            DestroiSocketCliente(socket);
                        acabou = 0;
                        free(input);
                        destruirInimigos(&inimigos);
                        fclose(arquivoSave);
                        fclose(arquivoSaveNome);
                        printf("Finalizando jogo...");
                        FinalizaJogo();
                        escolhaInicial=0;
                    }
                }
            }
            if(tela==6){
                if(colidiuMouseTexto(30, 50, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);
                        inimigosIniciados=1;
                        quantidadeInimigosVivos = quantidadeInimigos;
                        auxCaixa=60;
                        tela = 5; // VAI PRA TELA DE ESCOLHER A CLASSE
                        escolhaInicial = 0;
                        t = time(NULL);
                        tm = *localtime(&t);
                    }
                }else if(colidiuMouseTexto(150, 300, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 1;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);
                        inimigosIniciados=1;
                        quantidadeInimigosVivos = quantidadeInimigos;
                        auxCaixa=60;
                        tela = 4; // INICIA O JOGO
                        dificuldade = escolhaInicial;
                        escolhaInicial=0;
                        t = time(NULL);
                        tm = *localtime(&t);
                    }
                }else if(colidiuMouseTexto(350, 300, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 2;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);
                        inimigosIniciados=1;
                        quantidadeInimigosVivos = quantidadeInimigos;
                        auxCaixa=60;
                        tela = 4; // INICIA O JOGO
                        dificuldade = escolhaInicial;
                        escolhaInicial=0;
                        t = time(NULL);
                        tm = *localtime(&t);
                    }
                }else if(colidiuMouseTexto(550, 300, 30, 7, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 3;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);
                        inimigosIniciados=1;
                        quantidadeInimigosVivos = quantidadeInimigos;
                        auxCaixa=60;
                        tela = 4; // INICIA O JOGO
                        dificuldade = escolhaInicial;
                        escolhaInicial=0;
                        t = time(NULL);
                        tm = *localtime(&t);
                    }
                }
            }
            if(tela==1){
                if(colidiuMouseTexto(150, 100, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela = 0; // VAI PRA TELA INICIAL
                        escolhaInicial=0;
                    }
                }
            }
            if(tela==3){
                if(colidiuMouseTexto(200, 190, 50, 7, 1, evento.mouse.posX, evento.mouse.posY)){
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        mouseClicado = 0;
                    }
                }
                if(colidiuMouseTexto(200, 390, 50, 7, 1, evento.mouse.posX, evento.mouse.posY)){
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        mouseClicado = 1;
                    }
                }
                if(colidiuMouseTexto(570, 400, 30, 4, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 1;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(!isMultiplayer && strncmp(input, "                    ",20)!=0){
                            printf("Tentando conectar com o servidor: %s\n", input2);
                            socket = CriaSocketCliente(input2, 36664);

                            if(socket != -1 && GetAtivoSocketCliente(socket)){
                                inicio = CriaLista();
                                isMultiplayer=1;
                                tela = 5;
                                printf("Conectado com sucesso!\n");
                            }else{
                                printf("Nao foi possivel conectar.\n");
                            }
                        }
                    }
                }
                if(colidiuMouseTexto(100, 50, 30, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela=0;
                        escolhaInicial=0;
                        if(isMultiplayer){
                            isMultiplayer=0;
                            if(GetAtivoSocketCliente(socket)){
                                DestroiSocketCliente(socket);
                                socket = -1;
                                printf("Desconetado do servidor.\n");
                            }
                        }
                    }
                }
            }
            if(tela==2){
                if(colidiuMouseTexto(150, 250, 30, 22, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(volumeMusica!=100)
                            volumeMusica++;
                        SetVolumeBackground(volumeMusica);
                    }else if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_DIREITO){
                        if(volumeMusica!=0)
                            volumeMusica--;
                        SetVolumeBackground(volumeMusica);
                    }
                }else if(colidiuMouseTexto(150, 200, 30, 18, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 1;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(volumeJogo!=100)
                            volumeJogo++;
                        SetVolumeTudo(volumeJogo);
                    }else if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_DIREITO){
                        if(volumeJogo!=0)
                        volumeJogo--;
                        SetVolumeTudo(volumeJogo);
                    }
                }else if(colidiuMouseTexto(150, 150, 30, 15, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 2;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(fullscreen){
                            fullscreen=0;
                            SetModoJanela(JANELA_NORMAL);
                        }else{
                            fullscreen=1;
                            SetModoJanela(JANELA_TELACHEIA);
                        }
                    }
                }else if(colidiuMouseTexto(150, 100, 30, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 3;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        if(mostrarFps){
                            mostrarFps=0;
                        }else{
                            mostrarFps=1;
                        }
                    }
                }else if(colidiuMouseTexto(150, 50, 30, 6, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 4;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        escolhaInicial=0;
                        tela = 0;
                    }
                }
            }
            if(tela==5){
                if(colidiuMouseTexto(30, 50, 20, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela=0; //VOLTA PRA TELA INICAL
                        escolhaInicial=0;
                        if(isMultiplayer){
                            isMultiplayer=0;
                            comecouMultiplayer=0;
                            acabou = 0;
                            DestroiSocketCliente(socket);
                            socket = -1;
                            DestroiLista(inicio);
                            printf("Desconetado do servidor.\n");

                        }
                    }
                }else if(colidiuMouseTexto(50, 150, 20, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 1;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        setClasseSpeed(&objPrincipal);
                        tank_base_obj = CriaObjeto("../imagens/tank_base_2.png",1, 255, 0);
                        DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                        SetDimensoesObjeto(tank_base_obj, 64, 64);
                        tank_top_obj = CriaObjeto("../imagens/tank_top2.png",1, 255, 0);
                        MoveObjeto(tank_base_obj, _x, _y);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                        SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                        MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        MoveObjeto(objMapa, _xMapa, _yMapa);
                        PlayAudio(audio_Andar_Idle);
                        for(int i=0;i<24;i++){
                            MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                        }

                        if(!isMultiplayer){
                            tela=6;
                        }else{
                            tela=4;
                            char msg[40];
                            int tipo = -1;
                            if(objPrincipal.dano==150){
                                tipo=0;
                            }else if(objPrincipal.dano==100){
                                tipo=1;
                            }else if(objPrincipal.dano==250){
                                tipo=2;
                            }else{
                                tipo=3;
                            }
                            sprintf(msg, "criar:%d:%d:%d:%d:%f:%d:%d:%s\0", idMultiplayer, tipo, _x, _y, GetAnguloObjeto(tank_top_obj), movimento, objPrincipal.vivo, input);

                            EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                        }

                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(250, 150, 20, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 2;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        setClasseTank(&objPrincipal);
                        tank_base_obj = CriaObjeto("../imagens/tank_base_1.png",1, 255, 0);
                        DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                        SetDimensoesObjeto(tank_base_obj, 64, 64);
                        tank_top_obj = CriaObjeto("../imagens/tank_top1.png",1, 255, 0);
                        MoveObjeto(tank_base_obj, _x, _y);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                        SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                        MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        MoveObjeto(objMapa, _xMapa, _yMapa);
                        PlayAudio(audio_Andar_Idle);
                        for(int i=0;i<24;i++){
                            MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                        }

                       if(!isMultiplayer){
                            tela=6;
                        }else{
                            tela=4;
                            char msg[40];
                            int tipo = -1;
                            if(objPrincipal.dano==150){
                                tipo=0;
                            }else if(objPrincipal.dano==100){
                                tipo=1;
                            }else if(objPrincipal.dano==250){
                                tipo=2;
                            }else{
                                tipo=3;
                            }
                            sprintf(msg, "criar:%d:%d:%d:%d:%f:%d:%d:%s\0", idMultiplayer, tipo, _x, _y, GetAnguloObjeto(tank_top_obj), movimento, objPrincipal.vivo, input);

                            EnviaDadosSocketCliente(socket, msg, strlen(msg));
                        }
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(450, 150, 20, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 3;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        setClasseKnife(&objPrincipal);
                        tank_base_obj = CriaObjeto("../imagens/tank_base_3.png",1, 255, 0);
                        DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                        SetDimensoesObjeto(tank_base_obj, 64, 64);
                        tank_top_obj = CriaObjeto("../imagens/tank_top3.png",1, 255, 0);
                        MoveObjeto(tank_base_obj, _x, _y);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                        SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                        MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        MoveObjeto(objMapa, _xMapa, _yMapa);
                        PlayAudio(audio_Andar_Idle);
                        for(int i=0;i<24;i++){
                            MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                        }

                        if(!isMultiplayer){
                            tela=6;
                        }else{
                            tela=4;
                            char msg[40];
                            int tipo = -1;
                            if(objPrincipal.dano==150){
                                tipo=0;
                            }else if(objPrincipal.dano==100){
                                tipo=1;
                            }else if(objPrincipal.dano==250){
                                tipo=2;
                            }else{
                                tipo=3;
                            }
                            sprintf(msg, "criar:%d:%d:%d:%d:%f:%d:%d:%s\0", idMultiplayer, tipo, _x, _y, GetAnguloObjeto(tank_top_obj), movimento, objPrincipal.vivo, input);

                            EnviaDadosSocketCliente(socket, msg, strlen(msg));
                        }
                        escolhaInicial=0;
                    }
                }else if(colidiuMouseTexto(650, 150, 20, 10, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 4;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        setClasseReload(&objPrincipal);
                        tank_base_obj = CriaObjeto("../imagens/tank_base_4.png",1, 255, 0);
                        DefineFrameObjeto(tank_base_obj, 0, 0, 64, 64);
                        SetDimensoesObjeto(tank_base_obj, 64, 64);
                        tank_top_obj = CriaObjeto("../imagens/tank_top4.png",1, 255, 0);
                        MoveObjeto(tank_base_obj, _x, _y);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        SetPivoObjeto(tank_base_obj, 0.5f, 0.5f);
                        SetPivoObjeto(tank_top_obj, 0.5f, 0.5f);
                        MoveGeradorParticulas(geradordeParticulas, _x+32, _y+32);
                        MoveObjeto(tank_top_obj, _x+_xOffSet, _y+_yOffSet);
                        MoveObjeto(objMapa, _xMapa, _yMapa);
                        PlayAudio(audio_Andar_Idle);
                        for(int i=0;i<24;i++){
                            MoveObjeto(objCenario[i].objId, objCenario[i].xInicial+_xMapa, objCenario[i].yInicial+_yMapa);
                        }

                        if(!isMultiplayer){
                            tela=6;
                        }else{
                            tela=4;
                            char msg[40];
                            int tipo = -1;
                            if(objPrincipal.dano==150){
                                tipo=0;
                            }else if(objPrincipal.dano==100){
                                tipo=1;
                            }else if(objPrincipal.dano==250){
                                tipo=2;
                            }else{
                                tipo=3;
                            }
                            sprintf(msg, "criar:%d:%d:%d:%d:%f:%d:%d:%s\0", idMultiplayer, tipo, _x, _y, GetAnguloObjeto(tank_top_obj), movimento, objPrincipal.vivo, input);

                            EnviaDadosSocketCliente(socket, msg, strlen(msg));
                        }
                        escolhaInicial=0;
                    }
                }
            }
            if(tela==8){
                if(colidiuMouseTexto(30, 50, 20, 5, 1, evento.mouse.posX, evento.mouse.posY)){
                    escolhaInicial = 0;
                    if(evento.mouse.acao==MOUSE_PRESSIONADO && evento.mouse.botao == MOUSE_ESQUERDO){
                        tela=0; //VOLTA PRA TELA INICAL
                        escolhaInicial=0;
                    }
                }
            }
        }

        if(evento.tipoEvento == EVENTO_TECLADO){
            if(tela==3){
                if(evento.teclado.acao == TECLA_PRESSIONADA){
                    if(evento.teclado.tecla == TECLA_SHIFTESQUERDO){
                        capsLock=1;
                    }
                    if(evento.teclado.tecla == TECLA_CAPSLOCK){
                        if(capsLock)capsLock=0;
                        else capsLock=1;
                    }
                    if(!mouseClicado){
                        if(evento.teclado.tecla == TECLA_BACKSPACE){
                            if(letrasInput>0){
                                letrasInput--;
                                input[letrasInput] = ' ';
                            }
                        }else{
                            if(letrasInput<19){
                                char teste = pegarTeclado(evento, capsLock);
                                if(teste!=NULL){
                                    input[letrasInput] = teste;
                                    letrasInput++;
                                }
                            }
                        }
                    }else{
                        if(evento.teclado.tecla == TECLA_BACKSPACE){
                            if(letrasInput2>0){
                                letrasInput2--;
                                input2[letrasInput2] = ' ';
                            }
                        }else{
                            if(letrasInput2<19){
                                char teste = pegarTeclado(evento, capsLock);
                                if(teste!=NULL){
                                    input2[letrasInput2] = teste;
                                    letrasInput2++;
                                }
                            }
                        }
                    }
                }
                if(evento.teclado.acao == TECLA_LIBERADA && evento.teclado.tecla == TECLA_SHIFTESQUERDO){
                    capsLock = 0;
                }
                if(evento.teclado.acao == TECLA_LIBERADA && evento.teclado.tecla == TECLA_TAB){
                    if(mouseClicado)mouseClicado=0;
                    else mouseClicado=1;
                }
            }
        }

        if(evento.tipoEvento == EVENTO_REDE){
            if(evento.rede.tipoMensagem == REDE_MENSAGEM_TCP){
                char msg[100];
                sprintf(msg, "%s", evento.rede.mensagem);

                if(strncmp(msg, "id:", 3)==0){
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    idMultiplayer = atoi(cortada);
                }
                if(strncmp(msg, "fechado:", 8)==0){
                    tela=0;
                    isMultiplayer=0;
                    idMultiplayer=-1;
                    printf("Esse servidor ja iniciou!\n");
                }
                if(strncmp(msg, "criar:", 6)==0){
                    int tipo=-1, id=-1, x=0, y=0, angle=0, mov=0, vivo=0;
                    char nome[20];
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    tipo = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    x = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    y = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    angle = atof(cortada);
                    cortada = strtok(NULL, ":");
                    mov = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    vivo = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    strcpy(nome, cortada);

                    if(id!=idMultiplayer){
                        criarAdversario(inicio, id, x, y, angle, tipo, vivo, mov, nome);
                    }
                }
                if(strncmp(msg, "andar:", 6)==0){
                    int id=-1, xX=0, yY=0, xMapa=0, yMapa=0, mov=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    xX = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    yY = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    xMapa = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    yMapa = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    mov = atoi(cortada);

                    if(id!=idMultiplayer){
                        ObjetoAdversario *aqui = Busca(inicio, id);
                        if(aqui!=NULL){
                            aqui->x = xX;
                            aqui->y = yY;
                            aqui->xMapa = xMapa;
                            aqui->yMapa = yMapa;
                            aqui->movimento = mov;

                            if(aqui->movimento==3){
                                SetAnguloObjeto(aqui->objId, 90);
                            }else if(aqui->movimento==2){
                                SetAnguloObjeto(aqui->objId, 270);
                            }else if(aqui->movimento==1){
                                SetAnguloObjeto(aqui->objId, 180);
                            }else{
                                SetAnguloObjeto(aqui->objId, 0);
                            }
                        }
                    }
                }
                if(strncmp(msg, "topmove:", 8)==0){
                    int id=-1, angle=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    angle = atof(cortada);

                    if(id!=idMultiplayer){
                        ObjetoAdversario *aqui = Busca(inicio, id);
                        if(aqui!=NULL){
                            aqui->angle = angle;
                            SetAnguloObjeto(aqui->objTopoId, angle);
                        }
                    }
                }
                if(strncmp(msg, "desconectado:", 13)==0){
                    int id=-1;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);

                    if(id!=idMultiplayer){
                        Remocao(inicio, id);
                    }
                }
                if(strncmp(msg, "comecou:", 8)==0){
                    comecouMultiplayer=1;
                    strcpy(aviso, "A partida começou!");
                    auxAviso=3;
                }
                if(strncmp(msg, "tiro:", 5)==0){
                    int id=-1, tipo=0;
                    float xMouse=0, yMouse=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    xMouse = atof(cortada);
                    cortada = strtok(NULL, ":");
                    yMouse = atof(cortada);
                    cortada = strtok(NULL, ":");
                    tipo = atoi(cortada);

                    if(id!=idMultiplayer){
                        ObjetoAdversario *aqui = Busca(inicio, id);
                        if(aqui!=NULL){
                            int xaIr = (aqui->x + _xMapa+ 50*sin(grausParaRadianos(-aqui->angle)))+32;
                            int yaIr = (aqui->y + _yMapa+ 50*cos(grausParaRadianos(-aqui->angle)))+32;
                            if(tipo==0){
                                MoveGeradorParticulas(geradordeParticulasInimigoSpeed, xaIr, yaIr);
                                MudaDirecaoParticulas(geradordeParticulasInimigoSpeed, xMouse, yMouse);
                                CriaParticula(geradordeParticulasInimigoSpeed, 1, 0,0, 800+1000, 600+1000, 5);
                            }else if(tipo==1){
                                MoveGeradorParticulas(geradordeParticulasInimigoTank, xaIr, xaIr);
                                MudaDirecaoParticulas(geradordeParticulasInimigoTank, xMouse, yMouse);
                                CriaParticula(geradordeParticulasInimigoTank, 1, 0,0, 800+1000, 600+1000, 5);
                            }else if(tipo==2){
                                MoveGeradorParticulas(geradordeParticulasInimigoKnife, xaIr, yaIr);
                                MudaDirecaoParticulas(geradordeParticulasInimigoKnife, xMouse, yMouse);
                                CriaParticula(geradordeParticulasInimigoKnife, 1, 0,0, 800+1000, 600+1000, 5);
                            }else{
                                MoveGeradorParticulas(geradordeParticulasInimigoReload, xaIr, yaIr);
                                MudaDirecaoParticulas(geradordeParticulasInimigoReload, xMouse, yMouse);
                                CriaParticula(geradordeParticulasInimigoReload, 1, 0,0, 800+1000, 600+1000, 5);
                            }

                            PlayAudio(audio_Tiro2);

                        }
                    }
                }
                if(strncmp(msg, "vivo:", 5)==0){
                    int id=-1, vivo=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    vivo = atoi(cortada);

                    if(id!=idMultiplayer){
                        ObjetoAdversario *aqui = Busca(inicio, id);
                        if(aqui!=NULL){
                            aqui->vivo=vivo;
                            aqui->principal.vivo = vivo;
                            DefineFrameObjeto(aqui->objId, 64, 0, 64, 64);
                        }
                    }
                }
                if(strncmp(msg, "vida:", 5)==0){
                    int id=-1, vida=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    vida = atoi(cortada);

                    if(id!=idMultiplayer){
                        ObjetoAdversario *aqui = Busca(inicio, id);
                        if(aqui!=NULL){
                            aqui->principal.vida = vida;
                        }
                    }
                }
                if(strncmp(msg, "quantidade:", 11)==0){
                    int id=-1, quantidade=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    quantidade = atoi(cortada);

                    if(quantidade!=1){
                        char aviso2[40];
                        sprintf(aviso2, "Restam %d jogadores.", quantidade);

                        strcpy(aviso, aviso2);
                        auxAviso = 3;
                    }
                }
                if(strncmp(msg, "vencedor:", 9)==0){
                    int id=-1;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    id = atoi(cortada);

                    ObjetoAdversario *aqui = Busca(inicio, id);

                    acabou = 1;

                    if(aqui!=NULL){
                        strcpy(vencedor, aqui->nome);
                    }else{
                        if(idMultiplayer==id){
                            strcpy(vencedor, input);
                        }else{
                            strcpy(vencedor, "Alguém");
                        }
                    }
                }
                if(strncmp(msg, "caixa:", 6)==0){
                    int xCaixa= 0, yCaixa =0, usada=0;
                    char *cortada = strtok(msg, ":");
                    cortada = strtok(NULL, ":");
                    xCaixa = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    yCaixa = atoi(cortada);
                    cortada = strtok(NULL, ":");
                    usada = atoi(cortada);

                    caixa.x = xCaixa;
                    caixa.y = yCaixa;
                    caixa.usada = usada;
                }
            }
            if(evento.rede.tipoMensagem == REDE_DESCONEXAO){
                isMultiplayer=0;
                comecouMultiplayer=0;
                DestroiSocketCliente(socket);
                socket = -1;
                DestroiLista(inicio);
                tela=0;
                acabou = 0;
                printf("Desconectado do servidor.\n");
            }
        }

        if(tela==4 && ColisaoParticulasObjeto(geradordeParticulasInimigoTank, tank_base_obj)){
            if(objPrincipal.vivo){
                if(objPrincipal.vida-100<=0){
                    objPrincipal.vida = 0;
                }else{
                    objPrincipal.vida-= 100;
                }

                char msg[40];
                if(isMultiplayer){
                    sprintf(msg, "vida:%d:%d\0", idMultiplayer, objPrincipal.vida);
                    EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                }
                if(objPrincipal.vida<=0){
                    objPrincipal.vida = 0;

                    if(isMultiplayer){
                        sprintf(msg, "vivo:%d:%d\0",idMultiplayer, 0);
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                    }

                    PlayAudio(audio_Explosao);

                    DefineFrameObjeto(tank_base_obj, 64, 0, 64, 64);
                }
            }
        }
        if(tela==4 && ColisaoParticulasObjeto(geradordeParticulasInimigoSpeed, tank_base_obj)){
            if(objPrincipal.vivo){
                if(objPrincipal.vida-150<=0){
                    objPrincipal.vida = 0;
                }else{
                    objPrincipal.vida-= 150;
                }

                char msg[40];
                if(isMultiplayer){
                    sprintf(msg, "vida:%d:%d\0",idMultiplayer, objPrincipal.vida);
                    EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                }
                if(objPrincipal.vida<=0){
                    objPrincipal.vivo=0;

                    if(isMultiplayer){
                        sprintf(msg, "vivo:%d:%d\0",idMultiplayer, objPrincipal.vivo);
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                    }

                    PlayAudio(audio_Explosao);

                    DefineFrameObjeto(tank_base_obj, 64, 0, 64, 64);
                }
            }
        }
        if(tela==4 && ColisaoParticulasObjeto(geradordeParticulasInimigoKnife, tank_base_obj)){
            if(objPrincipal.vivo){
                if(objPrincipal.vida-250<=0){
                    objPrincipal.vida = 0;
                }else{
                    objPrincipal.vida -= 250;
                }
                char msg[40];
                if(isMultiplayer){
                    sprintf(msg, "vida:%d:%d\0",idMultiplayer, objPrincipal.vida);
                    EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                }
                if(objPrincipal.vida<=0){
                    objPrincipal.vivo=0;

                    if(isMultiplayer){
                        sprintf(msg, "vivo:%d:%d\0",idMultiplayer, objPrincipal.vivo);
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                    }

                    PlayAudio(audio_Explosao);

                    DefineFrameObjeto(tank_base_obj, 64, 0, 64, 64);
                }
            }
        }
        if(tela==4 && ColisaoParticulasObjeto(geradordeParticulasInimigoReload, tank_base_obj)){
            if(objPrincipal.vivo){
                if(objPrincipal.vida-125<=0){
                    objPrincipal.vida = 0;
                }else{
                    objPrincipal.vida-= 125;
                }
                 char msg[40];
                if(isMultiplayer){
                    sprintf(msg, "vida:%d:%d\0",idMultiplayer, objPrincipal.vida);
                    EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                }
                if(objPrincipal.vida<=0){
                    objPrincipal.vivo=0;

                    if(isMultiplayer){
                        sprintf(msg, "vivo:%d:%d\0",idMultiplayer, objPrincipal.vivo);
                        EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
                    }

                    PlayAudio(audio_Explosao);

                    DefineFrameObjeto(tank_base_obj, 64, 0, 64, 64);
                }
            }
        }

        if(isMultiplayer && tela==4){
            ObjetoAdversario *aux = inicio->prox;
            while(aux!=inicio){
                if(ColisaoParticulasObjeto(geradordeParticulas, aux->objId)){
                    PlayAudio(audio_Tomou_Tiro);

                }
                if(ColisaoParticulasObjeto(geradordeParticulasInimigoSpeed, aux->objId)){
                    PlayAudio(audio_Tomou_Tiro);

                }
                if(ColisaoParticulasObjeto(geradordeParticulasInimigoTank, aux->objId)){
                    PlayAudio(audio_Tomou_Tiro);

                }
                if(ColisaoParticulasObjeto(geradordeParticulasInimigoKnife, aux->objId)){
                    PlayAudio(audio_Tomou_Tiro);
                }
                if(ColisaoParticulasObjeto(geradordeParticulasInimigoReload, aux->objId)){
                    PlayAudio(audio_Tomou_Tiro);
                }
                aux = aux->prox;
            }
        }

        for(int i=0;i<24;i++){
            if(ColisaoParticulasObjeto(geradordeParticulas, objCenario[i].objId)){
                    //som
            }
            if(ColisaoParticulasObjeto(geradordeParticulasInimigoKnife, objCenario[i].objId)){
                    //som
            }
            if(ColisaoParticulasObjeto(geradordeParticulasInimigoReload, objCenario[i].objId)){
                    //som
            }
            if(ColisaoParticulasObjeto(geradordeParticulasInimigoSpeed, objCenario[i].objId)){
                    //som
            }
            if(ColisaoParticulasObjeto(geradordeParticulasInimigoTank, objCenario[i].objId)){
                    //som
            }
        }

        if(tela==4 && auxCaixa<=0 && !pause && !isMultiplayer){
            Coordenadas coor = getCoordenadasAleatorias(inimigos, quantidadeInimigos, objCenario);
            caixa.usada = 1;
            caixa.x = coor.x;
            caixa.y = coor.y;
            auxCaixa = 180; // outra caixa aparece em 3 minutos
        }

        if(!isMultiplayer && tela==4 && TestaColisaoObjetos(tank_base_obj, objCaixa)){
            objPrincipal.vida += 500;
            caixa.x = 3000;
            caixa.y = 3000;
            caixa.usada=0;

            if(isMultiplayer){
                char msg[100];
                sprintf(msg, "caixa:%d:%d\0", caixa.x, caixa.y, caixa.usada);
                EnviaDadosSocketCliente(socket, msg, strlen(msg)+1);
            }
            //som
        }

        if(objPrincipal.municao == 0 && objPrincipal.auxRecarga==0){
            objPrincipal.auxRecarga = objPrincipal.tempoReload;
            objPrincipal.municao--;
            PlayAudio(audio_Recarregar);
        }
        if(objPrincipal.municao==-1 && objPrincipal.auxRecarga==0){
            objPrincipal.municao = 20;
        }

        if(tela==4 && !objPrincipal.vivo){
            StopAudio(audio_Andar_Idle);
        }

        if(tela==4 && !pause && !isMultiplayer){
        if(inimigosIniciados){
            transparencia = 255;
            for(int i=0;i<quantidadeInimigos;i++){
                 if(ColisaoParticulasObjeto(geradordeParticulas, inimigos[i].objId)){
                    if(inimigos[i].vivo){
                        inimigos[i].principal.vida -= objPrincipal.dano;
                        PlayAudio(audio_Tomou_Tiro);
                        if(inimigos[i].principal.vida<=0){
                            inimigos[i].vivo = 0;
                            quantidadeInimigosVivos--;
                            DefineFrameObjeto(inimigos[i].objId, 64, 0, 64, 64);

                            PlayAudio(audio_Explosao);
                         }
                         //som
                    }
                 }
            }

            if(quantidadeInimigosVivos!=0){
                auxDelay=5;
            }

            if(quantidadeInimigosVivos==0 && auxDelay==0){
                fase++;
                destruirInimigos(&inimigos);
                objPrincipal.vida = objPrincipal.vidaTotal;
                objPrincipal.municao = 20;
                quantidadeInimigos = calculaQuantidadeInimigos(fase);
                quantidadeInimigosVivos = quantidadeInimigos;
                iniciaInimigos(&inimigos, quantidadeInimigos, objCenario);

                salvarJogo(&arquivoSave, &arquivoSaveNome, _x, _y, _xMapa, _yMapa, caixa.x, caixa.y, objPrincipal, dificuldade, fase, &inimigos, quantidadeInimigos);
            }

            // calculo do top dos inimigos
            if(objPrincipal.vivo==1){
                for(int i=0;i<quantidadeInimigos;i++){
                    if(!inimigos[i].vivo)continue;
                    int xw, yw, temAlgo=0;
                    float tamw;
                    xw = (inimigos[i].x+32)-_x+_xMapa;
                    yw = (inimigos[i].y+32)-_y+_yMapa;
                    tamw = (float)sqrt(pow(xw,2)+pow(yw,2));

                    if((inimigos[i].y+_yMapa)>400 && (inimigos[i].y+_yMapa)<600 && (inimigos[i].x+_xMapa)<800 && (inimigos[i].x+_xMapa)>0){
                        transparencia=100;
                    }

                    if(tamw <= 300*dificuldade){
                        inimigos[i].vendoInimigo=1;
                        for(int j=7;j<15;j++){ //checar somente os predios que ficam no meio
                            if(j==8)continue; //pula o predio2 que nao é no meio
                            if(obstaculoEntrePlayerInimigo(_x, _y, _xMapa, _yMapa, xw, yw, objCenario[j])){
                                inimigos[i].vendoInimigo = 0;
                                break;
                            }
                        }
                        if(inimigos[i].vendoInimigo==1){
                            inimigos[i].ultimoVistoX = _x-_xMapa;
                            inimigos[i].ultimoVistoY = _y-_yMapa;
                            float angle = atan2(_x-inimigos[i].x-_xMapa, _y-inimigos[i].y-_yMapa);
                            SetAnguloObjeto(inimigos[i].objTopoId, -radianosParaGraus(angle));
                            if(tamw <= (200*dificuldade)){
                                if(!inimigos[i].fugindo){
                                    inimigos[i].angleAndando = angle;
                                    if(dificuldade==1){
                                        angle+= (float)((rand()%100)-50)/100;
                                    }else if(dificuldade==2){
                                        angle+= (float)((rand()%50)-25)/100;
                                    }
                                    if(inimigos[i].recarregando<=0){
                                        if(inimigos[i].tipo == 1){
                                            MoveGeradorParticulas(geradordeParticulasInimigoTank, inimigos[i].x+_xMapa+32, inimigos[i].y+_yMapa+32);
                                            MudaDirecaoParticulas(geradordeParticulasInimigoTank, normalizarVetorX(sin(angle), cos(angle), 500), normalizarVetorY(sin(angle), cos(angle), 500));

                                            CriaParticula(geradordeParticulasInimigoTank, 1, 0,0, 800+50, 600+50, 5);
                                            PlayAudio(audio_Tiro2);
                                            inimigos[i].recarregando = inimigos[i].principal.tempoRecarga;
                                            inimigos[i].principal.municao--;
                                            if(inimigos[i].principal.municao<=0){
                                                inimigos[i].recarregando = inimigos[i].principal.tempoReload;
                                                inimigos[i].fugindo=1;
                                            }
                                        }else if(inimigos[i].tipo == 2){
                                            MoveGeradorParticulas(geradordeParticulasInimigoSpeed, inimigos[i].x+_xMapa+32, inimigos[i].y+_yMapa+32);
                                            MudaDirecaoParticulas(geradordeParticulasInimigoSpeed, normalizarVetorX(sin(angle), cos(angle), 500), normalizarVetorY(sin(angle), cos(angle), 500));

                                            CriaParticula(geradordeParticulasInimigoSpeed, 1, 0,0, 800+50, 600+50, 5);
                                            PlayAudio(audio_Tiro2);
                                            inimigos[i].recarregando = inimigos[i].principal.tempoRecarga;
                                            inimigos[i].principal.municao--;
                                            if(inimigos[i].principal.municao<=0){
                                                inimigos[i].recarregando = inimigos[i].principal.tempoReload;
                                                inimigos[i].fugindo=1;
                                            }
                                        }else if(inimigos[i].tipo == 3){
                                            MoveGeradorParticulas(geradordeParticulasInimigoKnife, inimigos[i].x+_xMapa+32, inimigos[i].y+_yMapa+32);
                                            MudaDirecaoParticulas(geradordeParticulasInimigoKnife, normalizarVetorX(sin(angle), cos(angle), 500), normalizarVetorY(sin(angle), cos(angle), 500));

                                            CriaParticula(geradordeParticulasInimigoKnife, 1, 0,0, 800+50, 600+50, 5);
                                            PlayAudio(audio_Tiro2);
                                            inimigos[i].recarregando = inimigos[i].principal.tempoRecarga;
                                            inimigos[i].principal.municao--;
                                            if(inimigos[i].principal.municao<=0){
                                                inimigos[i].recarregando = inimigos[i].principal.tempoReload;
                                                inimigos[i].fugindo=1;
                                            }
                                        }else if(inimigos[i].tipo == 4){
                                            MoveGeradorParticulas(geradordeParticulasInimigoReload, inimigos[i].x+_xMapa+32, inimigos[i].y+_yMapa+32);
                                            MudaDirecaoParticulas(geradordeParticulasInimigoReload, normalizarVetorX(sin(angle), cos(angle), 500), normalizarVetorY(sin(angle), cos(angle), 500));

                                            CriaParticula(geradordeParticulasInimigoReload, 1, 0,0, 800+50, 600+50, 5);
                                            PlayAudio(audio_Tiro2);
                                            inimigos[i].recarregando = inimigos[i].principal.tempoRecarga;
                                            inimigos[i].principal.municao--;
                                            if(inimigos[i].principal.municao<=0){
                                                inimigos[i].recarregando = inimigos[i].principal.tempoReload;
                                                inimigos[i].fugindo=1;
                                            }
                                        }
                                    }
                                }else{
                                    if(inimigos[i].recarregando<=0){
                                        inimigos[i].principal.municao=20;
                                        inimigos[i].fugindo=0;
                                    }
                                    //fugir
                                    fugirPlayer(&inimigos[i], _x, _y, _xMapa, _yMapa, objCenario);
                                }
                            }else{
                                if(!inimigos[i].fugindo){
                                    //seguir
                                    seguirPlayer(&inimigos[i], _x, _y, _xMapa, _yMapa, objCenario);
                                }else{
                                    //fugir
                                    fugirPlayer(&inimigos[i], _x, _y, _xMapa, _yMapa, objCenario);
                                }
                            }
                        }else{
                            //seguirUltimoVisto
                            seguirUltimoVisto(&inimigos[i], inimigos[i].ultimoVistoX, inimigos[i].ultimoVistoY, _xMapa, _yMapa, objCenario);
                        }
                    }else{
                        if(inimigos[i].fugindo){
                            //fugir
                            fugirPlayer(&inimigos[i], _x, _y, _xMapa, _yMapa, objCenario);
                        }else{
                            //andar aleatoriamente
                            andarAleatoriamente(&inimigos[i], _xMapa, _yMapa, objCenario);
                        }
                    }
                    //mudar textura
                    if(inimigos[i].andandoEmX){
                        if(inimigos[i].andandoPositivo){
                            SetAnguloObjeto(inimigos[i].objId, 270);
                        }else{
                            SetAnguloObjeto(inimigos[i].objId, 90);
                        }
                    }else{
                        if(inimigos[i].andandoPositivo){
                            SetAnguloObjeto(inimigos[i].objId, 0);
                        }else{
                            SetAnguloObjeto(inimigos[i].objId, 180);
                        }
                    }
                }
            }
            }
        }

        IniciaDesenho();

        switch(tela){
            case 0:
                DesenhaObjeto(objfundo);
                EscreverCentralizada("Tank Multiplayer 3000", 400, 550, fonte1);

                switch(escolhaInicial){
                    case 0:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte3);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 1:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte3);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 2:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte3);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 3:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte3);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 4:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte3);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 5:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte3);
                        EscreverEsquerda("Sair", 50, 50, fonte2);
                        break;
                    case 6:
                        EscreverEsquerda("Novo Jogo", 50, 350, fonte2);
                        EscreverEsquerda("Carregar Jogo", 50, 300, fonte2);
                        EscreverEsquerda("Multiplayer", 50, 250, fonte2);
                        EscreverEsquerda("Opções", 50, 200, fonte2);
                        EscreverEsquerda("Ajuda", 50, 150, fonte2);
                        EscreverEsquerda("Créditos", 50, 100, fonte2);
                        EscreverEsquerda("Sair", 50, 50, fonte3);
                        break;
                    default:
                        break;
                    }
                    break;
            case 1:
                DesenhaObjeto(objfundo);
                EscreverCentralizada("Créditos", 400, 550, fonte1);

                EscreverEsquerda("Jogo criado por:", 150, 250, fonte2);
                EscreverEsquerda("Diogo Zaror", 150, 200, fonte2);

                EscreverEsquerda("Voltar", 150, 100, fonte3);

                break;
            case 2:
                DesenhaObjeto(objfundo);
                EscreverCentralizada("Opções", 400, 550, fonte1);

                char vmusica[25];
                sprintf(vmusica,"Volume da Música:   %d", volumeMusica);
                char vjogo[25];
                sprintf(vjogo,"Volume do Jogo:   %d", volumeJogo);

                switch(escolhaInicial){
                    case 0:
                        EscreverEsquerda(vmusica, 150, 250, fonte3);
                        EscreverEsquerda(vjogo, 150, 200, fonte2);
                        if(fullscreen==0){
                            EscreverEsquerda("Modo Fullscreen", 150, 150, fonte2);
                        }else{
                            EscreverEsquerda("Modo Janela", 150, 150, fonte2);
                        }
                        if(mostrarFps==0){
                            EscreverEsquerda("Mostrar FPS", 150, 100, fonte2);
                        }else{
                            EscreverEsquerda("Esconder FPS", 150, 100, fonte2);
                        }
                        EscreverEsquerda("Voltar", 150, 50, fonte2);
                        break;
                    case 1:
                        EscreverEsquerda(vmusica, 150, 250, fonte2);
                        EscreverEsquerda(vjogo, 150, 200, fonte3);
                        if(fullscreen==0){
                            EscreverEsquerda("Modo Fullscreen", 150, 150, fonte2);
                        }else{
                            EscreverEsquerda("Modo Janela", 150, 150, fonte2);
                        }
                         if(mostrarFps==0){
                            EscreverEsquerda("Mostrar FPS", 150, 100, fonte2);
                        }else{
                            EscreverEsquerda("Esconder FPS", 150, 100, fonte2);
                        }
                        EscreverEsquerda("Voltar", 150, 50, fonte2);
                        break;
                    case 2:
                        EscreverEsquerda(vmusica, 150, 250, fonte2);
                        EscreverEsquerda(vjogo, 150, 200, fonte2);
                        if(fullscreen==0){
                            EscreverEsquerda("Modo Fullscreen", 150, 150, fonte3);
                        }else{
                            EscreverEsquerda("Modo Janela", 150, 150, fonte3);
                        }
                         if(mostrarFps==0){
                            EscreverEsquerda("Mostrar FPS", 150, 100, fonte2);
                        }else{
                            EscreverEsquerda("Esconder FPS", 150, 100, fonte2);
                        }

                        EscreverEsquerda("Voltar", 150, 50, fonte2);
                        break;
                    case 3:
                        EscreverEsquerda(vmusica, 150, 250, fonte2);
                        EscreverEsquerda(vjogo, 150, 200, fonte2);
                        if(fullscreen==0){
                            EscreverEsquerda("Modo Fullscreen", 150, 150, fonte2);
                        }else{
                            EscreverEsquerda("Modo Janela", 150, 150, fonte2);
                        }
                         if(mostrarFps==0){
                            EscreverEsquerda("Mostrar FPS", 150, 100, fonte3);
                        }else{
                            EscreverEsquerda("Esconder FPS", 150, 100, fonte3);
                        }

                        EscreverEsquerda("Voltar", 150, 50, fonte2);
                        break;
                    case 4:
                        EscreverEsquerda(vmusica, 150, 250, fonte2);
                        EscreverEsquerda(vjogo, 150, 200, fonte2);
                        if(fullscreen==0){
                            EscreverEsquerda("Modo Fullscreen", 150, 150, fonte2);
                        }else{
                            EscreverEsquerda("Modo Janela", 150, 150, fonte2);
                        }
                         if(mostrarFps==0){
                            EscreverEsquerda("Mostrar FPS", 150, 100, fonte2);
                        }else{
                            EscreverEsquerda("Esconder FPS", 150, 100, fonte2);
                        }

                        EscreverEsquerda("Voltar", 150, 50, fonte3);
                        break;
                    default:
                        break;
                    }
                    break;
                case 3:
                    DesenhaRetangulo(0,0, 800, 1200, CINZA);
                    EscreverCentralizada("Digite as informações", 365, 550, fonte4);

                    if(mouseClicado){
                        DesenhaRetangulo(195, 380, 60, 350, PRETO, 0);
                    }else{
                        DesenhaRetangulo(195, 180, 50, 350, PRETO, 0);
                    }

                    EscreverEsquerda("Digite o ip:", 200, 450, fonte5);
                    DesenhaRetangulo(200, 190, 50, 350, BRANCO, 0);
                    EscreverEsquerda(input, 200, 200, fonte2);
                    DesenhaRetangulo(560, 390, 50, 50, BRANCO);
                    EscreverEsquerda("Digite um nome:", 200, 250, fonte5);
                    DesenhaRetangulo(200, 390, 50, 350, BRANCO, 0);
                    EscreverEsquerda(input2, 200, 400, fonte2);

                    if(escolhaInicial==1){
                        EscreverEsquerda("->", 570, 400, fonte3);
                    }else{
                        EscreverEsquerda("->", 570, 400, fonte2);
                    }
                    if(escolhaInicial==0){
                        EscreverEsquerda("Voltar", 100, 50, fonte3);
                    }else{
                        EscreverEsquerda("Voltar", 100, 50, fonte2);
                    }

                    break;
                case 4:

                DesenhaObjeto(objMapa, 0);
                DesenhaObjeto(tank_base_obj);

                if(objPrincipal.vivo)DesenhaObjeto(tank_top_obj);

                for(int i=0;i<24;i++){
                    DesenhaObjeto(objCenario[i].objId, 0);
                }

                if(!isMultiplayer){
                    for(int i=0;i<quantidadeInimigos;i++){
                        MoveObjeto(inimigos[i].objId, inimigos[i].x+_xMapa, inimigos[i].y+_yMapa);
                        MoveObjeto(inimigos[i].objTopoId, inimigos[i].x+_xMapa, inimigos[i].y+_yMapa);
                        DesenhaObjeto(inimigos[i].objId);
                        if(inimigos[i].vivo){
                            DesenhaObjeto(inimigos[i].objTopoId);
                            desenhaVida(inimigos[i].x+_xMapa, inimigos[i].y+_yMapa+70, 5, 64, inimigos[i].principal.vida, inimigos[i].principal.vidaTotal, 255, 0);
                            if(inimigos[i].principal.municao <= 0){
                                DesenhaRetangulo(inimigos[i].x+_xMapa+70, inimigos[i].y+_yMapa+70, 5, 5, AZUL, 0);
                            }
                        }
                    }
                }else{
                    ObjetoAdversario *aux = inicio->prox;
                    while(aux!=inicio){
                        MoveObjeto(aux->objId, aux->x+_xMapa, aux->y+_yMapa);
                        MoveObjeto(aux->objTopoId, aux->x+_xMapa, aux->y+_yMapa);

                        DesenhaObjeto(aux->objId);
                        if(aux->principal.vida){
                            DesenhaObjeto(aux->objTopoId);
                            desenhaVida(aux->x+_xMapa, aux->y+_yMapa+70, 5, 64, aux->principal.vida, aux->principal.vidaTotal, 255, 0);
                            EscreverEsquerda(aux->nome, aux->x+_xMapa, aux->y+_yMapa+80, fonte10);
                        }
                        aux = aux->prox;
                    }
                }

                MoveParticulas(geradordeParticulas);
                DesenhaParticulas(geradordeParticulas);
                MoveParticulas(geradordeParticulasInimigoTank);
                DesenhaParticulas(geradordeParticulasInimigoTank);
                MoveParticulas(geradordeParticulasInimigoSpeed);
                DesenhaParticulas(geradordeParticulasInimigoSpeed);
                MoveParticulas(geradordeParticulasInimigoKnife);
                DesenhaParticulas(geradordeParticulasInimigoKnife);
                MoveParticulas(geradordeParticulasInimigoReload);
                DesenhaParticulas(geradordeParticulasInimigoReload);

                if(!isMultiplayer)
                DesenhaObjeto(objCaixa, 0);

                PIG_Cor branco;
                branco.r = 255;
                branco.g = 255;
                branco.b = 255;
                branco.a = transparencia;

                DesenhaRetangulo(0, 550, 50, 420, branco);

                char municao[25];

                if(objPrincipal.municao!=-1){
                    sprintf(municao, "Munição: %d", objPrincipal.municao);
                    DesenhaRetangulo(0, 520, 40, 180, branco);
                }else{
                    sprintf(municao, "Recarregando %.1f", objPrincipal.auxRecarga);
                    DesenhaRetangulo(0, 520, 40, 300, branco);
                }

                if(transparencia==255){
                    EscreverEsquerda("Vida: ", 5, 565, 3);
                    desenhaVida(100, 560, 30, 300, objPrincipal.vida, objPrincipal.vidaTotal, transparencia, 0);
                    EscreverEsquerda(municao, 5, 530, 3);
                }else{
                    EscreverEsquerda("Vida: ", 5, 565, 9);
                    desenhaVida(100, 560, 30, 300, objPrincipal.vida, objPrincipal.vidaTotal, transparencia, 0);
                    EscreverEsquerda(municao, 5, 530, 9);
                }

                if(!isMultiplayer){
                    desenhaMiniMapa(690, 490, 100, objCenario, inimigos, quantidadeInimigos, _x, _y, _xMapa, _yMapa, caixa.x, caixa.y, dificuldade, transparencia);
                }else{
                    EscreverEsquerda(input, _x, _y+70, fonte10);
                    desenhaMiniMapa(690, 490, 100, objCenario, _x, _y, _xMapa, _yMapa, caixa.x, caixa.y, transparencia, inicio);
                }

                if(auxAviso>0){
                    fazerAviso(aviso, auxAviso, fonte5);
                }

                if(quantidadeInimigosVivos==0 && auxDelay!=0){
                    char faseProxima[25];
                    sprintf(faseProxima, "Iniciando fase %d em %.1f", fase+1, auxDelay);

                    DesenhaRetangulo(0, 0, 50, 350, BRANCO, 0);
                    EscreverEsquerda(faseProxima, 30, 20, fonte5);
                }

                if(pause){
                    if(!opcoes){
                        DesenhaRetangulo(270, 260, 250, 200, BRANCO);
                        switch(escolhaInicial){
                        case 0:
                            EscreverEsquerda("Voltar", 320, 450, fonte3);
                            EscreverEsquerda("Opções", 320, 400, fonte2);
                            EscreverEsquerda("Sair", 320, 300, fonte2);
                            break;

                        case 1:
                            EscreverEsquerda("Voltar", 320, 450, fonte2);
                            EscreverEsquerda("Opções", 320, 400, fonte3);
                            EscreverEsquerda("Sair", 320, 300, fonte2);
                            break;

                        case 2:
                            EscreverEsquerda("Voltar", 320, 450, fonte2);
                            EscreverEsquerda("Opções", 320, 400, fonte2);
                            EscreverEsquerda("Sair", 320, 300, fonte3);
                            break;

                        default:
                            break;
                        }
                    }else{
                        DesenhaRetangulo(270, 180, 320, 370, BRANCO);

                        char vmusica[25];
                        sprintf(vmusica,"Volume da Música:   %d", volumeMusica);
                        char vjogo[25];
                        sprintf(vjogo,"Volume do Jogo:   %d", volumeJogo);

                        switch(escolhaInicial){
                            case 0:
                                EscreverEsquerda(vmusica, 300, 450, fonte3);
                                EscreverEsquerda(vjogo, 300, 400, fonte2);
                                if(fullscreen==0){
                                    EscreverEsquerda("Modo Fullscreen", 300, 350, fonte2);
                                }else{
                                    EscreverEsquerda("Modo Janela", 300, 350, fonte2);
                                }
                                if(mostrarFps==0){
                                    EscreverEsquerda("Mostrar FPS", 300, 300, fonte2);
                                }else{
                                    EscreverEsquerda("Esconder FPS", 300, 300, fonte2);
                                }
                                EscreverEsquerda("Voltar", 300, 200, fonte2);
                                break;

                            case 1:
                                EscreverEsquerda(vmusica, 300, 450, fonte2);
                                EscreverEsquerda(vjogo, 300, 400, fonte3);
                                if(fullscreen==0){
                                    EscreverEsquerda("Modo Fullscreen", 300, 350, fonte2);
                                }else{
                                    EscreverEsquerda("Modo Janela", 300, 350, fonte2);
                                }
                                if(mostrarFps==0){
                                    EscreverEsquerda("Mostrar FPS", 300, 300, fonte2);
                                }else{
                                    EscreverEsquerda("Esconder FPS", 300, 300, fonte2);
                                }
                                EscreverEsquerda("Voltar", 300, 200, fonte2);
                                break;
                            case 2:
                                EscreverEsquerda(vmusica, 300, 450, fonte2);
                                EscreverEsquerda(vjogo, 300, 400, fonte2);
                                if(fullscreen==0){
                                    EscreverEsquerda("Modo Fullscreen", 300, 350, fonte3);
                                }else{
                                    EscreverEsquerda("Modo Janela", 300, 350, fonte3);
                                }
                                if(mostrarFps==0){
                                    EscreverEsquerda("Mostrar FPS", 300, 300, fonte2);
                                }else{
                                    EscreverEsquerda("Esconder FPS", 300, 300, fonte2);
                                }
                                EscreverEsquerda("Voltar", 300, 200, fonte2);
                                break;
                            case 3:
                                EscreverEsquerda(vmusica, 300, 450, fonte2);
                                EscreverEsquerda(vjogo, 300, 400, fonte2);
                                if(fullscreen==0){
                                    EscreverEsquerda("Modo Fullscreen", 300, 350, fonte2);
                                }else{
                                    EscreverEsquerda("Modo Janela", 300, 350, fonte2);
                                }
                                if(mostrarFps==0){
                                    EscreverEsquerda("Mostrar FPS", 300, 300, fonte3);
                                }else{
                                    EscreverEsquerda("Esconder FPS", 300, 300, fonte3);
                                }
                                EscreverEsquerda("Voltar", 300, 200, fonte2);
                                break;
                            case 4:
                                EscreverEsquerda(vmusica, 300, 450, fonte2);
                                EscreverEsquerda(vjogo, 300, 400, fonte2);
                                if(fullscreen==0){
                                    EscreverEsquerda("Modo Fullscreen", 300, 350, fonte2);
                                }else{
                                    EscreverEsquerda("Modo Janela", 300, 350, fonte2);
                                }
                                if(mostrarFps==0){
                                    EscreverEsquerda("Mostrar FPS", 300, 300, fonte2);
                                }else{
                                    EscreverEsquerda("Esconder FPS", 300, 300, fonte2);
                                }
                                EscreverEsquerda("Voltar", 300, 200, fonte3);
                                break;
                            default:
                            break;
                        }

                    }
                }

                if(isMultiplayer && acabou){
                    char msg[50];
                    sprintf(msg, "%s venceu!", vencedor);
                    DesenhaRetangulo(150, 230, 200, 580, BRANCO, 0);
                    EscreverEsquerda(msg, 230, 300, fonte11);
                    EscreverEsquerda("Voltar", 400, 250, fonte6);
                }

                if(objPrincipal.vivo==0){
                    if(!isMultiplayer){
                        DesenhaRetangulo(200, 230, 200, 480, BRANCO, 0);
                        EscreverEsquerda("MORREU!", 230, 300, fonte8);
                        EscreverEsquerda("Voltar", 400, 250, fonte6);
                    }else{
                        if(auxAviso>0)
                        fazerAviso("Você morreu!", auxAviso, fonte5);
                    }
                }
                break;
            case 5:
                DesenhaRetangulo(0,0, 800, 1200, CINZA);
                EscreverCentralizada("Escolha a Classe", 350, 550, fonte4);

                if(timer<=0){
                    SetAnguloObjeto(objDemonstracao[0], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[1], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[2], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[3], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[4], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[5], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[6], GetAnguloObjeto(objDemonstracao[0])+1);
                    SetAnguloObjeto(objDemonstracao[7], GetAnguloObjeto(objDemonstracao[0])+1);
                }

                DesenhaObjeto(objDemonstracao[0]);
                DesenhaObjeto(objDemonstracao[1]);
                DesenhaObjeto(objDemonstracao[2]);
                DesenhaObjeto(objDemonstracao[3]);
                DesenhaObjeto(objDemonstracao[4]);
                DesenhaObjeto(objDemonstracao[5]);
                DesenhaObjeto(objDemonstracao[6]);
                DesenhaObjeto(objDemonstracao[7]);

                switch(escolhaInicial){
                    case 0:
                        EscreverEsquerda("Voltar", 30, 50, fonte6);

                        EscreverEsquerda("Speed", 50, 350, fonte5);
                        EscreverLongaEsquerda("A velocidade é o principal, porém atirar não é tão rápido.", 50, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 50, 150, fonte5);

                        EscreverEsquerda("Tank", 250, 350, fonte5);
                        EscreverLongaEsquerda("A vida é o principal, porém andar não é tão rápido.", 250, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 250, 150, fonte5);

                        EscreverEsquerda("Knife", 450, 350, fonte5);
                        EscreverLongaEsquerda("O dano é o principal, porém atirar não é tão rápido.", 450, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 450, 150, fonte5);

                        EscreverEsquerda("Reload", 650, 350, fonte5);
                        EscreverLongaEsquerda("O tempo de recarga é o principal, porém dano não é tão bom.", 650, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 650, 150, fonte5);

                        break;
                    case 1:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("Speed", 50, 350, fonte5);
                        EscreverLongaEsquerda("A velocidade é o principal, porém atirar não é tão rápido.", 50, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 50, 150, fonte6);

                        EscreverEsquerda("Tank", 250, 350, fonte5);
                        EscreverLongaEsquerda("A vida é o principal, porém andar não é tão rápido.", 250, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 250, 150, fonte5);

                        EscreverEsquerda("Knife", 450, 350, fonte5);
                        EscreverLongaEsquerda("O dano é o principal, porém atirar não é tão rápido.", 450, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 450, 150, fonte5);

                        EscreverEsquerda("Reload", 650, 350, fonte5);
                        EscreverLongaEsquerda("O tempo de recarga é o principal, porém dano não é tão bom.", 650, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 650, 150, fonte5);

                        break;
                    case 2:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("Speed", 50, 350, fonte5);
                        EscreverLongaEsquerda("A velocidade é o principal, porém atirar não é tão rápido.", 50, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 50, 150, fonte5);

                        EscreverEsquerda("Tank", 250, 350, fonte5);
                        EscreverLongaEsquerda("A vida é o principal, porém andar não é tão rápido.", 250, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 250, 150, fonte6);

                        EscreverEsquerda("Knife", 450, 350, fonte5);
                        EscreverLongaEsquerda("O dano é o principal, porém atirar não é tão rápido.", 450, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 450, 150, fonte5);

                        EscreverEsquerda("Reload", 650, 350, fonte5);
                        EscreverLongaEsquerda("O tempo de recarga é o principal, porém dano não é tão bom.", 650, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 650, 150, fonte5);

                        break;
                    case 3:
                       EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("Speed", 50, 350, fonte5);
                        EscreverLongaEsquerda("A velocidade é o principal, porém atirar não é tão rápido.", 50, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 50, 150, fonte5);

                        EscreverEsquerda("Tank", 250, 350, fonte5);
                        EscreverLongaEsquerda("A vida é o principal, porém andar não é tão rápido.", 250, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 250, 150, fonte5);

                        EscreverEsquerda("Knife", 450, 350, fonte5);
                        EscreverLongaEsquerda("O dano é o principal, porém atirar não é tão rápido.", 450, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 450, 150, fonte6);

                        EscreverEsquerda("Reload", 650, 350, fonte5);
                        EscreverLongaEsquerda("O tempo de recarga é o principal, porém dano não é tão bom.", 650, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 650, 150, fonte5);

                        break;
                    case 4:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("Speed", 50, 350, fonte5);
                        EscreverLongaEsquerda("A velocidade é o principal, porém atirar não é tão rápido.", 50, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 50, 150, fonte5);

                        EscreverEsquerda("Tank", 250, 350, fonte5);
                        EscreverLongaEsquerda("A vida é o principal, porém andar não é tão rápido.", 250, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 250, 150, fonte5);

                        EscreverEsquerda("Knife", 450, 350, fonte5);
                        EscreverLongaEsquerda("O dano é o principal, porém atirar não é tão rápido.", 450, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 450, 150, fonte5);

                        EscreverEsquerda("Reload", 650, 350, fonte5);
                        EscreverLongaEsquerda("O tempo de recarga é o principal, porém dano não é tão bom.", 650, 300, 170, 20, fonte5);
                        EscreverEsquerda("Escolher!", 650, 150, fonte6);

                        break;

                    default: break;
                }
                break;
            case 6:
                DesenhaRetangulo(0,0, 800, 1200, CINZA);
                EscreverCentralizada("Escolha a Dificuldade", 365, 550, fonte4);

                switch(escolhaInicial){
                    case 0:
                        EscreverEsquerda("Voltar", 30, 50, fonte6);

                        EscreverEsquerda("I", 160, 350, fonte7);
                        EscreverEsquerda("FÁCIL", 150, 300, fonte5);
                        DesenhaRetanguloVazado(140, 290, 140, 70, PRETO);

                        EscreverEsquerda("II", 350, 350, fonte7);
                        EscreverEsquerda("MÉDIO", 350, 300, fonte5);
                        DesenhaRetanguloVazado(340, 290, 140, 80, PRETO);

                        EscreverEsquerda("III", 540, 350, fonte7);
                        EscreverEsquerda("DÍFICIL", 550, 300, fonte5);
                        DesenhaRetanguloVazado(530, 290, 140, 105, PRETO);

                        break;
                    case 1:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("I", 160, 350, fonte7);
                        EscreverEsquerda("FÁCIL", 150, 300, fonte6);
                        DesenhaRetanguloVazado(140, 290, 140, 70, PRETO);

                        EscreverEsquerda("II", 350, 350, fonte7);
                        EscreverEsquerda("MÉDIO", 350, 300, fonte5);
                        DesenhaRetanguloVazado(340, 290, 140, 80, PRETO);

                        EscreverEsquerda("III", 540, 350, fonte7);
                        EscreverEsquerda("DÍFICIL", 550, 300, fonte5);
                        DesenhaRetanguloVazado(530, 290, 140, 105, PRETO);

                        break;
                    case 2:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("I", 160, 350, fonte7);
                        EscreverEsquerda("FÁCIL", 150, 300, fonte5);
                        DesenhaRetanguloVazado(140, 290, 140, 70, PRETO);

                        EscreverEsquerda("II", 350, 350, fonte7);
                        EscreverEsquerda("MÉDIO", 350, 300, fonte6);
                        DesenhaRetanguloVazado(340, 290, 140, 80, PRETO);

                        EscreverEsquerda("III", 540, 350, fonte7);
                        EscreverEsquerda("DÍFICIL", 550, 300, fonte5);
                        DesenhaRetanguloVazado(530, 290, 140, 105, PRETO);

                        break;
                    case 3:
                        EscreverEsquerda("Voltar", 30, 50, fonte5);

                        EscreverEsquerda("I", 160, 350, fonte7);
                        EscreverEsquerda("FÁCIL", 150, 300, fonte5);
                        DesenhaRetanguloVazado(140, 290, 140, 70, PRETO);

                        EscreverEsquerda("II", 350, 350, fonte7);
                        EscreverEsquerda("MÉDIO", 350, 300, fonte5);
                        DesenhaRetanguloVazado(340, 290, 140, 80, PRETO);

                        EscreverEsquerda("III", 540, 350, fonte7);
                        EscreverEsquerda("DÍFICIL", 550, 300, fonte6);
                        DesenhaRetanguloVazado(530, 290, 140, 105, PRETO);

                        break;
                    default:
                        break;
                    }
                    break;
            case 7:
                DesenhaRetangulo(0,0, 800, 1200, CINZA);
                EscreverCentralizada("Carregando o jogo", 365, 550, fonte4);

                if(auxCarregando<=0){
                    auxCarregando=0.2;
                    carregando++;
                }
                if(carregando>=20){
                    tela=4;
                    carregando=0;
                }

                EscreverEsquerda("Carregando...", 300, 300, fonte5);
                DesenhaRetangulo(150, 190, 70, 500, BRANCO, 0);
                DesenhaRetangulo(150, 200, 50, carregando*25, VERDE, 0);

            break;

            case 8:
                DesenhaRetangulo(0,0, 800, 1200, CINZA);
                EscreverCentralizada("Ajuda", 365, 550, fonte4);

                EscreverLongaEsquerda("Nesse jogo o objetivo é eliminar os inimigos para conseguir passar de fase. O jogo tem progressão infinita!", 50, 500, 300, 20, fonte5);
                EscreverLongaEsquerda("No jogo há um minimapa que indica sua posição e onde há caixas de vida. No modo difícil os inimigos não são mostrados.", 50, 400, 300, 20, fonte5);
                EscreverLongaEsquerda("Inimigos tem 20 balas assim como o jogador, quando essa munição acabar ele irá fugir de você! Um quadrado azul indicará isso!", 50, 250, 300, 20, fonte5);
                EscreverLongaEsquerda("No multiplayer é estilo battle royale, o último jogador a permanecer vivo ganha, não há como recuperar vida e você pode ser um espectador ao morrer!", 400, 500, 300, 20, fonte5);
                EscreverLongaEsquerda("O jogo é salvo toda vez que você passa de fase, ao clicar em 'Carregar Jogo' o jogo carregará a última coisa salva!", 400, 350, 300, 20, fonte5);
                EscreverLongaEsquerda("Qualquer problema no jogo, ou dúvida, envie um email para: poxtoc@gmail.com", 400, 100, 300, 20, fonte5);

                EscreverEsquerda("Voltar", 30, 50, fonte6);

            break;

            default: tela=0;
            break;

            }

            if(mostrarFps==1){
                char fps[10];
                sprintf(fps,"%.2f", GetFPS());
                EscreverEsquerda(fps,690, 570, fonte2);
            }

            EncerraDesenho();
        }
    if(isMultiplayer)
        DestroiSocketCliente(socket);

    free(input);
    destruirInimigos(&inimigos);
    fclose(arquivoSave);
    fclose(arquivoSaveNome);
    printf("Finalizando jogo...");

    FinalizaJogo();
    return 0;
}
