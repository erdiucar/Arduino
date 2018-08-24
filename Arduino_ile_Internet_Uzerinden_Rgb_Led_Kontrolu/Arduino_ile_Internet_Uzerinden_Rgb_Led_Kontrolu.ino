// Kullanılacak kütüphaneler çağrılıyor
#include <SPI.h>
#include <Ethernet.h>

// Kırmızı, yeşil ve mavi renk için sabit oluşturuyorum
const int kirmizi = 6;
const int yesil = 5;
const int mavi = 3;

// Led' in son durumunu bu değişkende tutuyorum. Eğer kapalı değilse son gelen renk hex formatında atanıyor (#000000 gibi)
String sonRenk = "Kapalı";

// Ethernet kalkanının, mac ve ip adresi giriliyor. Başka adreslerle çakışmadığı sürece istenilen girilebilir
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

// Ip adresi elle belirleniyor. istenirse otomatik atama da yapılabilir
IPAddress ip(192,168,1,13);

// HTTP için 8090 portunu açtığım için onu yazıyorum
EthernetServer server(8090);

// Burada gerekli ön hazırlıklar yapılıyor.
void setup()
{
  // Bilgisayarla seri iletişim başlatılıyor. Saniyede 9600 bit gönderimi ayarlanıyor
  Serial.begin(9600);

  // Çıkış olarak 3,5,6 numaralı pinler belirleniyor 
  pinMode(kirmizi, OUTPUT);
  pinMode(yesil, OUTPUT);
  pinMode(mavi, OUTPUT);
  
  // Led başlangıçta kapatılıyor. Rgb led anot olduğu için HIGH yapınca kapanıyor.
  digitalWrite(kirmizi, HIGH);
  digitalWrite(yesil, HIGH);
  digitalWrite(mavi, HIGH);

  // Ethernet belirlenen ip'de çalıştırılıyor
  Ethernet.begin (mac, ip);

  // Sunucu başlatılıyor
  server.begin();
}

// Döngü başlatılıyor
void loop()
{
  // İstemcinin bağlanmasını bekleniyor
  EthernetClient client = server.available();

  // Eğer istemci bağlandıysa bu bölüm çalışıyor
  if (client)
  {
    String gelen = "";

    // İstemci bağlı olduğu sürece bu döngü çalışıyor
    while (client.connected())
    {
      if (client.available())
      {
        // Arduino'ya bağlanan bilgisayardan gelen mesajın karakteri okunup c değişkenine atanıyor
        char c = client.read();
        
        // Seri monitörde gelen karakterleri sırayla yazdırıyorum
        Serial.print(c);

        // İstemciden gelen karakterler "gelen" isimli stringe ekleniyor
        gelen += c;
        
        // Eğer boş bir satırdan sonra tekrar boş bir satıra denk gelinirse HTTP isteği sonlanmış demektir
        if (c == '\n')
        {
          // İstemciden gelen mesaj "Kapat" ise led kapanıyor ve sonRenk değişkeni "Kapalı" oluyor.
          if (gelen == "Kapat\n")
          {
            client.print(sonRenk);
            
            sonRenk = ledKapa();
            
            //Serial.println(sonRenk);
          }
          else
          {
            // Renk aynı mı diye kontrol etmek amaçlı bir değişken oluşturuyorum
            String kontrol = renkAyarla(gelen);

            // Eğer renk aynıysa sonRenk aynı kalıyor ve istemciye "Renk aynı" diye mesaj gönderiliyor
            if (kontrol == "Renk aynı")
            {
              client.print("Renk aynı");
              //Serial.println(sonRenk);
              //Serial.println("Renk aynı");
            }

            // Değilse sonRenk'e yeni gelen renk atanıyor ve istemciye en son gönderdiği renk gönderiliyor
            else
            {
              sonRenk = kontrol;
              client.print(sonRenk);
              //Serial.println(sonRenk);
            }
          }
          
          break;
        }
      }
    }

    // Web tarayıcıya verileri alması için 1 ms zaman veriliyor
    delay(1); 

    // İstemciyle bağlantı sonlandırılıyor
    client.stop(); 
  }
}

// Bu metodla rgb led'in renkleri ayarlanıyor
void setColor(int red, int green, int blue)
{
  analogWrite(kirmizi, red);
  analogWrite(yesil, green);
  analogWrite(mavi, blue);
}

// Bu metodla rgb led kapanıyor ve "Kapalı" mesajı döndürüyor
String ledKapa()
{
  digitalWrite(kirmizi, HIGH);
  digitalWrite(yesil, HIGH);
  digitalWrite(mavi, HIGH);

  return "Kapalı";
}

// Rgb led anot olduğu için renk düzenlemesi yapmak gerekiyor
int renkDuzenle (int renk)
{
    return (255 - renk);
}

// Bu metodla gelen mesaj parçalara ayrılıyor. İstemciye gönderilecek renk belirleniyor
String renkAyarla(String s)
{
  String red = "";
  String green = "";
  String blue = "";
  String gonderilecekRenk = "";
  byte sayac = 1;

  // Gelen mesajın tüm karakterleri kontrol ediliyor. '+' karakteri görülürse sayaç arttırılarak diğer renge atama yapılıyor. '\n' ise döngü sonlandırılıyor.
  for(int i = 0, r = s.length(); i < r; i++)
  {
     if (s[i] == '+')
     {
      sayac++;
      continue;
     }
     else if (s[i] == '\n')
     {
      break;
     }
     else
     {
       if (sayac == 1)
       {
          red += s[i];
       }
       else if (sayac == 2)
       {
          green += s[i];
       }
       else if (sayac == 3)
       {
          blue += s[i];
       }
       else
       {
          gonderilecekRenk += s[i];
       }
     }
  }

  // sonRenk değişkeniyle gelen renk aynı ise, "Renk aynı" mesajı döndürülüyor
  if (gonderilecekRenk == sonRenk)
  {
    return "Renk aynı";
  }

  // Renk aynı değilse yeni renk ayarlanıyor ve bu yeni renk döndürülüyor
  else
  {
    setColor(renkDuzenle(red.toInt()), renkDuzenle(green.toInt()), renkDuzenle(blue.toInt()));
    return gonderilecekRenk;
  }
}
