#include "DFRobot_BNO055.h"
#include "DFRobot_BMP280.h"
#include "Wire.h"
#include <AntaresESP8266HTTP.h>     
#define ACCESSKEY "b2572534c45e97d1:6fe7c16148c14e13"      
#define WIFISSID "AF-618"         
#define PASSWORD "semesteran"     
#define projectName "Deteksi_Gelombang_Laut"   
#define deviceName "deteksi_laut01"

AntaresESP8266HTTP antares(ACCESSKEY);

typedef DFRobot_BNO055_IIC    BNO;
typedef DFRobot_BMP280_IIC BMP;

BNO   bno(&Wire, 0x28);    // input TwoWire interface dan IIC address
BMP bmp(&Wire, BMP::eSdo_low);

void printLastOperateStatus(BNO::eStatus_t eStatus)
{
  switch(eStatus) {
  case BNO::eStatusOK:    Serial.println("status oke"); break;
  case BNO::eStatusErr:   Serial.println("eror !!!"); break;
  case BNO::eStatusErrDeviceNotDetect:    Serial.println("Alat tidak terdeteksi"); break;
  case BNO::eStatusErrDeviceReadyTimeOut: Serial.println("Alat time out"); break;
  case BNO::eStatusErrDeviceStatus:       Serial.println("status internal eror"); break;
  default: Serial.println("status tidak diketahui"); break;
  }
}

void printLastOperateStatus(BMP::eStatus_t eStatus)
{
  switch(eStatus) {
  case BMP::eStatusOK:    Serial.println("everything ok"); break;
  case BMP::eStatusErr:   Serial.println("unknow error"); break;
  case BMP::eStatusErrDeviceNotDetected:    Serial.println("device not detected"); break;
  case BMP::eStatusErrParameter:    Serial.println("parameter error"); break;
  default: Serial.println("unknow status"); break;
  }
}

void setup()
{
  Serial.begin(115200);
  bno.reset();
  bmp.reset();
  antares.setDebug(true);   // Nyalakan debug. Set menjadi "false" jika tidak ingin pesan-pesan tampil di serial monitor
  antares.wifiConnection(WIFISSID,PASSWORD);  // Mencoba untuk menyambungkan ke WiFi
  while(bno.begin() != BNO::eStatusOK) {
    Serial.println("alat gagal dijalankan");
    printLastOperateStatus(bno.lastOperateStatus);
    delay(2000);
  }
  while(bmp.begin() != BMP::eStatusOK) {
    Serial.println("bmp begin faild");
    printLastOperateStatus(bmp.lastOperateStatus);
    delay(2000);
  }
  Serial.println("bno berhasil dijalankan");
  Serial.println("bmp begin success");
}

int i;

void loop()
{
  BNO::sEulAnalog_t   sEul;
  sEul = bno.getEul();
  BNO::sAxisAnalog_t readGyr, readAcc, readMag; 
  readGyr = bno.getAxis(BNO::eAxisGyr);
  readAcc = bno.getAxis(BNO::eAxisLia);
  readMag = bno.getAxis(BNO::eAxisMag);
  float   temp = bmp.getTemperature();
  double A,W,Y,V,kompas,temper;

  /*A : akselerasi
   *W : kecepatan sudut
   *Y : tinggi gelombang
   *V : kecepatan gelombang*/
   
  A = (sqrt(pow(readAcc.x,2)+pow(readAcc.y,2)+pow(readAcc.z,2)))/16; // dibagi 16 untuk mendapatkan nilai yang stabil
  W = (sqrt(pow(readGyr.x,2)+pow(readGyr.y,2)+pow(readGyr.z,2)) * 0.0175); //dikali 0.0175 untuk mengubah dari deg/sec ke rad/sec 
  Y = A / (pow(W,2));
  V = W * Y;
  temper = temp - 5;

  //kompas
  kompas =atan2(readMag.x,readMag.y);
  if(kompas < 0){
    kompas += 2*PI;
  }
  //mengubah radian ke derajat
  kompas = kompas * 180/M_PI;
  
  Serial.print("i: ");
  Serial.println(i);
  Serial.print("temperatur: "); 
  Serial.println(temper);
  Serial.print("Percepatan : ");
  Serial.println(A);
  Serial.print("Kecepatan sudut : ");
  Serial.println(W);
  Serial.print("Tinggi : ");
  Serial.println(Y);
  Serial.print("Kecepatan : ");
  Serial.println(V);
  Serial.print("Kompas : ");
  Serial.println(kompas);
  Serial.print("pitch:");
  Serial.print(sEul.pitch, 3);
  Serial.print(" ");
  Serial.print("roll:");
  Serial.print(sEul.roll, 3);
  Serial.print(" ");
  Serial.print("yaw:");
  Serial.print(sEul.head, 3);
  Serial.println(" ");

  i++;
  if(i==300){
    antares.add("Percepatan",A);
    antares.add("Kecepatan sudut",W);
    antares.add("Tinggi",Y);
    antares.add("Kecepatan",V);
    antares.add("Temperatur",temper);
    antares.add("Kompas",kompas);
    antares.send(projectName, deviceName);
    i=0;
  }
  delay(100);
}
