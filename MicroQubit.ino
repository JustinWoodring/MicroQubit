/*
  MicroQubit - Microcontroller-Based Quantum Simulator

  A web-based quantum computing simulator running on embedded hardware

  Features:
  - 10-qubit quantum statevector simulation
  - Universal quantum gates (X, Y, Z, H, T, CNOT)
  - Complex amplitude simulation with phase
  - 16x2 LCD display for quantum state readout
  - 12x8 LED matrix for probability visualization
  - Modern web interface for gate operations

  Hardware:
  - Arduino UNO R4 WiFi (or compatible microcontroller)
  - 16x2 LCD Display (HD44780 compatible)
  - 12x8 LED Matrix

  Author: Justin Woodring
  Repository: https://github.com/justinwoodring/MicroQubit
  License: MIT

  Based on quantum computing principles and statevector simulation
*/

#include <WiFiS3.h>
#include <LiquidCrystal.h>
#include "Arduino_LED_Matrix.h"

// WiFi credentials - CHANGE THESE
const char* ssid = "ChangeMe";
const char* password = "ChangeMe";

// LCD pins (keeping your existing configuration)
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LED Matrix
ArduinoLEDMatrix matrix;

// Web server on port 80
WiFiServer server(80);

// Quantum simulation constants
#define NUM_QUBITS 10
#define STATE_SIZE (1 << NUM_QUBITS)  // 2^10 = 1024 states

// Quantum state vector - complex amplitudes
// Using separate arrays for real and imaginary parts
float stateReal[STATE_SIZE];  // Real part of amplitude
float stateImag[STATE_SIZE];  // Imaginary part of amplitude
int currentMeasurement = 0;

// LCD state rotation variables
unsigned long lastLCDUpdate = 0;
int currentDisplayState = 0;
int dotCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Quantum Sim");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // Initialize LED Matrix
  matrix.begin();

  // Initialize quantum state to |0000000000⟩
  initializeQuantumState();

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  lcd.clear();
  lcd.print("WiFi Connect...");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(3000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.print("IP:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    server.begin();
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("WiFi Failed!");
    Serial.println("\nWiFi connection failed!");
  }

  updateDisplay();
}

void loop() {
  // Update LCD display rotation
  updateLCDRotation();

  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Parse request and execute quantum operation
            handleRequest(request);

            // Send HTTP response
            sendHTMLPage(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}

void initializeQuantumState() {
  // Initialize to |0000000000⟩ state
  for (int i = 0; i < STATE_SIZE; i++) {
    stateReal[i] = 0.0;
    stateImag[i] = 0.0;
  }
  stateReal[0] = 1.0;  // |0⟩ state with amplitude 1+0i
  currentMeasurement = 0;
}

float getStateProbability(int state) {
  // Probability = |amplitude|^2 = real^2 + imag^2
  float re = stateReal[state];
  float im = stateImag[state];
  return re * re + im * im;
}

void handleRequest(String request) {
  // Parse gate operations from URL
  if (request.indexOf("GET /gate?") >= 0) {
    int gateStart = request.indexOf("op=") + 3;
    int gateEnd = request.indexOf("&", gateStart);
    if (gateEnd == -1) gateEnd = request.indexOf(" ", gateStart);

    String gateType = request.substring(gateStart, gateEnd);

    int qubitStart = request.indexOf("q=") + 2;
    int qubitEnd = request.indexOf("&", qubitStart);
    if (qubitEnd == -1) qubitEnd = request.indexOf(" ", qubitStart);

    int qubit = request.substring(qubitStart, qubitEnd).toInt();

    // For CNOT, parse target qubit
    int target = -1;
    if (gateType == "CNOT") {
      int targetStart = request.indexOf("t=") + 2;
      int targetEnd = request.indexOf("&", targetStart);
      if (targetEnd == -1) targetEnd = request.indexOf(" ", targetStart);
      target = request.substring(targetStart, targetEnd).toInt();
    }

    // Apply gate
    if (gateType == "X") {
      applyXGate(qubit);
    } else if (gateType == "Y") {
      applyYGate(qubit);
    } else if (gateType == "Z") {
      applyZGate(qubit);
    } else if (gateType == "H") {
      applyHGate(qubit);
    } else if (gateType == "T") {
      applyTGate(qubit);
    } else if (gateType == "CNOT") {
      applyCNOTGate(qubit, target);
    } else if (gateType == "M") {
      measureQubit(qubit);
    }

    updateDisplay();
  } else if (request.indexOf("GET /reset") >= 0) {
    initializeQuantumState();
    updateDisplay();
  }
}

void applyXGate(int qubit) {
  // X gate (NOT gate) - flips qubit
  // Matrix: [[0, 1], [1, 0]]
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;
  float tempReal[STATE_SIZE];
  float tempImag[STATE_SIZE];

  for (int i = 0; i < STATE_SIZE; i++) {
    int flipped = i ^ mask;
    tempReal[i] = stateReal[flipped];
    tempImag[i] = stateImag[flipped];
  }

  for (int i = 0; i < STATE_SIZE; i++) {
    stateReal[i] = tempReal[i];
    stateImag[i] = tempImag[i];
  }

  Serial.print("Applied X gate to qubit ");
  Serial.println(qubit);
}

void applyYGate(int qubit) {
  // Y gate - Pauli Y
  // Matrix: [[0, -i], [i, 0]]
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;
  float tempReal[STATE_SIZE];
  float tempImag[STATE_SIZE];

  for (int i = 0; i < STATE_SIZE; i++) {
    int flipped = i ^ mask;
    if (i & mask) {
      // |1⟩ component: multiply by i
      tempReal[i] = -stateImag[flipped];
      tempImag[i] = stateReal[flipped];
    } else {
      // |0⟩ component: multiply by -i
      tempReal[i] = stateImag[flipped];
      tempImag[i] = -stateReal[flipped];
    }
  }

  for (int i = 0; i < STATE_SIZE; i++) {
    stateReal[i] = tempReal[i];
    stateImag[i] = tempImag[i];
  }

  Serial.print("Applied Y gate to qubit ");
  Serial.println(qubit);
}

void applyZGate(int qubit) {
  // Z gate - phase flip
  // Matrix: [[1, 0], [0, -1]]
  // Multiplies |1⟩ component by -1
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;

  for (int i = 0; i < STATE_SIZE; i++) {
    if (i & mask) {
      stateReal[i] = -stateReal[i];
      stateImag[i] = -stateImag[i];
    }
  }

  Serial.print("Applied Z gate to qubit ");
  Serial.println(qubit);
}

void applyTGate(int qubit) {
  // T gate - π/4 phase gate
  // Matrix: [[1, 0], [0, e^(iπ/4)]]
  // e^(iπ/4) = cos(π/4) + i*sin(π/4) = (1+i)/√2
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;
  float cosPi4 = 0.70710678118;  // cos(π/4) = 1/√2
  float sinPi4 = 0.70710678118;  // sin(π/4) = 1/√2

  for (int i = 0; i < STATE_SIZE; i++) {
    if (i & mask) {
      // Multiply |1⟩ component by e^(iπ/4)
      float oldReal = stateReal[i];
      float oldImag = stateImag[i];
      stateReal[i] = oldReal * cosPi4 - oldImag * sinPi4;
      stateImag[i] = oldReal * sinPi4 + oldImag * cosPi4;
    }
  }

  Serial.print("Applied T gate to qubit ");
  Serial.println(qubit);
}

void applyCNOTGate(int control, int target) {
  // CNOT gate - controlled NOT
  // Flips target qubit if control qubit is |1⟩
  if (control >= NUM_QUBITS || target >= NUM_QUBITS || control == target) return;

  int controlMask = 1 << control;
  int targetMask = 1 << target;
  float tempReal[STATE_SIZE];
  float tempImag[STATE_SIZE];

  for (int i = 0; i < STATE_SIZE; i++) {
    // If control bit is set, flip target bit
    if (i & controlMask) {
      int flipped = i ^ targetMask;
      tempReal[i] = stateReal[flipped];
      tempImag[i] = stateImag[flipped];
    } else {
      tempReal[i] = stateReal[i];
      tempImag[i] = stateImag[i];
    }
  }

  for (int i = 0; i < STATE_SIZE; i++) {
    stateReal[i] = tempReal[i];
    stateImag[i] = tempImag[i];
  }

  Serial.print("Applied CNOT gate: control=");
  Serial.print(control);
  Serial.print(", target=");
  Serial.println(target);
}

void applyHGate(int qubit) {
  // Hadamard gate - creates superposition
  // Matrix: (1/√2) * [[1, 1], [1, -1]]
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;
  float tempReal[STATE_SIZE];
  float tempImag[STATE_SIZE];
  float invSqrt2 = 0.70710678118;  // 1/√2

  for (int i = 0; i < STATE_SIZE; i++) {
    int i0 = i & ~mask;  // State with qubit = 0
    int i1 = i | mask;   // State with qubit = 1

    if (i == i0) {
      // H|0⟩ = (|0⟩ + |1⟩)/√2
      tempReal[i] = invSqrt2 * (stateReal[i0] + stateReal[i1]);
      tempImag[i] = invSqrt2 * (stateImag[i0] + stateImag[i1]);
    } else {
      // H|1⟩ = (|0⟩ - |1⟩)/√2
      tempReal[i] = invSqrt2 * (stateReal[i0] - stateReal[i1]);
      tempImag[i] = invSqrt2 * (stateImag[i0] - stateImag[i1]);
    }
  }

  for (int i = 0; i < STATE_SIZE; i++) {
    stateReal[i] = tempReal[i];
    stateImag[i] = tempImag[i];
  }

  Serial.print("Applied H gate to qubit ");
  Serial.println(qubit);
}

void measureQubit(int qubit) {
  // Collapse quantum state by measurement
  if (qubit >= NUM_QUBITS) return;

  int mask = 1 << qubit;
  float prob0 = 0.0;

  // Calculate probability of measuring 0
  for (int i = 0; i < STATE_SIZE; i++) {
    if ((i & mask) == 0) {
      prob0 += getStateProbability(i);
    }
  }

  // Simulate measurement (simplified - always measure most probable)
  int result = (prob0 > 0.5) ? 0 : 1;
  currentMeasurement = result;

  // Collapse state - zero out incompatible basis states
  for (int i = 0; i < STATE_SIZE; i++) {
    if (((i & mask) != 0) != (result != 0)) {
      stateReal[i] = 0.0;
      stateImag[i] = 0.0;
    }
  }

  normalizeState();

  Serial.print("Measured qubit ");
  Serial.print(qubit);
  Serial.print(" = ");
  Serial.println(result);
}

void normalizeState() {
  // Normalize the state vector so that sum of |amplitude|^2 = 1
  float sum = 0.0;
  for (int i = 0; i < STATE_SIZE; i++) {
    sum += getStateProbability(i);
  }

  if (sum > 0.0001) {
    float norm = sqrt(sum);
    for (int i = 0; i < STATE_SIZE; i++) {
      stateReal[i] /= norm;
      stateImag[i] /= norm;
    }
  }
}

void updateDisplay() {
  // Reset LCD rotation
  currentDisplayState = 0;
  lastLCDUpdate = millis();
  dotCount = 0;

  // Update LED matrix visualization
  updateLEDMatrix();
}

void updateLCDRotation() {
  unsigned long currentTime = millis();

  // Collect significant states (probability > 1%)
  int significantStates[20];
  float significantProbs[20];
  int numSignificant = 0;

  for (int i = 0; i < STATE_SIZE && numSignificant < 20; i++) {
    float prob = getStateProbability(i);
    if (prob > 0.01) {  // 1% threshold
      significantStates[numSignificant] = i;
      significantProbs[numSignificant] = prob;
      numSignificant++;
    }
  }

  if (numSignificant == 0) {
    numSignificant = 1;
    significantStates[0] = 0;
    significantProbs[0] = 0.0;
  }

  // Update display every 2 seconds
  if (currentTime - lastLCDUpdate >= 2000) {
    lastLCDUpdate = currentTime;
    currentDisplayState = (currentDisplayState + 1) % numSignificant;
    dotCount = 0;
  }

  // Update dots every 500ms
  if ((currentTime - lastLCDUpdate) % 500 < 50 && dotCount < 2) {
    dotCount++;
    displayStateOnLCD(significantStates[currentDisplayState], significantProbs[currentDisplayState]);
  }
}

void displayStateOnLCD(int state, float prob) {
  lcd.clear();

  // Line 1: |state>
  lcd.print("|");

  // Print binary state with padding
  String binaryState = "";
  for (int i = NUM_QUBITS - 1; i >= 0; i--) {
    binaryState += (state & (1 << i)) ? "1" : "0";
  }

  // Fit on display (16 chars max)
  if (binaryState.length() <= 13) {
    lcd.print(binaryState);
    lcd.print(">");
  } else {
    lcd.print(binaryState.substring(0, 13));
    lcd.print(">");
  }

  // Line 2: Probability with dots
  lcd.setCursor(0, 1);
  lcd.print("P=");
  lcd.print(prob * 100, 1);
  lcd.print("%");

  // Add animated dots
  for (int i = 0; i < dotCount; i++) {
    lcd.print(".");
  }
}

void updateLEDMatrix() {
  // LED Matrix: 12 columns x 8 rows = 96 LEDs
  // Arduino UNO R4 WiFi LED Matrix format:
  // - 3 x uint32_t array (96 bits total)
  // - Bits are numbered 0-95 (row-major order: row 0 col 0-11, row 1 col 0-11, ...)
  // - Uses MSB-first: bit position = 31 - (bit_number % 32)

  uint32_t frame[3] = {0, 0, 0};

  // Calculate individual qubit probabilities
  float qubitProbs[NUM_QUBITS];
  for (int q = 0; q < NUM_QUBITS; q++) {
    qubitProbs[q] = 0.0;
    int mask = 1 << q;

    // Sum probabilities where qubit q is |1⟩
    for (int i = 0; i < STATE_SIZE; i++) {
      if (i & mask) {
        qubitProbs[q] += getStateProbability(i);
      }
    }
  }

  // Find highest probability state for bottom row
  float maxStateProb = 0.0;
  for (int i = 0; i < STATE_SIZE; i++) {
    float prob = getStateProbability(i);
    if (prob > maxStateProb) {
      maxStateProb = prob;
    }
  }

  // Helper function to set a pixel
  auto setPixel = [&](int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 12) return;
    int bitNum = row * 12 + col;  // Bit number 0-95
    int index = bitNum / 32;      // Which uint32_t (0, 1, or 2)
    int offset = 31 - (bitNum % 32);  // Bit position (MSB-first)
    frame[index] |= (1UL << offset);
  };

  // Draw bar chart for each qubit (columns 0-9)
  // Row 0 = top, Row 7 = bottom
  for (int q = 0; q < NUM_QUBITS; q++) {
    int barHeight = (int)(qubitProbs[q] * 7.0 + 0.5);  // Scale to 0-7
    if (barHeight > 7) barHeight = 7;

    // Fill from bottom (row 7) up
    for (int h = 0; h < barHeight; h++) {
      int row = 7 - h;  // Start from bottom
      setPixel(row, q);
    }
  }

  // Row 7 (bottom): show max state probability across all 10 qubits
  int totalBarLength = (int)(maxStateProb * 10.0 + 0.5);  // Scale to 0-10
  for (int col = 0; col < totalBarLength && col < NUM_QUBITS; col++) {
    setPixel(7, col);
  }

  matrix.loadFrame(frame);
}

void sendHTMLPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html; charset=utf-8");
  client.println();

  client.println("<!DOCTYPE html><html><head>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<title>MicroQubit - Quantum Simulator</title>");
  client.println("<style>");
  client.println("*{margin:0;padding:0;box-sizing:border-box}");
  client.println("body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:linear-gradient(135deg,#0a0a1a 0%,#1a0a2e 100%);");
  client.println("color:#e0e0e0;min-height:100vh;padding:20px}");
  client.println(".container{max-width:800px;margin:0 auto;background:rgba(20,20,40,0.8);border-radius:20px;");
  client.println("padding:30px;box-shadow:0 8px 32px rgba(0,255,136,0.1);backdrop-filter:blur(10px)}");
  client.println("h1{text-align:center;font-size:2.5em;background:linear-gradient(90deg,#00ff88,#00d4ff);");
  client.println("-webkit-background-clip:text;-webkit-text-fill-color:transparent;margin-bottom:10px}");
  client.println(".subtitle{text-align:center;color:#888;margin-bottom:30px;font-size:0.9em}");
  client.println(".section{margin:25px 0;padding:20px;background:rgba(30,30,60,0.5);border-radius:15px;");
  client.println("border:1px solid rgba(0,255,136,0.2)}");
  client.println(".section h3{color:#00ff88;margin-bottom:15px;font-size:1.2em}");
  client.println("label{color:#aaa;font-size:0.9em;margin-right:10px}");
  client.println(".qubit-select{background:#2a2a4a;color:#fff;border:2px solid #00ff88;border-radius:8px;");
  client.println("padding:10px 15px;font-size:16px;cursor:pointer;transition:all 0.3s}");
  client.println(".qubit-select:hover{background:#3a3a5a;border-color:#00d4ff}");
  client.println(".gate-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px;margin-top:15px}");
  client.println(".gate-btn{background:linear-gradient(135deg,#0066cc,#0052a3);border:none;color:#fff;");
  client.println("padding:15px 20px;font-size:15px;font-weight:600;border-radius:10px;cursor:pointer;");
  client.println("transition:all 0.3s;box-shadow:0 4px 15px rgba(0,102,204,0.3)}");
  client.println(".gate-btn:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(0,102,204,0.5)}");
  client.println(".gate-btn:active{transform:translateY(0)}");
  client.println(".cnot-btn{background:linear-gradient(135deg,#cc6600,#a35200)}");
  client.println(".cnot-btn:hover{box-shadow:0 6px 20px rgba(204,102,0,0.5)}");
  client.println(".reset-btn{background:linear-gradient(135deg,#cc0000,#990000);border:none;color:#fff;");
  client.println("padding:15px 40px;font-size:16px;font-weight:600;border-radius:10px;cursor:pointer;");
  client.println("margin-top:20px;transition:all 0.3s;box-shadow:0 4px 15px rgba(204,0,0,0.3)}");
  client.println(".reset-btn:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(204,0,0,0.5)}");
  client.println(".info-box{background:rgba(0,100,200,0.1);border-left:4px solid #00d4ff;padding:15px;");
  client.println("border-radius:8px;margin-top:20px}");
  client.println(".info-box p{margin:8px 0;font-size:0.9em}");
  client.println(".footer{text-align:center;margin-top:30px;padding-top:20px;border-top:1px solid rgba(255,255,255,0.1);");
  client.println("color:#666;font-size:0.85em}");
  client.println(".footer a{color:#00ff88;text-decoration:none}");
  client.println(".footer a:hover{text-decoration:underline}");
  client.println(".control-row{display:flex;align-items:center;gap:15px;flex-wrap:wrap;margin-bottom:15px}");
  client.println("@media(max-width:600px){h1{font-size:2em}.gate-grid{grid-template-columns:1fr 1fr}}");
  client.println("</style></head><body>");

  client.println("<div class='container'>");
  client.println("<h1>⚛️ MicroQubit</h1>");
  client.println("<div class='subtitle'>10-Qubit Statevector Quantum Simulator</div>");

  client.println("<div class='section'>");
  client.println("<div class='control-row'>");
  client.println("<label>Control Qubit:</label>");
  client.println("<select id='qubit' class='qubit-select'>");
  for (int i = 0; i < NUM_QUBITS; i++) {
    client.print("<option value='");
    client.print(i);
    client.print("'>Q");
    client.print(i);
    client.println("</option>");
  }
  client.println("</select>");
  client.println("<label style='margin-left:20px'>Target Qubit:</label>");
  client.println("<select id='target' class='qubit-select'>");
  for (int i = 0; i < NUM_QUBITS; i++) {
    client.print("<option value='");
    client.print(i);
    client.print("'>Q");
    client.print(i);
    client.println("</option>");
  }
  client.println("</select>");
  client.println("</div></div>");

  client.println("<div class='section'>");
  client.println("<h3>Single-Qubit Gates</h3>");
  client.println("<div class='gate-grid'>");
  client.println("<button class='gate-btn' onclick='applyGate(\"X\")'>X</button>");
  client.println("<button class='gate-btn' onclick='applyGate(\"Y\")'>Y</button>");
  client.println("<button class='gate-btn' onclick='applyGate(\"Z\")'>Z</button>");
  client.println("<button class='gate-btn' onclick='applyGate(\"H\")'>H</button>");
  client.println("<button class='gate-btn' onclick='applyGate(\"T\")'>T</button>");
  client.println("<button class='gate-btn' onclick='applyGate(\"M\")'>M</button>");
  client.println("</div></div>");

  client.println("<div class='section'>");
  client.println("<h3>Two-Qubit Gates</h3>");
  client.println("<div class='gate-grid'>");
  client.println("<button class='gate-btn cnot-btn' onclick='applyCNOT()'>CNOT</button>");
  client.println("</div></div>");

  client.println("<div style='text-align:center'>");
  client.println("<button class='reset-btn' onclick='reset()'>Reset to |0⟩</button>");
  client.println("</div>");

  client.println("<div class='info-box'>");
  client.println("<p><strong>📊 LCD Display:</strong> Shows rotating quantum states |ψ⟩ with probabilities</p>");
  client.println("<p><strong>💡 LED Matrix:</strong> Bar chart visualization of qubit probabilities</p>");
  client.println("<p><strong>⚡ Gates:</strong> X (NOT) | Y, Z (Pauli) | H (Hadamard) | T (π/4) | M (Measure) | CNOT (Entangle)</p>");
  client.println("</div>");

  client.println("<div class='footer'>");
  client.println("<p>MicroQubit - Microcontroller Quantum Simulator</p>");
  client.println("<p>By <a href='https://github.com/justinwoodring' target='_blank'>Justin Woodring</a> | ");
  client.println("<a href='https://github.com/justinwoodring/MicroQubit' target='_blank'>GitHub Repository</a></p>");
  client.println("</div>");

  client.println("</div>");

  client.println("<script>");
  client.println("function applyGate(gate){");
  client.println("var q=document.getElementById('qubit').value;");
  client.println("window.location.href='/gate?op='+gate+'&q='+q;}");
  client.println("function applyCNOT(){");
  client.println("var c=document.getElementById('qubit').value;");
  client.println("var t=document.getElementById('target').value;");
  client.println("if(c==t){alert('Control and target must be different!');return;}");
  client.println("window.location.href='/gate?op=CNOT&q='+c+'&t='+t;}");
  client.println("function reset(){window.location.href='/reset';}");
  client.println("</script>");

  client.println("</body></html>");
  client.println();
}
