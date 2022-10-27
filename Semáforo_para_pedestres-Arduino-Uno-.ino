// C++ code
//
#define Sbit(N)    (1<<N)
#define Rstbit(N) ~(1<<N)
volatile byte Pedido_de_passagem = 0;
volatile byte contador = 0; // CONTA NUMERO DE INTERUPCOES DE 1 segundo
volatile byte tempoVerde = 1;
volatile byte tempoAmarelo = 1;
volatile byte tempoVermelho = 1;

// definindo os estados
enum {INI, LAMPADA, TEMPO1, TEMPO2,FIM} estMQ;
enum Estd {Verde, Amarelo, Vermelho} semafaro = Verde; 

void configurarTempo(byte T1, byte T2 , byte c){
    switch(c){
      case 1:
          // Subtraimdo o valor de zero da tabela ASCII para obter o valor numerico referente ao char
          tempoVerde = ((T2-48)+(10*(T1-48)));  
          break;
        case 2:
          tempoAmarelo = ((T2-48)+(10*(T1-48)));    
          break;
        case 3:
          tempoVermelho = ((T2-48)+(10*(T1-48)));   
          break;
    }
}

// armazenamento temporario do valor e esperar 
char valorT1, valorT2 =0;
byte cor = 0; // verde = 1, amarelo = 2 e vermelho = 3
void processaMQ(char m){
  switch(estMQ){
    case INI:
      if(m=='I'){
      estMQ = LAMPADA;
      }break;
    
    case LAMPADA: // estado para Esperar mensagem iniciar
      switch(m){
          case 'A':
              estMQ = TEMPO1;
            cor = 1;
              break;
            case 'B':
              estMQ = TEMPO1;
            cor = 2;
              break;
      case 'C':
              estMQ = TEMPO1;
            cor = 3;
              break;
            default:
              Serial.println("Byte de Cor nao conhecido as cores sao:");
              Serial.println("Verde = A, Amarelo = B e Vermelho = C");
                estMQ = INI;
              break;
        }
    break;
    
    case TEMPO1:  // tratar a mensagem e verificar se e valida
    if(m>='0' && m<='9'){
      valorT1=m;
      estMQ=TEMPO2;
    }
    else{
      estMQ=INI;
      Serial.println("Tempo1 invalido escreva um valor de 0 a 9");
    }
    break;
    
    case TEMPO2:  // tratar a mensagem e verificar se e valida
    if(m>='0' && m<='9'){
      valorT2=m;
      estMQ=FIM;
    }
    else{
      estMQ=INI;
      Serial.println("Tempo2 invalido escreva um valor de 1 a 9");
    }
    break;
    
    case FIM:
    if(m=='F'){ // efetivamente executar o processamento
     configurarTempo(valorT1, valorT2, cor);
     Serial.println("Os dados foram salvos");}
    else{
    Serial.println("BYTE de filalizaçao incorreto, nada foi alterado.");
    }
    estMQ=INI;
    break;
  }
}


void LuzesSemafaro(){
    switch (semafaro){
        case (Verde):
        PORTC |= (Sbit(PORTC2));
        PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC1));
        if(contador >= tempoVerde &&   Pedido_de_passagem == 1){
          contador = 0;
            Pedido_de_passagem = 0;
            semafaro = Amarelo;}
      break;
      
      case (Amarelo):
        PORTC |= (Sbit(PORTC1));
        PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC2));
        if (contador >= tempoAmarelo) {
          contador = 0;
            semafaro = Vermelho;}
        break;
      
      case (Vermelho):
        PORTC |= (Sbit(PORTC0));
        PORTC &= ~(Sbit(PORTC1)|Sbit(PORTC2));
        if (contador >= tempoVermelho) {
          contador =0;
            semafaro = Verde;}
        break;
        
      default :
      semafaro = Verde;
    }
} 

void serialEvent(){
  char ch;
  while(Serial.available()){
    ch=Serial.read(); 
    processaMQ(ch);
  }
}

void configuraPinos(){
    // pinos com saidas e iniciam em zero
  DDRC |= (Sbit(DDC0)|Sbit(DDC1)|Sbit(DDC2));
    PORTC &= ~(Sbit(PORTC0)|Sbit(PORTC1)|Sbit(PORTC2));
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
    OCR1A=15625;  
    TCNT1 = 0; // zerar a contagem
    TIMSK1 |= Sbit(OCIE1A); // habilita a interupcao do timer 1 
  //resutado final gera interucao a cada 1 segundo
}

ISR(TIMER1_COMPA_vect){
  contador++;
}

void setup(){
    Serial.begin(9600);
    configuraPinos();
    configurar_ISR();
    configurarTimer1(); 
}

void loop(){
  LuzesSemafaro();
  delay(500); 
}