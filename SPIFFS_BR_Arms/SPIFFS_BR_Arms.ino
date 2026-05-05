// AJUSTADO PARA ESP32 + SPIFFS (mesma lógica de operação)
// SPIFFS significa SPI Flash File System

// sistema de arquivos que roda dentro da memória Flash do ESP32
// Capacidade: de ~800 - centenas de milhares de posições
// Sem limite fixo de EEPROM
// Mesmo comportamento de gravação/replay

// Pontos técnicos importantes
// SPIFFS grava em Flash - ainda tem desgaste (mas muito mais tolerante)
// Leitura é sequencial - mantém a lógica original
// Arquivo cresce continuamente - pode apagar ao iniciar nova gravação (já tratado)

// Antigo - EEPROM limitada e frágil
// Atual  - armazenamento massivo com mesma lógica operacional

// Comparações
// EEPROM - como um array bruto
// SPIFFS - como um sistema de arquivos (tipo HD)

#include <Servo.h>
#include "SPIFFS.h"

Servo servo1;//base
Servo servo2;//cotovelo
Servo servo3;//mão
Servo servo4;//punho
Servo servo5;//garra

byte memoria[105];
byte memor[5];
double direcao[5], registro[5], diferenca[5];
byte paso[5];

byte pasoN=0,i=0,secao=1,limite=0;
size_t eprom=0; // agora é offset no arquivo
byte EE=0;

long tempoc=0;
long tempob=0;

unsigned long tempotranscorrido=millis();
unsigned long tempomicros=micros();

byte time=1000;

#define retardo 4000
#define FILE_PATH "/mov.bin"

boolean automata=false, primeira_memoria=true, primeiro_paso=true;
boolean garraAberta=true, primeira_secao=true, ultima_memoria=true;

File file; // handler global para leitura

// ================= SETUP =================
void setup()
{
    SPIFFS.begin(true);

    for(i=5;i<8;i++) pinMode(i,INPUT);

    i=0;
    pinMode(13, OUTPUT);

    servo1.attach(10);
    servo2.attach(11);
    servo3.attach(9);
    servo4.attach(3);
    servo5.attach(12);

    Serial.begin(115200);

    // calcula quantos passos já existem
    File f = SPIFFS.open(FILE_PATH, FILE_READ);
    if(f) {
        pasoN = f.size() / 5;
        f.close();
    } else {
        pasoN = 0;
    }
}

// ================= LOOP =================
void loop()
{
    tempotranscorrido=millis();
    tempomicros=micros();
    botao();

    if(!automata)
    {
        mover();
    }
    else
    {
        if(primeiro_paso)
        {
            leitura();

            if(ultima_memoria) ultimo_registro();

            eprom = 0;
            file = SPIFFS.open(FILE_PATH, FILE_READ);

            primeiro_paso=false;
        }
        else if(pasoN <= 0 || !file || !file.available())
        {
            // reinicia leitura
            if(file) file.close();
            file = SPIFFS.open(FILE_PATH, FILE_READ);
            eprom = 0;
        }

        if(primeira_secao) matriz_de_traducao();

        if(tempomicros-tempob > time)
        {
            tempob=tempomicros;
            execute_automata();
        }
    }
}

// ================= BOTÃO =================
void botao()
{
  if((digitalRead(6))==LOW)
  {
    delay(20);
    if((digitalRead(6))==HIGH)
    {
      if(i==0)
      {
        i=1;
        tempoc=tempotranscorrido;
      }
      else if((i==1)&&((tempotranscorrido-tempoc)<500))
      {
        i=2;
      }
    }
  }

  if((i==1)&&((tempotranscorrido-tempoc)>1000))
  {
    pasoN++;

    if(primeira_memoria)
    {
      pasoN=0;
      SPIFFS.remove(FILE_PATH); // limpa arquivo
    }

    memoriza();

    primeira_memoria=false;
    i=0;
  }
  else if(i==2)
  {
    automata=true;
    Serial.println("\nEXECUTA AUTOMATO");
    i=0;
  }
}

// ================= MOVIMENTO =================
void mover()
{
  leitura();
  servo1.write(paso[0]);
  servo2.write(paso[1]);
  servo3.write(paso[2]);
  servo4.write(paso[3]);
  servo5.write(paso[4]);
}

// ================= EXECUÇÃO =================
void execute_automata()
{
  if(secao<=limite)
  {
    secao++;

    registro[0]+=direcao[0];
    registro[1]+=direcao[1];
    registro[2]+=direcao[2];
    registro[3]+=direcao[3];
    registro[4]+=direcao[4];

    servo1.write(registro[0]);
    servo2.write(registro[1]);
    servo3.write(registro[2]);
    servo4.write(registro[3]);
    servo5.write(registro[4]);

    primeira_secao=false;
  }
  else
  {
    limite=0;
    secao=0;
    primeira_secao=true;
  }
}

// ================= MATRIZ =================
void matriz_de_traducao()
{
  for(EE=0;EE<5;EE++)
  {
    if(file.available())
      memor[EE] = file.read();
  }
  EE=0;

  diferenca[0] = abs(registro[0]-memor[0]);
  diferenca[1] = abs(registro[1]-memor[1]);
  diferenca[2] = abs(registro[2]-memor[2]);
  diferenca[3] = abs(registro[3]-memor[3]);
  diferenca[4] = abs(registro[4]-memor[4]);

  limite = max(diferenca[0],diferenca[1]);
  limite = max(limite,diferenca[2]);
  limite = max(limite,diferenca[3]);
  limite = max(limite,diferenca[4]);

  for(int k=0;k<5;k++)
  {
    if(registro[k]>memor[k])
      direcao[k] = (0-diferenca[k])/limite;
    else
      direcao[k] = diferenca[k]/limite;
  }
}

// ================= MEMÓRIA =================
void memoriza()
{
  leitura();

  File f = SPIFFS.open(FILE_PATH, FILE_APPEND);

  for(int EE=0;EE<5;EE++)
  {
    f.write(paso[EE]);
  }

  f.close();

  digitalWrite(13,HIGH);
  delay(100);
  digitalWrite(13,LOW);
}

// ================= AUX =================
void ultimo_registro()
{
  for(int k=0;k<5;k++)
    registro[k] = paso[k];

  ultima_memoria=false;
}

void leitura()
{
  paso[0]=map(analogRead(A0),1023,0,180,0);
  paso[1]=map(analogRead(A1),1023,0,180,0);
  paso[2]=map(analogRead(A2),1023,0,0,180);
  paso[3]=map(analogRead(A3),1023,0,0,180);

  if(digitalRead(5)==HIGH) paso[4]=90;
  else paso[4]=130;

  for(int k=0;k<5;k++)
    paso[k]=constrain(paso[k],0,180);
}