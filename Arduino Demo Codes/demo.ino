#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define GREEN_LED 5  // Green LED (Success)
#define RED_LED 18   // Red LED (Failure)

bool inAdminMode = false;  // Flag to track admin mode

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Fingerprint Sensor Setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing Fingerprint Sensor...");

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  displayMessage("System Ready");
  delay(2000);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor detected!");
    displayMessage("Fingerprint OK!");
  } else {
    Serial.println("Fingerprint sensor NOT found!");
    displayMessage("Sensor Error!");
    while (1);
  }
}

void loop() {
  char key = keypad.getKey();  

  if (key == '#' && !inAdminMode) {
    handleAdminMode();
  } 

  if (!inAdminMode) {
    searchFingerprint();
  }
}

// Function to search for a fingerprint
void searchFingerprint() {
  Serial.println("\nPlace your finger to search...");
  displayMessage("Place Finger...");

  int p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) {
    return;
  }

  if (p != FINGERPRINT_OK) {
    Serial.println("Error capturing image");
    return;
  }

  Serial.println("Image captured");
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error processing image");
    return;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint found! ID: ");
    Serial.println(finger.fingerID);
    displayMessage("Access Granted");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    Serial.println("Fingerprint not found");
    displayMessage("Access Denied");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }

  delay(2000);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

// Function to handle admin access
void handleAdminMode() {
  inAdminMode = true;
  Serial.println("\nEnter Admin Password: ");
  displayMessage("Enter Admin PWD");

  String adminPass = getAdminPassword();

  if (adminPass == "123") {
    showAdminMenu();
  } else {
    Serial.println("Incorrect password. Resuming fingerprint search...");
    displayMessage("Wrong Password!");
    delay(2000);
    inAdminMode = false;
  }
}

// Function to get admin password from keypad
String getAdminPassword() {
  String password = "";
  char key;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("Enter PWD: ");
  display.display();

  while (true) {
    key = keypad.getKey();
    if (key && key >= '0' && key <= '9') {
      password += key;
      Serial.print("*");
      display.print("*");
      display.display();
    } else if (key == '#') {
      Serial.println();
      return password;
    }
  }
}

// Function to show admin menu
void showAdminMenu() {
  Serial.println("\nAdmin Access Granted!");
  displayMessage("Admin Mode");

  while (true) {
    Serial.println("1: Enroll Fingerprint");
    Serial.println("2: Delete Fingerprint");
    Serial.println("3: Exit");
    displayMessage("1:Enroll             2:Del                3:Exit");

    char key = waitForKey();
    Serial.print("Selected: "); Serial.println(key);

    if (key == '1') {
      int id = getIDFromKeypad("Enter ID to Enroll:");
      enrollFingerprint(id);
    } else if (key == '2') {
      int delID = getIDFromKeypad("Enter ID to Delete:");
      deleteFingerprint(delID);
    } else if (key == '3') {
      Serial.println("Exiting Admin Mode...");
      inAdminMode = false;
      displayMessage("Exiting...");
      delay(2000);
      return;
    }
  }
}

// Function to enroll a fingerprint
void enrollFingerprint(int id) {
  displayMessage("Enroll ID: " + String(id));

  Serial.print("Place your finger for ID "); Serial.println(id);
  displayMessage("Place Finger");

  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("First image captured");
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;

  Serial.println("Remove your finger...");
  displayMessage("Remove Finger");
  delay(2000);

  Serial.println("Place the same finger again...");
  displayMessage("Place Same Finger");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  Serial.println("Second image captured");
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return;

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint stored successfully!");
    displayMessage("Enroll Success!");
  } else {
    Serial.println("Error storing fingerprint.");
    displayMessage("Enroll Failed!");
  }
}

// Function to delete a fingerprint
void deleteFingerprint(int id) {
  displayMessage("Deleting ID: " + String(id));
  int p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint deleted successfully!");
    displayMessage("Deleted Successfully!");
  } else {
    Serial.println("Error deleting fingerprint");
    displayMessage("Delete Failed!");
  }
}

// Function to display messages on OLED
void displayMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.println(message);
  display.display();
}

// Function to wait for a key press
char waitForKey() {
  char key = 0;
  while (!key) key = keypad.getKey();
  return key;
}

// Function to get an ID from keypad with live display
int getIDFromKeypad(String prompt) {
  int id = 0;
  char key;
  displayMessage(prompt);

  while (true) {
    key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      id = id * 10 + (key - '0');
      displayMessage(prompt + "\n" + String(id));
    } else if (key == '#') {
      return id;
    }
  }
}
