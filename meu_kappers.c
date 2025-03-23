#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>
#include <math.h>

const int SCREEN_W = 960; //largura da tela
const int SCREEN_H = 540; //altura da tela
const float FPS = 100; //frames por segundo na tela
const int PISO_H = 30; //altura do piso dos andares

#define NUM_ANDARES 3
#define NUM_SALAS 3
#define COP_VEL 2
#define LADRAO_VEL 1
#define GRAVIDADE 0.1
#define JUMP_VEL 3
#define DESCONTO_VEL_LAMA 0.2
#define ALTURA_PULO 17

const int PESSOA_W = 30; //Largura do policial e do ladrão
const int PESSOA_H = 50; //Altura do policial e do ladrão

typedef struct Pessoa {
	float x, x_mall, y, yvel;
	int x_esq;
	int x_dir;
	float vel;
	int sala;
	int andar;
	int pulando;
	int puloLimite;
	int caindo;
	ALLEGRO_COLOR cor;
} Pessoa;

typedef struct Mud {
	int x, y, w;
} Mud;

//---------------Variaveis globais----------------
ALLEGRO_COLOR BKG_COLOR;
ALLEGRO_COLOR andar_cores[NUM_ANDARES]; // Armazena as cores dos andares
ALLEGRO_FONT *size_32;  	
int FLOOR_H;
int MALL_W;
int score;

void init_global_vars() {
	BKG_COLOR = al_map_rgb(15,15,15); //define a cor de fundo
	size_32 = al_load_font("arial.ttf", 32, 1);   	
	MALL_W = SCREEN_W * NUM_SALAS; //largura do shopping (é o tamanho da tela*o numero de salas)
	FLOOR_H = SCREEN_H / NUM_ANDARES; //altura dos andares (altura da tela/nº de andares)

	//gerar cores aleatórias para as salas
	int i;
	for (i=0; i < NUM_SALAS; i++){
		int r = rand()%256; //parte vermelha
		int g = rand()%256; //parte verde
		int b = rand()%256; //parte azul
		//armazenar em um vetor para utilizar sem que fica reiniciando dentro de um looping
		andar_cores[i] = al_map_rgb(r,g,b);
}
}

void init_Policial(Pessoa *cop){
	cop->cor = al_map_rgb(255,255,255);
	cop->x = PESSOA_W;
	cop->y = SCREEN_H - PISO_H;
	cop->x_mall = cop->x;
	cop->x_dir = 0;
	cop->x_esq = 0;
	cop->vel = COP_VEL;	
	cop->sala = 0;
	cop->andar = 0;
	cop->pulando = 0;
	cop->yvel = 0;
	cop->puloLimite = 0;
	cop->caindo = 0;
}

void init_Ladrao(Pessoa *ladrao){
	ladrao->cor = al_map_rgb(255,8,127);
	ladrao->sala = 1;
	ladrao->andar = 1;
	ladrao->x_mall = ladrao->sala * SCREEN_W + SCREEN_W - PESSOA_W;
	ladrao->x = (int)ladrao->x_mall % SCREEN_W;			//posição geometrica em x do ladrão
	ladrao->y = SCREEN_H - (ladrao->andar)*FLOOR_H - PISO_H;		//posição geometrica em y do ladrão
	ladrao->x_dir = 0;
	ladrao->x_esq = 1;
	ladrao->vel = LADRAO_VEL;	
	ladrao->pulando = 0;
	ladrao->yvel = 0;
	ladrao->puloLimite = 0;
	ladrao->caindo = 0;
}

void init_Lama (Mud (*lama)[NUM_ANDARES]){
	int i = 0;
	
	for (int i = 0; i < NUM_SALAS; i++) {
		for (int j = 0; j < NUM_ANDARES; j++) {

			lama[i][j].x = 50 + rand() % (SCREEN_W - 100); // Posição horizontal
			lama[i][j].y = SCREEN_H - (j)*(SCREEN_H/NUM_ANDARES);     // Posição vertical
			lama[i][j].w = 70 + rand()%150;  // Largura da poça

		}
	}
}

//-------------------------------------------------------------------
void desenha_cenario(Pessoa cop, Mud Lama[][NUM_ANDARES]){

    //limpa a tela e coloca cor de fundo
    al_clear_to_color(BKG_COLOR);

    int i, y_andar, j;
    for(i=0; i<NUM_ANDARES; i++) {
   	y_andar = SCREEN_H - (i*(SCREEN_H/NUM_ANDARES));
	al_draw_filled_rectangle(0, y_andar, SCREEN_W, y_andar - PISO_H, andar_cores[cop.sala]); 
   		
   }
   
 
	for(j=0; j<NUM_ANDARES; j++) {
		al_draw_filled_rectangle(Lama[cop.sala][j].x, Lama[cop.sala][j].y, Lama[cop.sala][j].x + Lama[cop.sala][j].w, Lama[cop.sala][j].y - PISO_H, al_map_rgb(141,73,37));
	}

}

void desenha_policial(Pessoa p) {
    
        if(p.x_dir == 0 && p.x_esq == 0) {
            al_draw_filled_triangle(p.x - PESSOA_W/2.0, p.y, 
                                   p.x + PESSOA_W/2.0, p.y,
                                   p.x, p.y - PESSOA_H,
                                   p.cor);
        }
        else {
            al_draw_filled_triangle(p.x, p.y, 
                                   p.x, p.y - PESSOA_W,
                                   p.x + PESSOA_H * pow(-1, 2*p.x_dir + p.x_esq), p.y - PESSOA_W/2.0,
                                   p.cor);
        }
    

    // Exibe o score
    char text_score[4];
    sprintf(text_score, "%d", score);
    al_draw_text(size_32, al_map_rgb(102,178,255), SCREEN_W - 70, 20, 0, text_score);  

}

void desenha_ladrao(Pessoa p, Pessoa cop) {
    //  Só desenha o ladrão se ele estiver na mesma sala do policial. 
	if (p.sala == cop.sala){
        if(p.x_dir == 0 && p.x_esq == 0) {
            al_draw_filled_triangle(p.x - PESSOA_W/2.0, p.y, 
                                   p.x + PESSOA_W/2.0, p.y,
                                   p.x, p.y - PESSOA_H,
                                   p.cor);
        }
        else {
            al_draw_filled_triangle(p.x, p.y, 
                                   p.x, p.y - PESSOA_W,
                                   p.x + PESSOA_H * pow(-1, 2*p.x_dir + p.x_esq), p.y - PESSOA_W/2.0,
                                   p.cor);
        }
	}

}

void elevador(Pessoa *p){
    if (p->andar % 2 == 1) {
        if (p->x_mall - PESSOA_H <= 0) { // Corrige o cálculo para a borda esquerda
            p->andar += 1;
            p->x_mall = 0 + PESSOA_W ; // Ajusta a posição no próximo andar
			p->x_esq = 0;
        } else if (p->x_mall + PESSOA_H >= MALL_W) {
            p->andar -= 1;
            p->x_mall = MALL_W - PESSOA_W; // Ajusta a posição no andar anterior
			p->x_dir = 0;
        }
    } else if (p->andar % 2 == 0) {
		if (p->andar == 0){
			if (p->x_mall + PESSOA_H >= MALL_W) { // Corrige o cálculo para a borda direita
				p->andar += 1;
				p->x_mall = MALL_W - PESSOA_W; // Ajusta a posição no próximo andar
				p->x_dir = 0;
		}
        }else if (p->x_mall + PESSOA_H >= MALL_W) { // Corrige o cálculo para a borda direita
            p->andar += 1;
			p->x_mall = MALL_W - PESSOA_W; // Ajusta a posição no próximo andar
			p->x_dir = 0;
        } else if (p->x_mall - PESSOA_H <= 0) { // Corrige o cálculo para a borda esquerda
            p->andar -= 1;
        	p->x_mall = 0 + PESSOA_W; // Ajusta a posição no andar anterior
			p->x_esq = 0;
        }
    }
	//a posição em x é a posição na tela visivel ela é o resto da divisão posição no shopping pela tamanho da tela.
	p->x = ((int)p->x_mall) % SCREEN_W;
	p->y = SCREEN_H - (p->andar)*FLOOR_H - PISO_H;
}

void obstaculo_lama (Pessoa *p, Mud obst[][NUM_ANDARES]){
    int em_lama = 0; // Flag para verificar se o personagem está na lama

    // criado uma matriz de lama, para pegar o endereço de memoria da lama que está na sala e andar no bonequinho
    Mud *lama = &obst[p->sala][p->andar]; 

    // Verifica se o personagem está na mesma sala e andar da lama, e só verifica a colisão se o personagem estiver com os pés no chão
	if (p->y >= SCREEN_H - (p->andar) * FLOOR_H - PISO_H) { 
		if (p->x > lama->x && p->x < lama->x + lama->w) {
			em_lama = 1; // Personagem está na lama
    	}
	}

    // Aplicar redução de velocidade se estiver na lama
    if (em_lama) {
        p->vel = COP_VEL * DESCONTO_VEL_LAMA; // Reduz velocidade
        p->pulando = 0; // Impede que pule
        //printf("Personagem na lama! Velocidade reduzida para %.2f\n", p->vel);
    } else {
        p->vel = COP_VEL; // Restaura a velocidade normal se não estiver na lama
    }
}

void update_pessoa (Pessoa *p, Mud lama[][NUM_ANDARES]){

	int y_base = SCREEN_H - (p->andar)*FLOOR_H - PISO_H;

	obstaculo_lama(p, lama);

	if (p->x_dir == 1){
		//quando a distancia percorrida no shopping + tamanho do pessoa for menor que a largura do shopping atualiza a pessoa
		if (p->x_mall + PESSOA_H < MALL_W){
			//quando a telca D estiver apertaad vai andar a posição no shoppig mais a velocidade para a direita
			if (p->andar == 0){
			p->x_mall += (p->vel);
			} 
			if (p->andar == 1){
			p->x_mall += (p->vel+2);
			}
			if (p->andar == 2){
				p->x_mall += (p->vel+4);
			}
		}else{
			elevador(p);
		}
	}

	if (p->x_esq == 1){
		if (p->x_mall - PESSOA_H > 0 ){
			//quando a tecla D estiver apertada vai andar a velocidade mais a posição no shopping para a esquerda
			if (p->andar == 0){
			p->x_mall -= (p->vel);
			} 
			if (p->andar == 1){
			p->x_mall -= (p->vel+2);
			}
			if (p->andar == 2){
				p->x_mall -= (p->vel+4);
			}
		}else{
			elevador(p);
		}
	}

	if (p->pulando == 1){
			if (p->y >= p->puloLimite - ALTURA_PULO){
				p->yvel = -JUMP_VEL; //para subir a velocidade do pulo precisa ser negativa e para descer positiva.
				p->y += p->yvel;}
			else{
				p->pulando = 0;
				p->caindo = 1;
			}
		}

	if (p->caindo == 1){
			p->yvel += GRAVIDADE; //aumenta a velocidade para descer
			p->y += (p->yvel); //descer
			// Verifica se o boneco atingiu o chão
			if (p->y >= y_base){
				p->yvel = 0;     //Reseta a velocidade vertical
				p->caindo = 0;  //para de cair
				p->y = y_base;
			}
	}

	//a posição em x é a posição na tela visivel ela é o resto da divisão posição no shopping pela tamanho da tela.
	p->x = ((int)p->x_mall) % SCREEN_W;
	//p->y = SCREEN_H - (p->andar)*FLOOR_H - PISO_H;
	//a sala onde o bonequinho ta é a largura do shopping dividido pelo tamanho da tela. 
	p->sala = p->x_mall / SCREEN_W;
}

void update_ladrao(Pessoa *p){

	if (p->x_dir == 1){
		//quando a distancia percorrida no shopping + tamanho do pessoa for menor que a largura do shopping atualiza a pessoa
		if (p->x_mall + PESSOA_H < MALL_W){
			//quando a telca D estiver apertaad vai andar a posição no shoppig mais a velocidade para a direita
			p->x_mall += p->vel;
		}else{
			elevador(p);
			p->x_dir = 0;
			p->x_esq = 1;
		}
	}

	if (p->x_esq == 1){
		if (p->x_mall - PESSOA_H > 0 ){
			//quando a tecla D estiver apertada vai andar a velocidade mais a posição no shopping para a esquerda
			p->x_mall -= p->vel;
		}else{
			elevador(p);
			p->x_esq = 0;
			p->x_dir = 1;
		}
	}

	p->x = ((int)p->x_mall) % SCREEN_W;
	//p->y = SCREEN_H - (p->andar)*FLOOR_H - PISO_H;
	//a sala onde o bonequinho ta é a largura do shopping dividido pelo tamanho da tela. 
	p->sala = p->x_mall / SCREEN_W;

}

int colisaoPolicialLadrao(Pessoa p, Pessoa l){
	// Só verifica colisão se estiverem na mesma sala e andar
    if (p.sala != l.sala || p.andar != l.andar) {
        return 1; 
    }

	// Calcula os limites dos avatores
    float pEsquerda = p.x;
    float pDireita = p.x + PESSOA_H;
    float pTopo = p.y - PESSOA_H;
    float pBase = p.y;

    float lEsquerda = l.x;
    float lDireita = l.x + PESSOA_H;
    float lTopo = l.y - PESSOA_W;
    float lBase = l.y;
	
	if (pEsquerda < lDireita && pDireita > lEsquerda && pTopo < lBase && pBase > lTopo){
		return 0;
	}
}

int ladraoVenceu(Pessoa ladrao) {
    // Verifica se o ladrão está na última sala (sala 2, índice 1) e no último andar (andar 2, índice 1)
    if (ladrao.sala == NUM_SALAS - 1 && ladrao.andar == NUM_ANDARES - 1) {
      
        if (ladrao.x_mall >= MALL_W - PESSOA_H) {
            return 1; // Ladrão venceu
        }
    }
    return 0; // Ladrão ainda não venceu
}

int main (){

    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
	int i, j;

    //inicializa o Allegro
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

 	// Inicializa addons necessários
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();

	//Inicializa as variaveis globais
    init_global_vars();
	Pessoa policial;
	Pessoa ladrao;
	init_Policial(&policial);
	init_Ladrao(&ladrao);
	//----------------- Obstacles -------------	
	Mud Lama[NUM_SALAS][NUM_ANDARES];
	init_Lama(Lama);

    //cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

    //cria uma tela usando a variavel criada com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

    //instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}

    //cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
	al_init_ttf_addon();


	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    //ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   

    //registra na fila os eventos de tela (ex: clicar no X da janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
    //registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source()); 	
    //registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

    //inicia o temporizador
	al_start_timer(timer);
    
    int playing = 1, ladrao_venceu = 0;
    while (playing) {

        ALLEGRO_EVENT ev;

        //espera por um evento e o armazena na variavel de evento ev
        al_wait_for_event(event_queue, &ev);

        //se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {
			
			//desenha_cenario(cop, obst);
			desenha_cenario(policial, Lama);
			update_pessoa(&policial, Lama);
			desenha_policial(policial);
			desenha_ladrao(ladrao, policial);
			update_ladrao(&ladrao);
			playing = colisaoPolicialLadrao(policial, ladrao);
			if (ladrao_venceu = ladraoVenceu(ladrao)) {
				playing = 0;
			}

			//reinicializo a tela
			al_flip_display();
			
			if(al_get_timer_count(timer)%(int)FPS == 0) {
				score += 1;
				//printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
			}
		}

        //se o tipo de evento for o fechamento da tela (clique no x da janela)
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            playing = 0;
        }
        //se o tipo de evento for APERTAR uma tecla
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN){
            //imprimir qual tecla foi
            printf("\ncodigo tecla: %d", ev.keyboard.keycode);
            switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_A:
				policial.x_esq = 1;
				break;

			case ALLEGRO_KEY_D:
				policial.x_dir = 1;
				break;

			case ALLEGRO_KEY_SPACE:
					if (policial.pulando == 0 && policial.caindo == 0){
						policial.puloLimite = policial.y;
						policial.pulando = 1;
					}
					break;

			}
            al_flip_display();
    }

        //se o tipo de evento for SOLTAR uma tecla
        else if (ev.type == ALLEGRO_EVENT_KEY_UP){
            //imprimir qual tecla foi
            //printf("\ncodigo tecla: %d", ev.keyboard.keycode);
            switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_A:
				policial.x_esq = 0;
				break;

			case ALLEGRO_KEY_D:
				policial.x_dir = 0;
				break;

			}
            al_flip_display();
    }

    }

	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
	al_rest(1);
	
	if(ladrao_venceu) {
		al_clear_to_color(al_map_rgb(0,0,0));
		ALLEGRO_BITMAP *ending = al_load_bitmap("ending2.png");
		//coloca na tela a imagem armazenada na variavel image nas posicoes x=50, y=100
		al_draw_bitmap(ending, 20, 0, 0);

		//printf("\nLadrao ganhou...");

		//atualiza a tela
		al_flip_display();

		//pausa a tela por 3.0 segundos
		al_rest(8.0);	
		al_destroy_bitmap(ending);		
	}
	else {
	
		char my_text[20];	
		char text_record[100], text_score[100];
		//colore toda a tela de azul
		al_clear_to_color(al_map_rgb(230,240,250));
		
		FILE *arq, *arq_aux;
		arq = fopen("recorde.txt", "r");
		int recorde = 0;
		fscanf(arq,"%d", &recorde);
		if (score < recorde){
			arq_aux = fopen("Auxiliar.txt", "w");
			fprintf(arq_aux, "%d", score);
			fclose(arq);
			remove("recorde.txt");
			fclose(arq_aux);
			rename("Auxiliar.txt", "recorde.txt");
			sprintf(my_text, "Você atingiu um novo recorde: %d pontos", score);
			al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/2, SCREEN_H/3, ALLEGRO_ALIGN_CENTER, "Fim de jogo!");
			al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/2, SCREEN_H/3 + 50, ALLEGRO_ALIGN_CENTER, "PARABÉNS!! VOCÊ É RÁPIDO.");
			al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/2, SCREEN_H/3 + 100, ALLEGRO_ALIGN_CENTER, my_text);
		}else{
		sprintf(text_score, "Pontuação: %d", score);
		sprintf(text_record, "Recorde atual: %d", recorde);
		sprintf(my_text, "VOCÊ CAPTUROU O LADRÃO!");
		al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/2, SCREEN_H/3 + 50, ALLEGRO_ALIGN_CENTER, my_text);
		al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/2 - 150, SCREEN_H/3 + 100, ALLEGRO_ALIGN_CENTER, text_score);
		al_draw_text(size_32, al_map_rgb(0, 128, 0), SCREEN_W/2 + 150, SCREEN_H/3 + 100, ALLEGRO_ALIGN_CENTER, text_record);
		fclose(arq);
		}

		//reinicializa a tela
		al_flip_display();	
	   	al_rest(3);	
	 }
	
    al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

    return 0;

}