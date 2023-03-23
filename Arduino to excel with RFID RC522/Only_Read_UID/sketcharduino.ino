
//Bibliotecas
#include <SPI.h>
#include <MFRC522.h>

#include "Wire.h"
#include <LiquidCrystal.h>

//Pinos
#define LED_VERDE 14
#define LED_VERMELHO 15
#define BUZZER 8
#define SS_PIN 53
#define RST_PIN 22

#define DS1307_ADDRESS 0x68

String IDtag = ""; //Variável que armazenará o ID da Tag
bool Permitido = false; //Variável que verifica a permissão 

//Vetor responsável por armazenar os ID's das Tag's cadastradas
String TagsCadastradas[] = {"E79ABD89", 
                            "A1C53948",
                            "3F80BD89"};
                            
MFRC522 LeitorRFID(SS_PIN, RST_PIN);    // Cria uma nova instância para o leitor e passa os pinos como parâmetro


int readsuccess;
byte readcard[3];
char str[32] = "";

// Inicializa o LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte zero = 0x00; 

void setup() {
        Serial.begin(9600);             // Inicializa a comunicação Serial
        SPI.begin();                    // Inicializa comunicacao SPI 
        LeitorRFID.PCD_Init();          // Inicializa o leitor RFID
        pinMode(LED_VERDE, OUTPUT);     // Declara o pino do led verde como saída
        pinMode(LED_VERMELHO, OUTPUT);  // Declara o pino do led vermelho como saída
        pinMode(BUZZER, OUTPUT);        // Declara o pino do buzzer como saída
                        

        Serial.println("CLEARDATA");
        Serial.println("LABEL,Data,Hora,ID Tag");
        delay(1000);
        
        Serial.println("Scan PICC to see UID...");
        Serial.println("");


        // Define o LCD com 20 colunas e 4 linhas
        lcd.begin(20, 4);
        Wire.begin();
        Serial.begin(9600);
        // A linha abaixo pode ser retirada apos setar a data e hora
        //SelecionaDataeHora();         
}

void loop() {  
  Leitura();  //Chama a função responsável por fazer a leitura das Tag's

 Mostrarelogio();
 delay(1000);  
}


void Leitura(){
        IDtag = ""; //Inicialmente IDtag deve estar vazia.
        
        // Verifica se existe uma Tag presente
        if ( !LeitorRFID.PICC_IsNewCardPresent() || !LeitorRFID.PICC_ReadCardSerial() ) {
            delay(50);
            return;
        }
        
        // Pega o ID da Tag através da função LeitorRFID.uid e Armazena o ID na variável IDtag        
        for (byte i = 0; i < LeitorRFID.uid.size; i++) {        
            IDtag.concat(String(LeitorRFID.uid.uidByte[i], HEX));
        }        
        
        //Compara o valor do ID lido com os IDs armazenados no vetor TagsCadastradas[]
        for (int i = 0; i < (sizeof(TagsCadastradas)/sizeof(String)); i++) {
          if(  IDtag.equalsIgnoreCase(TagsCadastradas[i])  ){
              Permitido = true; //Variável Permitido assume valor verdadeiro caso o ID Lido esteja cadastrado
          }
        }       
        if(Permitido == true) acessoLiberado(); //Se a variável Permitido for verdadeira será chamada a função acessoLiberado()        
        else acessoNegado(); //Se não será chamada a função acessoNegado()
        delay(2000); //aguarda 2 segundos para efetuar uma nova leitura    
         
        
      
}

void acessoLiberado(){
  Serial.println("Tag Cadastrada: " + IDtag); //Exibe a mensagem "Tag Cadastrada" e o ID da tag não cadastrada
    efeitoPermitido();  //Chama a função efeitoPermitido()
    Permitido = false;  //Seta a variável Permitido como false novamente

    getid();
      Serial.println( (String) "DATA,DATE,TIME," + IDtag );

    lcd.clear();
    lcd.setCursor(0,1);          
    lcd.print("  Ponto registrado");
    delay(2000); 
      lcd.clear();
              
  }

void acessoNegado(){
  Serial.println("Tag NAO Cadastrada: " + IDtag); //Exibe a mensagem "Tag NAO Cadastrada" e o ID da tag cadastrada
  efeitoNegado(); //Chama a função efeitoNegado()

  lcd.clear();
  lcd.setCursor(0,1);          
  lcd.print("  Erro de leitura");
  delay(2000); 
    lcd.clear();  
}

void efeitoPermitido(){  
  int qtd_bips = 2; //definindo a quantidade de bips
  for(int j=0; j<qtd_bips; j++){
    //Ligando o buzzer com uma frequência de 1500 hz e ligando o led verde.
    tone(BUZZER,1500);
    digitalWrite(LED_VERDE, HIGH);   
    delay(100);   
    
    //Desligando o buzzer e led verde.      
    noTone(BUZZER);
    digitalWrite(LED_VERDE, LOW);
    delay(100);
  }  
}

void efeitoNegado(){  
  int qtd_bips = 1;  //definindo a quantidade de bips
  for(int j=0; j<qtd_bips; j++){   
    //Ligando o buzzer com uma frequência de 500 hz e ligando o led vermelho.
    tone(BUZZER,500);
    digitalWrite(LED_VERMELHO, HIGH);   
    delay(500); 
      
    //Desligando o buzzer e o led vermelho.
    noTone(BUZZER);
    digitalWrite(LED_VERMELHO, LOW);
    delay(500);
  }  
}


void array_to_string(byte array[], unsigned int len, char buffer[]){
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 3) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

int getid(){  
  if(!LeitorRFID.PICC_IsNewCardPresent()){
    return 0;
  }
  if(!LeitorRFID.PICC_ReadCardSerial()){
    return 0;
  }
  
  Serial.println("THE UID OF THE SCANNED CARD IS:");
  
  for(int i=0;i<3;i++){
    readcard[i]=LeitorRFID.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 3, str);
    IDtag = str;
  }
  LeitorRFID.PICC_HaltA();
  return 1;
}


void SelecionaDataeHora() // Seta a data e a hora do DS1307
{
 byte segundos = 1; // Valores de 0 a 59
 byte minutos = 42; // Valores de 0 a 59
 byte horas = 99; // Valores de 0 a 23
 byte diadasemana = 0; // Valores de 0 a 6 (0=Domingo, 1 = Segunda...)
 byte diadomes = 19; // Valores de 1 a 31
 byte mes = 3; // Valores de 1 a 12
 byte ano = 23; // Valores de 0 a 99
 Wire.beginTransmission(DS1307_ADDRESS);
 // Stop no CI para que o mesmo possa receber os dados
 Wire.write(zero); 
 
 // As linhas abaixo escrevem no CI os valores de 
 // data e hora que foram colocados nas variaveis acima
 Wire.write(ConverteParaBCD(segundos));
 Wire.write(ConverteParaBCD(minutos));
 Wire.write(ConverteParaBCD(horas));
 Wire.write(ConverteParaBCD(diadasemana));
 Wire.write(ConverteParaBCD(diadomes));
 Wire.write(ConverteParaBCD(mes));
 Wire.write(ConverteParaBCD(ano));
 Wire.write(zero); //Start no CI
 Wire.endTransmission(); 
}
 
byte ConverteParaBCD(byte val)
{ 
 // Converte o número de decimal para BCD
 return ( (val/10*16) + (val%10) );
}
 
byte ConverteparaDecimal(byte val) 
{ 
 // Converte de BCD para decimal
 return ( (val/16*10) + (val%16) );
}
 
void Mostrarelogio()
{
 Wire.beginTransmission(DS1307_ADDRESS);
 Wire.write(zero);
 Wire.endTransmission();
 Wire.requestFrom(DS1307_ADDRESS, 7);
 int segundos = ConverteparaDecimal(Wire.read());
 int minutos = ConverteparaDecimal(Wire.read());
 int horas = ConverteparaDecimal(Wire.read() & 0b111111); 
 int diadasemana = ConverteparaDecimal(Wire.read()); 
 int diadomes = ConverteparaDecimal(Wire.read());
 int mes = ConverteparaDecimal(Wire.read());
 int ano = ConverteparaDecimal(Wire.read());
 // Imprime mensagem na primeira linha do display
 lcd.setCursor(0,0);
 lcd.print("   JWA Tecnologia ");
  
 // Mostra a hora atual no display
 lcd.setCursor(6, 2);
 if (horas < 10)
 {lcd.print("0");}
 lcd.print(horas);
 lcd.print(":");
 if (minutos < 10)
 {lcd.print("0");}
 lcd.print(minutos);
 lcd.print(":");
 if (segundos < 10)
 {lcd.print("0");}
 lcd.print(segundos);
  
 // Mostra a data atual no display
 lcd.setCursor(0, 3);
 lcd.print("Data : ");
 lcd.setCursor(7,3);
 if (diadomes < 10)
 {lcd.print("0");}
 lcd.print(diadomes);
 lcd.print(".");
 if (mes < 10)
 {lcd.print("0");}
 lcd.print(mes);
 lcd.print(".");
 lcd.print(ano);
  
 //Mostra o dia da semana no display
 lcd.setCursor(17, 3);
 switch(diadasemana)
 {
 case 0:lcd.print("Dom");
 break;
 case 1:lcd.print("Seg");
 break;
 case 2:lcd.print("Ter");
 break;
 case 3:lcd.print("Qua");
 break;
 case 4:lcd.print("Qui");
 break;
 case 5:lcd.print("Sex");
 break;
 case 6:lcd.print("Sab");
 }
}




