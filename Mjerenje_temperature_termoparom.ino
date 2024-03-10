/* Mjerenje temperature termoparom
 Autor : Maurin Rastovac */

#include <max6675.h>                           //library za mjerenje temperature termoparom pomoću MAX6675 čipa
#include <LiquidCrystal.h>                  //library za upravljanje sa LCD-om
#include <avr/io.h>                              //avr library za input/output registre
#include <avr/interrupt.h>                  //avr library za interupte
#include <avr/sleep.h>                       //library za postavljenja atmela čipa u sleep mode

MAX6675 thermocouple(16, 15, 14); //MAX6675 thermocouple(thermoCLK, thermoCS, thermoSO) //zadajemo na kojem će pinovima biti MAX6675
LiquidCrystal lcd(11, 10, 9, 8, 7, 6);  //LiquidCrystal lcd(rs, en, d4, d5, d6, d7) //zadajemo na kojem će pinovima biti LCD

void init_ports(void);                            //postavimo izlaze i ulaze na portovima
void init_interrupt(void);                     //postavljamo kakve interupte želim i uključujemo ih
void init_timer(void);                          //definiramo tajmer i pokrećemo ga
void init_lcd(void);                             //uključujemo LCD
void turn_off(void);                           //funkcija za isključenje sistema

int t,n=0,i=0; //t-varijabla koja gleda jeli gumb 2 bio pritisnut dok smo u hold funkciji , n-varijabla da kad uđemo u funkciju hold ne postavlja ponovo kod if-a, i-brojač za tajmer
float C,F,K; //varijable za spremanje temperature kad uđemo u hold funkciju
volatile int gumb_1=0, gumb_2=0;              //brojač za gumbove
char degree[8] = {B01100,                           //Napravimo ° simbol
                             B10010,
                             B10010,
                             B01100,
                             B00000,
                             B00000,
                             B00000,
                             B00000
};

ISR (INT0_vect){                            //interupt 0 funkcija 
    gumb_1++;                                 //povećavamo varijablu za 1
}

ISR (INT1_vect){                             //interupt 0 funkcija 
    gumb_2++;                                  //povećavamo varijablu za 1
    TCNT1=3036;                            //resetiramo tajmer da počne brojati ispočetka
    i=0;                                            //resetiramo varijablu natrag na 0
}

ISR(TIMER1_OVF_vect){                  // tajmer 1 funkcija koji se aktivira svakih 4 sekunde
    i++;                                                  //povećavamo varijablu za 1
    TCNT1=3036;                                //resetiramo tajmer tako da broji točno 4 sekunde
  }

void setup(){
  init_ports();                                      //postavimo izlaze i ulaze na portovima
  init_interrupt();                               //postavljamo kakve interupte želim i uključujemo ih
  init_timer();                                    //definiramo tajmer i pokrećemo ga
  init_lcd();                                       //uključujemo LCD
  ADCSRA=0;                                 //isključimo ADC da uštedimo struju
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); //postavljamo koji način rada želimo za funkciju sleep_mode();
  sleep_enable();                                       //uključujemo mogućnost sleep funkciju
  delay(500);                                            //pričekamo malo da se sve stabilizira
}

void loop(){
  lcd.clear();                                 //izbrišemo sve na LCD-u
  lcd.setCursor(0, 0);                  //postavimo pokazivač na početak LCD-a (početak 1. reda)
  lcd.print("Temperatura:");      //ispišemo Temperatura:
  lcd.setCursor(0,1);                 //postavimo pokazivač na početak 2. reda na LCD-u
  if(gumb_2>=3)                      //funkcija koja resetira brojač gumb_2 kad dođe do 3
    gumb_2=0;                         //resetiramo varijablu na 0
   switch(gumb_2){               //switch funkcija koja gleda kolika je vrijednost varijable gumb_2 trenutačno
     case 0:lcd.print(thermocouple.readCelsius()); //ako je varijabla 0 pokreće se ovaj dio programa i ispisuje koliku temperaturu termopar trenutno ocitava u celsiusima
            lcd.write(byte(0));                     //napišemo simbol koji smo na početku programa napravili a to je °
            lcd.print("C ");                        //napišemo C na LCD-u
            break;                                     //break funkcija za svaki slučaj
     case 1:lcd.print(thermocouple.readFahrenheit()); //ako je varijabla 1 pokreće se ovaj dio programa i ispisuje koliku temperaturu termopar trenutno očitava u farenhajtu
            lcd.write(byte(0));                                      //napišemo simbol koji smo na početku programa napravili a to je °
            lcd.print("F ");                                         //napišemo F na LCD-u
            break;                                                      //break funkcija za svaki slučaj
     case 2:lcd.print((thermocouple.readCelsius()+273.15)); //ako je varijabla 2 pokreće se ovaj dio programa i ispisuje koliku temperaturu termopar trenutno očitava u kelvinima
            lcd.write(byte(0));                               //napišemo simbol koji smo na početku programa napravili a to je °
            lcd.print("K ");                                  //napisemo K na LCD-u
            break;                                                //break funkcija za svaki slučaj
      }
  if(thermocouple.readCelsius()==0.00){   //ako termopar očitava 0°C onda se pokreće ova funkcija
    lcd.setCursor(13, 1);                              //idemo na kraj LCD-a 
    lcd.print("Min");                                   //ispisujemo Min na LCD-u kao oznaka da termopar očitava najmanju vrijednos koju može (zapravo MAX6675 ne može nize)
    }
  if(thermocouple.readCelsius()==1023.75){ //ako termopar očitava 1023.75°C onda se pokreće ova funkcija
    lcd.setCursor(13, 1);                             //idemo na kraj LCD-a
    lcd.print("Max");                                 //ispisujemo Max na LCD-u kao oznaka da termopar očitava najveću vrijednos koju može (zapravo MAX6675 ne može vise)
    }
  while(gumb_1!=0){ //kad se stisne gumb 1 idemo u hold funkciju, to jest display prestane ažurirati temperaturu
    if(n==0){      //funkcija koja se samo na početku programa pokreće
      t=gumb_2; //spremimo kolika je varijabla gumba 2 bila kad smo ušli u while funkciju
      C=thermocouple.readCelsius();                                 //spremimo kolika je temperatura bila kad smo ušli u while funkciju (u celsiusima)
      F=thermocouple.readFahrenheit();                           //spremimo kolika je temperatura bila kad smo ušli u while funkciju (u farenhaitima)
      K=(thermocouple.readCelsius()+273.15);                //spremimo kolika je temperatura bila kad smo ušli u while funkciju (u kelvinima)
      TCNT1=3036;        //resetiramo tajmer da počne brojati ispočetka
      i=0;                        //resetiramo varijablu natrag na 0
      n++;                      //povećamo varijablu za 1 tako da se if funkcija ne pokreće ponovo
      }
     if(t!=gumb_2){
      if(gumb_2>=3)
      gumb_2=0;
      lcd.clear();                             //izbrišemo sve na LCD-u
      lcd.setCursor(0, 0);               //postavimo pokazivač na početak LCD-a(početak 1. reda)
      lcd.print("Temperatura:");    //ispišemo Temperatura:
      lcd.setCursor(0,1);               //postavimo pokazivač na početak 2. reda na LCD-u
      switch(gumb_2){
       case 0:lcd.print(C);
              lcd.write(byte(0));                      //napišemo simbol koji smo na početku programa napravili a to je °
              lcd.print("C");                            //napišemo C na LCD-u
              break;                                         //break funkcija za svaki slučaj
       case 1:lcd.print(F);
              lcd.write(byte(0));                     //napišemo simbol koji smo na početku programa napravili a to je °
              lcd.print("F");                           //napišemo F na LCD-u
              break;                                       //break funkcija za svaki slučaj
       case 2:lcd.print(K);
              lcd.write(byte(0));                   //napišemo simbol koji smo na početku programa napravili a to je °
              lcd.print("K");                        //napišemo K na LCD-u
              break;                                     //break funkcija za svaki slučaj
        }
    t=gumb_2;                                       //ažuriramo vrijednosti varijable
    }
    lcd.setCursor(15, 0);  //idemo na kraj 1. reda
    lcd.print("H"); //ispisujemo slovo H kao indikator da trenutačno držimo istu temperaturu
    if(gumb_1>=2){ //kad ponovo stisnemo gumb 1 pokreće se funkcija
      gumb_1=0;   //postavljamo vrijednost varijable na 0 i while funkcija se ugasi
      n=0;              //postavljamo vrijednost varijable na 0
      }
  if(i==450)     //nakon određenog vremena pokreće se funkcija za isključenje (dok smo u hold-u)
    turn_off();
    }
  if(i==450)    //nakon određenog vremena pokreće se funkcija za isključenje (najvjerojatnije ću postaviti nakon 30 minuta u slučaju da se zaboravi ugasiti uređaj)
    turn_off(); //pozivamo turn_off funkciju
  delay(700); //pričekamo 400ms pa se ponovo ponavlja loop (minimalno 220ms zbog MAX6675)
  }

void init_ports(void){                                      //postavimo izlaze i ulaze na portovima
  DDRD&=(~(1<<DDD3)) | (~(1<<DDD2)); //postavimo da su PD3 i PD2 ulazi
  DDRD^=(1<<DDD5);                                  //postavimo da je PD5 izlaz
  DDRB^=(1<<DDB4);                                  //postavimo da je PB4 izlaz
  PORTD|=(1<<PORTD3) | (1<<PORTD2);   //uključujemo internal pull up otpornik na PD3 i PD2
  PORTD|=(1<<PORTD5);               //puštamo 5V na PD5 da se LCD uključi
  PORTB|=(1<<PORTB4);               //puštamo 5V na PB4 da se svijetlo na LCD-u uključi
  }
void init_interrupt(void){                    //postavljamo kakve interupte želim i uključujemo ih
  sei();                                                  // uključimo interrupte
  EICRA|=(1<<ISC01) | (1<<ISC11);//ISC01 i ISC11 postavljamo u 1 tako da se interupt događa na padajući brid
  EIMSK|=(1<<INT0) | (1<<INT1);  //uključujemo interupte INT0 i INT1
  }

void init_timer(void){                                //definiramo tajmer i pokrećemo ga
  TCCR1A=0;                                           
  TCCR1B=0;
  TCNT1=3036;                                         // tajmer kreće od 3036 brojati
  TCCR1B|=(1<<CS12) | (1<<CS10);      //stavljamo CS12 i CS10 u 1 tako da imamo prescaler od 1024
  TIMSK1|=(1<<TOIE1);                         //uključujemo tajmer 1
  
  }
void init_lcd(void){                             //uključujemo LCD
  lcd.begin(16, 2);                               //LCD se pokreće
  lcd.createChar(0, degree);               //napravimo ° simbol
  }

 void turn_off(void){                           //funkcija za isključenje sistema
    lcd.noDisplay ();                             //ugasimo LCD
    PORTB^=(1<<PORTB4);             //isključimo PB4 to jest svijetlo na LCD-u
    PORTD^=(1<<PORTD5);            //isključimo PD5 to jest LCD
    noInterrupts ();                              //isključujemo interupte
    sleep_mode();                               //idemo u sleep
    i=0;                                               //resetiramo varijablu
  }
