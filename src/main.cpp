#include <Arduino.h>
#include <Servo.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

Servo myservo;

// Модуль DS1302: DAT = Pin 6, CLK = Pin 7, RST = Pin 5
ThreeWire myWire(6, 7, 5);
RtcDS1302<ThreeWire> Rtc(myWire);

int last_second = -1;
int last_angle = 0; // Початковий кут 0

// Функція для ПЛАВНОГО повороту без стрибків струму
void moveServoSmooth(int fromAngle, int toAngle)
{
  if (fromAngle == toAngle)
    return;

  myservo.attach(9);

  if (fromAngle < toAngle)
  {
    for (int pos = fromAngle; pos <= toAngle; pos++)
    {
      myservo.write(pos);
      delay(8); // регулює швидкість: чим більше число, тим плавніше
    }
  }
  else
  {
    for (int pos = fromAngle; pos >= toAngle; pos--)
    {
      myservo.write(pos);
      delay(8);
    }
  }

  delay(100);
  myservo.detach(); // Вимикаємо утримання
}

void setup()
{
  Serial.begin(9600);

  // Старт у положенні 0°
  myservo.attach(9);
  myservo.write(0);
  delay(300);
  myservo.detach();

  Rtc.Begin();
  Rtc.SetIsWriteProtected(false);

  if (!Rtc.GetIsRunning())
  {
    Rtc.SetIsRunning(true);
  }

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled + 8);
}

void loop()
{
  RtcDateTime now = Rtc.GetDateTime();

  if (!now.IsValid())
  {
    Serial.println("RTC Read Error!");
    delay(1000);
    return;
  }

  int current_second = now.Second();

  if (current_second != last_second)
  {
    last_second = current_second;

    int target_angle = 0;
    if (current_second < 15)
      target_angle = 0;
    else if (current_second < 30)
      target_angle = 45;
    else if (current_second < 45)
      target_angle = 90;
    else
      target_angle = 135;

    Serial.print("Time: ");
    if (now.Hour() < 10)
      Serial.print('0');
    Serial.print(now.Hour());
    Serial.print(':');
    if (now.Minute() < 10)
      Serial.print('0');
    Serial.print(now.Minute());
    Serial.print(':');
    if (current_second < 10)
      Serial.print('0');
    Serial.print(current_second);

    Serial.print(" | Servo Angle: ");
    Serial.print(target_angle);
    Serial.println(" deg");

    // Плавний поворот серводвигуна
    if (target_angle != last_angle)
    {
      moveServoSmooth(last_angle, target_angle);
      last_angle = target_angle;
    }
  }

  delay(50);
}