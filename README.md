# MicroQubit

⚛️ **A quantum computing simulator running on microcontroller hardware**

MicroQubit brings quantum computing to embedded systems, implementing a full statevector simulator with 10 qubits on an Arduino UNO R4 WiFi. Apply quantum gates, create entanglement, and visualize quantum states in real-time through an intuitive web interface.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Arduino%20UNO%20R4%20WiFi-00979D.svg)
![Qubits](https://img.shields.io/badge/qubits-10-brightgreen.svg)

## ✨ Features

### Quantum Simulation
- **10-qubit statevector simulation** with complex amplitudes
- **Universal quantum gates**: X, Y, Z, H (Hadamard), T (π/4 phase), CNOT
- **Measurement operations** with quantum state collapse
- **Full phase simulation** enabling quantum interference effects
- **Entanglement support** via two-qubit CNOT gates

### Hardware Outputs
- **16x2 LCD Display**: Rotates through quantum states showing |ψ⟩ notation and probabilities
- **12x8 LED Matrix**: Real-time bar chart visualization of individual qubit probabilities
- **Web Interface**: Modern, responsive UI for gate operations accessible from any device

### Web Interface
- Clean, gradient-based dark theme optimized for quantum visualization
- Separate controls for single-qubit and two-qubit gates
- Real-time state updates with animated transitions
- Mobile-responsive design

## 🔧 Hardware Requirements

- **Arduino UNO R4 WiFi** (or compatible microcontroller with WiFi)
- **16x2 LCD Display** (HD44780 compatible)
  - RS → Pin 12
  - Enable → Pin 11
  - D4 → Pin 5
  - D5 → Pin 4
  - D6 → Pin 3
  - D7 → Pin 2
- **Built-in 12x8 LED Matrix** (UNO R4 WiFi)

## 📦 Software Requirements

- Arduino IDE 2.0 or later
- Libraries:
  - `WiFiS3.h` (for Arduino UNO R4 WiFi)
  - `LiquidCrystal.h`
  - `Arduino_LED_Matrix.h`

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/justinwoodring/MicroQubit.git
cd MicroQubit
```

### 2. Configure WiFi

Open `MicroQubit.ino` and update your WiFi credentials:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 3. Upload to Arduino

1. Open the project in Arduino IDE
2. Select **Board**: Arduino UNO R4 WiFi
3. Select the correct **Port**
4. Click **Upload**

### 4. Connect

1. Open the Serial Monitor (115200 baud)
2. Note the IP address displayed on the LCD or Serial Monitor
3. Navigate to the IP address in your web browser
4. Start experimenting with quantum gates!

## 🎮 Usage

### Web Interface

1. **Select qubits**: Choose control and target qubits from the dropdowns
2. **Apply gates**: Click any gate button to apply it to the selected qubit(s)
3. **View states**: Watch the LCD cycle through quantum states with probabilities
4. **Observe probabilities**: LED matrix displays P(|1⟩) for each qubit as vertical bars
5. **Reset**: Click "Reset to |0⟩" to return to the initial state

### Quantum Gates

| Gate | Symbol | Description |
|------|--------|-------------|
| X | Pauli-X | Quantum NOT - flips \|0⟩ ↔ \|1⟩ |
| Y | Pauli-Y | Bit flip + phase flip |
| Z | Pauli-Z | Phase flip on \|1⟩ |
| H | Hadamard | Creates superposition |
| T | T-gate | π/4 phase rotation |
| M | Measure | Collapses quantum state |
| CNOT | Controlled-NOT | Entangles two qubits |

### Example: Creating a Bell State

Create quantum entanglement (Bell state: |00⟩ + |11⟩):

1. Apply **H** gate to qubit 0 (creates superposition)
2. Apply **CNOT** with control=0, target=1 (creates entanglement)
3. Result: 50% probability of |0000000000⟩ and 50% probability of |0000000011⟩

## 🧮 How It Works

### Statevector Simulation

MicroQubit uses a full statevector representation with complex amplitudes:

- **State space**: 2^10 = 1024 basis states
- **Storage**: Separate `float` arrays for real and imaginary parts
- **Gates**: Implemented as unitary transformations on the statevector
- **Measurement**: Calculates probabilities as |ψ|² = real² + imag²

### Quantum Interference

Unlike classical probability, quantum amplitudes can interfere constructively and destructively. The complex amplitude simulation enables effects like:

- **Constructive interference**: H-Z-H on |0⟩ → |1⟩
- **Phase kickback**: CNOT can create phase relationships between qubits
- **Entanglement**: Non-separable quantum states

### Display Visualization

**LCD**: Shows quantum states in Dirac notation (|ψ⟩) with rotating display for states with >1% probability

**LED Matrix**: Vertical bar charts where:
- Each column (0-9) represents a qubit
- Bar height shows P(qubit = |1⟩)
- Bottom row shows maximum state probability

## 📊 Memory Constraints

The Arduino UNO R4 WiFi has limited RAM (~32KB), so the 10-qubit simulation (1024 complex amplitudes × 8 bytes = 8KB) uses approximately 25% of available memory. This leaves room for the web server, LCD, and LED matrix operations.

For larger qubit counts, consider:
- Using a microcontroller with more RAM (ESP32, Teensy 4.1)
- Implementing sparse statevector representations
- Using density matrix formalism for mixed states

## 🛠️ Customization

### Changing Qubit Count

Modify the `NUM_QUBITS` constant:

```cpp
#define NUM_QUBITS 8  // Reduce to 8 qubits
```

⚠️ Note: Each additional qubit doubles memory usage

### Adding Custom Gates

Implement new gate functions following the pattern:

```cpp
void applyCustomGate(int qubit) {
  // Your gate transformation here
  // Modify stateReal[] and stateImag[] arrays
}
```

## 📝 Project Structure

```
MicroQubit/
├── HelloWorld.ino         # Main Arduino sketch
├── README.md              # This file
└── LICENSE                # MIT License
```

## 🤝 Contributing

Contributions are welcome! Feel free to:

- Report bugs
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👤 Author

**Justin Woodring**

- GitHub: [@justinwoodring](https://github.com/justinwoodring)
- Project: [MicroQubit](https://github.com/justinwoodring/MicroQubit)

## 🙏 Acknowledgments

- Based on quantum computing principles and statevector simulation techniques
- Inspired by quantum computing frameworks like Qiskit and Cirq
- Built for the Arduino UNO R4 WiFi platform

## 🔗 Resources

- [Quantum Computing Basics](https://qiskit.org/textbook/ch-states/introduction.html)
- [Arduino UNO R4 WiFi Documentation](https://docs.arduino.cc/hardware/uno-r4-wifi)
- [Quantum Gates Reference](https://en.wikipedia.org/wiki/Quantum_logic_gate)

---

⚛️ **Quantum computing, now running on your desk!**
