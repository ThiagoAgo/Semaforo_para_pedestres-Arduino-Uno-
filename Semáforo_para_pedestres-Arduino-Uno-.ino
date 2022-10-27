// C++ code
//
#define Sbit(N)    (1<<N)
#define Rstbit(N) ~(1<<N)
volatile byte Pedido_de_passagem = 0;
volatile byte contador = 0; // CONTA NUMERO DE INTERUPCOES DE 2,5 segundos

enum Estd {Verde, Amarelo, Vermelho, Vermelho_Fim} semafaro = Verde; 

void configuraPinos(){
  	// pinos com saidas e iniciam em zero
	DDRC |= (Sbit(DDC0)|Sbit(DDC1)|Sbit(DDC2)|Sbit(DDC3)|Sbit(DDC4));
    PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC1)|Sbit(PORTC2)|Sbit(PORTC3)|Sbit(PORTC4));
  	// pino 2 como entrada pull-up
  	MCUCR &= Rstbit(PUD); 
  	PORTD |= Sbit(PORTD2);
  	DDRD &= Rstbit(DDD2);  	
}

void configurar_ISR(){
  // HABILITA A INTERUPCAO INT0
  EIMSK |= Sbit(INT0);
  //INTERUPCAO NO PINO2 PARA BORDA DE DESCIDA
  EICRA |= Sbit(ISC01); 
  EICRA &= Rstbit(ISC00);
}

ISR(INT0_vect){
  Pedido_de_passagem = 1;
  contador = 0;
  TCNT1 = 0; // zerar a contagem do timer 1
}

void configurarTimer1(){
  // configurar o timer com CTC
	TCCR1A &= ~(Sbit(WGM11)|Sbit(WGM10));
  	TCCR1B |= Sbit(WGM12);
  	TCCR1B &= ~(Sbit(WGM13));
  // escolhe prescaler como 1024
  	TCCR1B |= (Sbit(CS12)|Sbit(CS10));
  	TCCR1B &= ~(Sbit(CS11));
  	// Configurar  valor maximo da contagem
  	OCR1A=39062;  
  	TCNT1 = 0; // zerar a contagem
  	TIMSK1 |= Sbit(OCIE1A); // habilita a interupcao do timer 1 
	//resutado final gera interucao a cada 2,5 segundos
}

ISR(TIMER1_COMPA_vect){
  contador++;
}

void LuzesSemafaro(){
    switch (semafaro){
      	case (Verde):
    		PORTC |= (Sbit(PORTC2)|Sbit(PORTC3));
    		PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC1)|Sbit(PORTC4));
      	if(contador == 4 &&   Pedido_de_passagem == 1){
      		contador = 0;
            Pedido_de_passagem = 0;
          	semafaro = Amarelo;}
    	break;
      
  		case (Amarelo):
    		PORTC |= (Sbit(PORTC1)|Sbit(PORTC3));
    		PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC2)|Sbit(PORTC4));
      	if (contador == 2) {
        	contador = 0;
          	semafaro = Vermelho;}
      	break;
      
  		case (Vermelho):
    		PORTC |= (Sbit(PORTC0)|Sbit(PORTC4));
    		PORTC &= ~(Sbit(PORTC1)|Sbit(PORTC2)|Sbit(PORTC3));
      	if (contador == 12) {
        	contador =0;
          	semafaro = Vermelho_Fim;}
      	break;
      	
      	case (Vermelho_Fim):
      		PORTC |= (Sbit(PORTC0)|Sbit(PORTC4));
    		PORTC &= ~(Sbit(PORTC1)|Sbit(PORTC2));
      		PORTC ^= (1<<PORTC3);
      	if (contador ==4){
      		contador =0;
          	semafaro = Verde;
        }
      	break;

    	default :
    	semafaro = Verde;
    }
} 

void setup(){
  	configuraPinos();
  	configurar_ISR();
  	configurarTimer1();	
}

void loop(){
  LuzesSemafaro();
  delay(500); 
}