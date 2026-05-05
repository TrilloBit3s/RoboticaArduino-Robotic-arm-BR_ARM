// CÓDIGO FUNCIONANDO 100% 02/04/2018 
// 250 bytes / 5 bytes por passo ≈ 50 passos (máximo teórico)
// | Placa            | EEPROM usada | Passos                |
// | ---------------- | ------------ | --------------------- |
// | Uno              | ~1 KB        | ~200 passos (teórico) |
// | Seu código atual | ~250 bytes   | ~50 passos            |
// | ESP32 (4 KB)     | 4096 bytes   | ~800 passos           |

#include <Servo.h>
#include <EEPROM.h>

Servo servo1;//base
Servo servo2;//cotovelo
Servo servo3;//mão
Servo servo4;//punho
Servo servo5;//garra

/*arranjos só podem memorizar 20 posições com esta configuração 
* (no arduino UNO) o uso da EEPROM limita a capacidade do braço 
* de aprender 20 posições * 5 servos = memórias*/

byte memoria[105];
byte memor[5];
double direcao[5], registro[5], diferenca[5];
byte paso[5];
byte pasoN=0,i=0,secao=1,limite=0;
byte eprom,EE=0;
long tempoc=0;
long tempob=0;
unsigned long tempotranscorrido=millis();
unsigned long tempomicros=micros();
byte time=1000;
#define retardo 4000
boolean automata=false, primeira_memoria=true, primeiro_paso=true;
boolean garraAberta=true, primeira_secao=true, ultima_memoria=true;

    void setup()
    {
        for(i=5;i<8;i++)
        {
            pinMode(i,INPUT);
        }
       
        i=0; 
        pinMode(13, OUTPUT);
        
        servo1.attach(10);//base
        servo2.attach(11);//ombro
        servo3.attach(9);//braço
        servo4.attach(3);//punho
        servo5.attach(12);//garra
        
        Serial.begin(115200);
        Serial.print("\t\t | ");
        
        for(i=0;i<3;i++)
        {
            Serial.print("###");
            //delay(25); //esse delay faz se arrebente no chão, não é bom utilizar a menos que você melhore algo
        }

        i=0;
        Serial.print(" | 100% 0.75s");
        Serial.print("\t\t  GRAVA NA EEPROM ");
    }

    void loop()
    {
        tempotranscorrido=millis();
        tempomicros=micros();
        botao();
        
        if(!automata)
        {
            mover();
        }
        else if(automata)
        {        
            if(primeiro_paso)
            {
                leitura();
                
                if(ultima_memoria)ultimo_registro();
                pasoN=0;
                eprom=1;
                primeiro_paso=false;
            }
            else if(pasoN>EEPROM.read(250))
            {
                pasoN=0;
                eprom=1;
            }

            if(primeira_secao)matriz_de_traducao();
            if(tempomicros-tempob>time)
            {
                tempob=tempomicros;
                execute_automata();
            }
        }
    }

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
      eprom=1;
    }
    memoriza();
    EEPROM.write(250,pasoN);
    primeira_memoria=false;
    i=0;
  }
  else if(i==2)
  {
    automata=true;
    Serial.print("\n\t EXECUTA AUTOMATO ");
    i=0;
  }
}

void mover()
{
  leitura();
  servo1.write(paso[0]);
  servo2.write(paso[1]);
  servo3.write(paso[2]);
  servo4.write(paso[3]);
  servo5.write(paso[4]);
}

void execute_automata()
{
  if(secao<=limite)
  {
    Serial.print("\n\t EXECUTA MEMORIA ");
    Serial.print(pasoN);
    Serial.print("\n\t SEÇÃO=");
    Serial.print(secao);
    secao++;
    if(secao == 1) time = retardo*4;       
    else if(secao == 10) time = retardo*3;   
    else if(secao == 20) time = retardo*2; 
    else if(secao == 30) time = retardo*1;
    if(secao == limite-40) time = retardo*2;
    else if(secao == limite-30) time = retardo*3;
    else if(secao == limite-20) time = retardo*4;
    else if(secao == limite-10) time = retardo*5; 
    
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
    pasoN++;
    primeira_secao=true;
  }
  while (digitalRead(7)==true)
  {
    servo1.write(90);
    servo2.write(10);
    servo3.write(140);
    servo4.write(90);
    servo5.write(90);//alcanse da garra 90-130
    digitalWrite(13, HIGH); delay(500); 
    digitalWrite(13, LOW); delay(500);
  }
}

void matriz_de_traducao()
{
  Serial.print("\n\t CALCULO MATRIZ");
  
  for(EE=0;EE<5;EE++)
  {
    memor[EE]=EEPROM.read(eprom);
    eprom++;
  }
  EE=0;
  
  diferenca[0] = abs (registro[0]-memor[0]);
  diferenca[1] = abs (registro[1]-memor[1]);
  diferenca[2] = abs (registro[2]-memor[2]);
  diferenca[3] = abs (registro[3]-memor[3]);
  diferenca[4] = abs (registro[4]-memor[4]);
  
  if(diferenca[0]<=5)diferenca[0]=0;
  if(diferenca[1]<=5)diferenca[1]=0;
  if(diferenca[2]<=5)diferenca[2]=0;
  if(diferenca[3]<=5)diferenca[3]=0;
  if(diferenca[4]<=5)diferenca[4]=0;
  
  limite = max(diferenca[0],diferenca[1]);
  limite = max(limite,diferenca[2]);
  limite = max(limite,diferenca[3]);
  limite = max(limite,diferenca[4]);
  Serial.print("\n\t\tLIMITE=");
  Serial.print(limite);
  
  if(registro[0]>memor[0]) direcao[0]=(0-diferenca[0])/limite; else
 direcao[0]=diferenca[0]/limite;
  if(registro[1]>memor[1]) direcao[1]=(0-diferenca[1])/limite; else
 direcao[1]=diferenca[1]/limite;
  if(registro[2]>memor[2]) direcao[2]=(0-diferenca[2])/limite; else
 direcao[2]=diferenca[2]/limite;
  if(registro[3]>memor[3]) direcao[3]=(0-diferenca[3])/limite; else
 direcao[3]=diferenca[3]/limite;
  if(registro[4]>memor[4]) direcao[4]=(0-diferenca[4])/limite; else 
 direcao[4]=diferenca[4]/limite;
}

void ultimo_registro()
{
  registro[0] = paso[0];
  registro[1] = paso[1];
  registro[2] = paso[2];
  registro[3] = paso[3];
  registro[4] = paso[4];
  ultima_memoria=false;
}

void memoriza()
{
  leitura();
  
  for(EE=0;EE<5;EE++)
  {
    EEPROM.write(eprom,paso[EE]);
    eprom++;
  }
  EE=0;
  Serial.print("\n\t MEMORIA\t\t"); 
  Serial.print(pasoN);
  digitalWrite(13,HIGH);
  delay(100);//100
  digitalWrite(13,LOW);
}
void leitura()
{
  paso[0]=map(analogRead(A0),1023,0,180,0);//base  10 pin
  paso[1]=map(analogRead(A1),1023,0,180,0);//ombro 11 pin 
  paso[2]=map(analogRead(A2),1023,0,0,180);//braço  9 pin
  paso[3]=map(analogRead(A3),1023,0,0,180);//punho 3 pin
  if((digitalRead(5))==HIGH)paso[4]=90;else paso[4]=130;
  

  paso[0]=constrain(paso[0],0,180);
  paso[1]=constrain(paso[1],0,180);
  paso[2]=constrain(paso[2],0,180);
  paso[3]=constrain(paso[3],0,180);
  paso[4]=constrain(paso[4],0,180);
}